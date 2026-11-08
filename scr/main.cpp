#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <climits>

struct Facility {
    std::string id;
    std::string name;
    std::string building;
    std::string category;
};

static std::string trim(const std::string& s) {
    const std::string ws = " \t\r\n\f\v";
    std::size_t b = s.find_first_not_of(ws);
    if (b == std::string::npos) return "";
    std::size_t e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

static std::string toUpper(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i)
        s[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
    return s;
}

static std::string toLower(std::string s) {
    for (std::size_t i = 0; i < s.size(); ++i)
        s[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(s[i])));
    return s;
}

static bool equalsIgnoreCase(const std::string& a, const std::string& b) {
    return toLower(a) == toLower(b);
}

static bool isCommentLine(const std::string& line) {
    std::string t = trim(line);
    return !t.empty() && t[0] == '#';
}

static std::vector<std::string> splitFields(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    for (std::size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '|') {
            fields.push_back(trim(cur));
            cur.clear();
        } else {
            cur += line[i];
        }
    }
    fields.push_back(trim(cur));
    return fields;
}

static bool isValidFacilityId(const std::string& id) {
    if (id.size() != 4 || id[0] != 'F') return false;
    for (int i = 1; i < 4; ++i)
        if (!std::isdigit(static_cast<unsigned char>(id[i]))) return false;
    return true;
}

static void warn(const std::string& file, int lineNo, const std::string& reason) {
    std::cout << "Warning: " << file << " line " << lineNo << ": " << reason << std::endl;
}

class FacilityHashTable {
public:
    explicit FacilityHashTable(std::size_t cap)
        : buckets_(cap == 0 ? 1 : cap, static_cast<Node*>(0)), entries_(0), collisions_(0) {}

    ~FacilityHashTable() {
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            Node* n = buckets_[i];
            while (n) {
                Node* next = n->next;
                delete n;
                n = next;
            }
        }
    }

    bool insert(const std::string& key, int index) {
        std::size_t b = hash(key);
        Node* n = buckets_[b];
        Node* tail = 0;
        while (n) {
            if (n->key == key) return false;
            tail = n;
            n = n->next;
        }
        Node* fresh = new Node(key, index);
        if (tail) {
            tail->next = fresh;
            ++collisions_;
        } else {
            buckets_[b] = fresh;
        }
        ++entries_;
        return true;
    }

    int find(const std::string& key) const {
        Node* n = buckets_[hash(key)];
        while (n) {
            if (n->key == key) return n->index;
            n = n->next;
        }
        return -1;
    }

    std::size_t capacity() const { return buckets_.size(); }
    std::size_t entries() const { return entries_; }
    std::size_t collisions() const { return collisions_; }

private:
    struct Node {
        std::string key;
        int index;
        Node* next;
        Node(const std::string& k, int i) : key(k), index(i), next(0) {}
    };

    FacilityHashTable(const FacilityHashTable&);
    FacilityHashTable& operator=(const FacilityHashTable&);

    std::size_t hash(const std::string& key) const {
        unsigned long h = 5381UL;
        for (std::size_t i = 0; i < key.size(); ++i)
            h = (h * 33UL + static_cast<unsigned char>(key[i])) % 4294967291UL;
        return static_cast<std::size_t>(h % buckets_.size());
    }

    std::vector<Node*> buckets_;
    std::size_t entries_;
    std::size_t collisions_;
};

class FacilityManager {
public:
    explicit FacilityManager(std::size_t hashCapacity) : table_(hashCapacity) {}

    bool add(const Facility& f) {
        int idx = static_cast<int>(records_.size());
        if (!table_.insert(f.id, idx)) return false;
        records_.push_back(f);
        return true;
    }

    const Facility* findById(const std::string& id) const {
        int idx = table_.find(id);
        return idx < 0 ? 0 : &records_[idx];
    }

    const std::vector<Facility>& all() const { return records_; }
    const FacilityHashTable& table() const { return table_; }

