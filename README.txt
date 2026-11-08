================================================================================
CAMPUS FACILITY LOCATOR SYSTEM
Project 2 - C++ CLI Project (Milestone 1)
================================================================================

OVERVIEW
--------------------------------------------------------------------------------
A console-based campus navigation program that loads campus data from plain-text
files. Users can look up facilities by ID, view how campus buildings are
connected, find which buildings are reachable from a starting building, and
browse the academic department hierarchy.

The project applies three core data structures to a realistic problem:
a custom hash table (facility lookup by ID), an undirected graph with
Breadth-First Search (buildings and walkways), and a general tree (academic
departments), with all data loaded from text files.

This README describes the Milestone 1 version of the program only.


TEAM / AUTHORS
--------------------------------------------------------------------------------
Course:      CSCE 2110
Instructor:  Mokarram
Semester:    Fall 2026
Team:        Saul Hernandez, Cooper Gounah, Colin Segovia

FEATURES (MILESTONE 1)
--------------------------------------------------------------------------------
File Loading
  - Loads buildings.txt, connections.txt, facilities.txt, and departments.txt
    from the data folder
  - Skips blank lines and comment lines (first non-whitespace character is #)
  - Trims whitespace (spaces, tabs, and Windows \r line endings) around fields
  - Rejects invalid records with a warning showing the filename, line number,
    and reason, then continues loading
  - A missing file prints an error naming the file; the file is treated as
    empty and the program keeps running
  - Prints a load summary line after each file (accepted and rejected counts)

Facility Records and Hash Table
  - Facilities are stored in a custom hash table keyed by facility ID
  - Collisions are handled by chaining; the facilities.txt load summary reports
    the hash capacity, number of entries, and collision count
  - A duplicate-key insertion is rejected and does not change the collision
    count
  - View all facilities in the order they appear in facilities.txt
  - Search a facility by ID (case-insensitive; the ID is normalized to
    uppercase before validation and lookup)

Campus Graph and BFS
  - Buildings and walkways are loaded into an undirected graph
  - View the campus graph (each building and its neighbors, in the order the
    connections were accepted)
  - Display graph counts and degrees
  - Find all buildings reachable from a chosen building using BFS

Department Hierarchy (General Tree)
  - Departments are loaded into a general tree; siblings keep insertion order
  - Add a department from the menu (the first department becomes the root
    without a parent prompt; later departments require an existing parent)
  - Search for a department by name
  - Display the department hierarchy
  - Run a preorder traversal

Input Handling
  - Invalid menu input and text typed at numeric prompts are rejected with a
    message, and the menu is shown again
  - Names containing spaces are supported; blank names are rejected

Planned for the final version (NOT part of Milestone 1)
  - Facility search by name and by category, and the category report
  - Shortest path and connectivity report
  - Additional tree traversals and department statistics


PROJECT STRUCTURE
--------------------------------------------------------------------------------
Project2/
|
|-- include/                    Header files (reserved for future use; empty
|                               in Milestone 1, kept with a .gitkeep file)
|
|-- src/                        Source files
|     |-- main.cpp              Menu loop, file loading, facility hash table,
|                               and facility search
|
|-- data/                       The four starter input files (unchanged)
|     |-- buildings.txt
|     |-- connections.txt
|     |-- facilities.txt
|     |-- departments.txt
|
|-- testdata/                   Our own test files (including the hash
|                               collision test data)
|
|-- README.txt
|-- CONTRIBUTIONS.txt
|-- design_document.txt
|-- test_results.txt
|-- .gitignore


CLASSES AND DATA STRUCTURES
--------------------------------------------------------------------------------
Facility
  Stores facility ID, name, building name, and category.

FacilityHashTable
  Custom hash table keyed by facility ID (stored in uppercase). Uses chaining
  to resolve collisions and tracks capacity, entry count, and collision count.

FacilityManager
  Owns the hash table and the list of accepted facilities (kept in file order
  for display). Handles adding facilities, ID lookup, and category spelling.

Campus graph (undirected)
  Stores buildings and walkways. Neighbors are kept in accepted connection
  order and are never sorted, so BFS order is predictable.

Department tree (general tree)
  Stores the department hierarchy with the first accepted department as the
  root. Siblings are kept in insertion order.

Data structure summary:
  Structure            Used for                          Behavior
  -------------------  --------------------------------  ----------------------
  Hash table           Facility lookup by ID             Chaining, O(1) average
  Undirected graph     Buildings and walkways            BFS reachability
  General tree         Department hierarchy              Preorder traversal


REQUIREMENTS
--------------------------------------------------------------------------------
  - A C++ compiler with C++11 or newer support (g++ recommended)
  - Works on Windows, Linux, or macOS


HOW TO BUILD
--------------------------------------------------------------------------------
From the project's root folder (the one containing include/, src/, and data/):

  g++ -std=c++11 -Wall -Wextra -I include src/main.cpp -o main

If more .cpp files are added to src/, list each of them in the command.
Visual Studio users can instead add the files from src/ and include/ to a
project and set "include" as an additional include directory.


HOW TO RUN
--------------------------------------------------------------------------------
Run the program from the project's ROOT folder so it can find the data files
(it looks for the data folder relative to the working directory):

  Windows:        main.exe
  Linux / macOS:  ./main

Optional command-line arguments:

  main [dataDirectory] [hashCapacity]

  dataDirectory   Folder containing the four input files (default: data)
  hashCapacity    Number of hash table buckets (default: 11)

Example (collision test, small table):

  Windows:        main.exe testdata\collision 7
  Linux / macOS:  ./main testdata/collision 7


USING THE PROGRAM
--------------------------------------------------------------------------------
Main menu:

   1. View all facilities            List every loaded facility
   2. Search facility by ID          Enter an ID such as F004 (case-insensitive)
   3. View campus graph              Show buildings and their neighbors
   4. Display graph counts and degrees
   5. Find reachable buildings (BFS) Enter a starting building name
   6. Add department                 Enter a name and a parent department
   7. Search department              Enter a department name
   8. Display department hierarchy
   9. Run preorder traversal
  10. Exit

Facility ID search examples:
  Input   Result
  F004    Found: Computer Lab A | Science Building | Computer Lab
  f004    Same result as F004 (input normalized to uppercase)
  004     Invalid format: an ID is F followed by exactly three digits
  F999    Not found: no facility with ID F999

Example load summary on startup:
  buildings.txt: 7 accepted, 0 rejected
  connections.txt: 6 accepted, 0 rejected
  facilities.txt: 10 accepted, 0 rejected (hash capacity 11, entries 10,
  collisions 0)
  departments.txt: 7 accepted, 0 rejected


MATCHING RULES
--------------------------------------------------------------------------------
  - Names are matched without regard to capitalization (ASCII letters).
  - Whitespace around fields and typed names is trimmed; spaces inside names
    are preserved and must match exactly.
  - The spelling from the first accepted record is kept for buildings,
    categories, and departments.
  - Facility IDs are normalized to uppercase before validation. A valid ID is
    F followed by exactly three digits (for example F001).
  - A line is a comment only if its first non-whitespace character is #.


DATA FILE FORMATS
--------------------------------------------------------------------------------
Fields are separated by the pipe character ( | ), one record per line.
Blank lines and comment lines are skipped.

data/buildings.txt
  BuildingName
  Example:  Library
  Names must be unique ignoring capitalization.

data/connections.txt
  BuildingA|BuildingB
  Example:  Library|Student Union
  Both buildings must already be accepted. Self-connections and duplicates
  (including reversed duplicates such as A|B and B|A) are rejected.

data/facilities.txt
  FacilityID|FacilityName|BuildingName|Category
  Example:  F004|Computer Lab A|Science Building|Computer Lab
  The ID must be unique and the building must already be accepted. Facility
  names may repeat.

data/departments.txt
  DepartmentName|ParentName
  Example:  Computer Science|College of Engineering
  The first valid record is the root and uses NONE as its parent. Every later
  department must name a parent accepted on an earlier line.


VERSION CONTROL
--------------------------------------------------------------------------------
Developed collaboratively using Git and GitHub. Build outputs (*.exe, *.o) and
IDE folders (.vs/, .vscode/) are excluded through .gitignore.

================================================================================
