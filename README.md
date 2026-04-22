MCA Assignment: Generic DS & DB System

Course: DSUC+DBMS, MCA, HBTU Kanpur

Developer: Shivkant

Instructor: Dr. Siddharth Srivastava 
🚀 Overview

This project implements a high-performance, generic system that bridges the gap between Data Structures (Main Memory) and Databases (Secondary Storage). It allows for the manipulation of semi-structured data in RAM using dynamic data structures, with the ability to persist changes back to files upon request.
🛠️ Environment & Prerequisites

To ensure full compatibility, this system was developed and tested in the following environment:

    Operating System: Alpine Linux (x86_64) running on Oracle VirtualBox.

    Compiler: gcc.

    Language: Pure C.

📂 Features

The system is designed to be completely generic, meaning it can handle any data pool (e.g., MCASampleData1, MCASampleData2, or new custom sets) regardless of the headers.
Core Operations (RAM-based)

    Sorting: Dynamic sorting based on any user-specified header.

    CRUD Operations: Efficient Insertion, Deletion, and Updates performed in main memory.

    Data Persistence: Results are only written back to persistent storage on demand.

Advanced Functionality

    Join Operations: Support for Full, Inner, and Outer Joins between data sets.

    Custom Query Language: A semi-structured query language for basic Selection, Projection, and Joins.

    Performance Metrics: Built-in execution time calculation (excluding user input time) using C libraries.

🏗️ System Architecture

The system uses a flexible architecture to remain generic.

    Note: We utilized [mention your chosen method: e.g., Linked Lists/Hash Tables with Void Pointers or Recompilation strategies] to ensure the system adapts to different headers and data types at runtime.

📖 How to Run

    Clone the repository:
    Bash

    git clone [Your-GitHub-Link]
    cd [Your-Repository-Name]

    Download Sample Data (within your Alpine VM): 
    Bash

    wget https://raw.githubusercontent.com/siddharths-del/TVP-Assignments/main/MCASampleData1.tar.gz
    wget https://raw.githubusercontent.com/siddharths-del/TVP-Assignments/main/MCASampleData2.tar.gz

    Compile and Execute:
    Bash

    gcc main.c -o system
    ./system