    std::string canonicalCategory(const std::string& cat) const {
        for (std::size_t i = 0; i < categories_.size(); ++i)
            if (equalsIgnoreCase(categories_[i], cat)) return categories_[i];
        return cat;
    }

    void recordCategory(const std::string& cat) {
        for (std::size_t i = 0; i < categories_.size(); ++i)
            if (equalsIgnoreCase(categories_[i], cat)) return;
        categories_.push_back(cat);
    }

private:
    FacilityHashTable table_;
    std::vector<Facility> records_;
    std::vector<std::string> categories_;
};

static int findBuilding(const std::vector<std::string>& buildings, const std::string& name) {
    for (std::size_t i = 0; i < buildings.size(); ++i)
        if (equalsIgnoreCase(buildings[i], name)) return static_cast<int>(i);
    return -1;
}

static void loadBuildings(const std::string& path, const std::string& fileName,
                          std::vector<std::string>& buildings) {
    int accepted = 0, rejected = 0;
    std::ifstream in(path.c_str());
    if (!in) {
        std::cout << "Error: cannot open file " << fileName << std::endl;
    } else {
        std::string line;
        int lineNo = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            if (trim(line).empty() || isCommentLine(line)) continue;
            std::string name = trim(line);
            if (name.find('|') != std::string::npos) {
                warn(fileName, lineNo, "wrong field count (expected 1 field)");
                ++rejected;
                continue;
            }
            if (findBuilding(buildings, name) >= 0) {
                warn(fileName, lineNo, "duplicate building name '" + name + "'");
                ++rejected;
                continue;
            }
            buildings.push_back(name);
            ++accepted;
        }
    }
    std::cout << fileName << ": " << accepted << " accepted, " << rejected << " rejected" << std::endl;
}

static void loadFacilities(const std::string& path, const std::string& fileName,
                           const std::vector<std::string>& buildings, FacilityManager& mgr) {
    int accepted = 0, rejected = 0;
    std::ifstream in(path.c_str());
    if (!in) {
        std::cout << "Error: cannot open file " << fileName << std::endl;
    } else {
        std::string line;
        int lineNo = 0;
        while (std::getline(in, line)) {
            ++lineNo;
            if (trim(line).empty() || isCommentLine(line)) continue;
            std::vector<std::string> f = splitFields(line);
            if (f.size() != 4) {
                warn(fileName, lineNo, "wrong field count (expected 4 fields)");
                ++rejected;
                continue;
            }
            if (f[0].empty() || f[1].empty() || f[2].empty() || f[3].empty()) {
                warn(fileName, lineNo, "empty required field");
                ++rejected;
                continue;
            }
            std::string id = toUpper(f[0]);
            if (!isValidFacilityId(id)) {
                warn(fileName, lineNo, "invalid facility ID '" + f[0] + "'");
                ++rejected;
                continue;
            }
            int b = findBuilding(buildings, f[2]);
            if (b < 0) {
                warn(fileName, lineNo, "unknown building '" + f[2] + "'");
                ++rejected;
                continue;
            }
            if (mgr.findById(id)) {
                warn(fileName, lineNo, "duplicate facility ID " + id);
                ++rejected;
                continue;
            }
            Facility fac;
            fac.id = id;
            fac.name = f[1];
            fac.building = buildings[b];
            mgr.recordCategory(f[3]);
            fac.category = mgr.canonicalCategory(f[3]);
            mgr.add(fac);
            ++accepted;
        }
    }
    std::cout << fileName << ": " << accepted << " accepted, " << rejected << " rejected"
              << " (hash capacity " << mgr.table().capacity()
              << ", entries " << mgr.table().entries()
              << ", collisions " << mgr.table().collisions() << ")" << std::endl;
}

static void viewAllFacilities(const FacilityManager& mgr) {
    const std::vector<Facility>& all = mgr.all();
    if (all.empty()) {
        std::cout << "No facilities loaded." << std::endl;
        return;
    }
    std::cout << "ID   | Name | Building | Category" << std::endl;
    for (std::size_t i = 0; i < all.size(); ++i)
        std::cout << all[i].id << " | " << all[i].name << " | "
                  << all[i].building << " | " << all[i].category << std::endl;
}

static bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    if (!std::getline(std::cin, out)) return false;
    out = trim(out);
    return true;
}

static void searchFacilityById(const FacilityManager& mgr) {
    std::string input;
    if (!readLine("Enter facility ID: ", input)) return;
    std::string id = toUpper(input);
    if (!isValidFacilityId(id)) {
        std::cout << "Invalid format: an ID is F followed by exactly three digits" << std::endl;
        return;
    }
    const Facility* f = mgr.findById(id);
    if (!f) {
        std::cout << "Not found: no facility with ID " << id << std::endl;
        return;
    }
    std::cout << "Found: " << f->name << " | " << f->building << " | " << f->category << std::endl;
}

static void printMenu() {
    std::cout << "\n===== Campus Navigator =====" << std::endl;
    std::cout << " 1. View all facilities" << std::endl;
    std::cout << " 2. Search facility by ID" << std::endl;
    std::cout << " 3. View campus graph" << std::endl;
    std::cout << " 4. Display graph counts and degrees" << std::endl;
    std::cout << " 5. Find reachable buildings (BFS)" << std::endl;
    std::cout << " 6. Add department" << std::endl;
    std::cout << " 7. Search department" << std::endl;
    std::cout << " 8. Display department hierarchy" << std::endl;
    std::cout << " 9. Run preorder traversal" << std::endl;
    std::cout << "10. Exit" << std::endl;
}

static bool parseMenuChoice(const std::string& s, int& value) {
    if (s.empty() || s.size() > 6) return false;
    for (std::size_t i = 0; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    value = std::atoi(s.c_str());
    return true;
}

int main(int argc, char* argv[]) {
    std::string dataDir = "data";
    std::size_t hashCapacity = 11;
    if (argc > 1) dataDir = argv[1];
    if (argc > 2) {
        long c = std::atol(argv[2]);
        if (c > 0) hashCapacity = static_cast<std::size_t>(c);
        else std::cout << "Invalid hash capacity; using " << hashCapacity << std::endl;
    }
    if (!dataDir.empty() && dataDir[dataDir.size() - 1] != '/' && dataDir[dataDir.size() - 1] != '\\')
        dataDir += "/";

    std::vector<std::string> buildings;
    FacilityManager facilities(hashCapacity);

    loadBuildings(dataDir + "buildings.txt", "buildings.txt", buildings);

    // MEMBER B: load connections.txt here (after buildings, before facilities) and print its load summary.

    loadFacilities(dataDir + "facilities.txt", "facilities.txt", buildings, facilities);

    // MEMBER C: load departments.txt here and print its load summary.

    bool running = true;
    while (running) {
        printMenu();
        std::string line;
        if (!readLine("Enter choice: ", line)) break;
        int choice = 0;
        if (!parseMenuChoice(line, choice)) {
            std::cout << "Invalid input: please enter a number from 1 to 10." << std::endl;
            continue;
        }
        switch (choice) {
            case 1:
                viewAllFacilities(facilities);
                break;
            case 2:
                searchFacilityById(facilities);
                break;
            case 3:
                // MEMBER B: call the "view campus graph" function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 4:
                // MEMBER B: call the "display graph counts and degrees" function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 5:
                // MEMBER B: call the BFS reachability function here (prompt for start building, handle unknown names).
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 6:
                // MEMBER C: call the "add department" function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 7:
                // MEMBER C: call the "search department" function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 8:
                // MEMBER C: call the "display department hierarchy" function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 9:
                // MEMBER C: call the preorder traversal function here.
                std::cout << "Not implemented yet." << std::endl;
                break;
            case 10:
                std::cout << "Goodbye." << std::endl;
                running = false;
                break;
            default:
                std::cout << "Invalid input: please enter a number from 1 to 10." << std::endl;
                break;
        }
    }
    return 0;
}
