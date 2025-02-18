# ⚙️ Assembler – System Programming Lab Final Project

**🌐 OpenU Course Link**: [20465 – System Programming Lab](https://www.openu.ac.il/courses/20465.htm)  
**📑 Task Definition & Specification**: [docs/spec.pdf](docs/spec.pdf)  
**👨‍🏫 Course Instructor**: Danny Calfon  
**📅 Submission Date**: 28.03.2022  

#### 📝 Task Summary
Implement an assembler as outlined in docs/spec.pdf. This repository provides a complete assembler for a custom assembly language specified by the Open University's System Programming Lab CS course. 
The assembler reads assembly source files (.as), expands macros, handles symbol definitions, generates machine code, and produces auxiliary files essential for proper execution. 
It incorporates rigorous error handling at every stage—macro expansion, first pass, and second pass—to detect and log issues without terminating early, ensuring all errors are discovered.

<br>

## 📋 Table of Contents

- [🌟 Overview](#overview)
- [🏗️ File Structure](#%EF%B8%8F-file-structure)
- [🛠️ Usage](#%EF%B8%8F-usage)
- [📜 License](#-license)

<br>
<br>

## 🌟 Overview
The assembler is designed to handle assembly language files, process macros, and generate machine code. It supports multiple stages including macro expansion, first pass, and second pass to ensure the[...]
#### 📋 Implementation Overview and General Approach
#### 🎯 Objectives (Goals)
1. **Memory Efficiency**: It was important for us to allocate the memory of the binary image output dynamically so that the memory use of the program would be efficient and that the memory allocation [...]
2. **Separation of Concern**: We tried to split the program into independent components that will be responsible for well-defined tasks and that the sharing of data between files will be through some [...]
3. **Abstraction of Data Structures and Variables**: Used to fit our needs by defining different typedef structs, types, enums, and constants.
#### 🗂️ My Implementation Overview in Short
#### Stages
1. **Macros Stage**: The assembler parses the original source (.as) file and generates the expanded (.am) source file that contains the same source code content, the only difference is that the macros[...]
2. **First Run**: In the first run, the assembler counts the size of the data image and the instruction image (without allocating any memory), adds all the symbols to the symbol table, and verifies th[...]
3. **In Between the First and Second Run**: If the first run ended successfully, the assembler updates the address of each data symbol in the symbol table, then allocates the exact amount of memory ne[...]
4. **Second Run**: In the second run, the assembler writes the words of each line of code in binary format, inserts the words (binary bits) to the memory image in the correct position depending on the[...]
5. **Export (Generate Files)**: If the second run finishes without any error, the required outputs (.ob, .ext, .ent files) are generated.
#### 🔄 Next
The assembler then handles the next source file, repeating these 5 steps for each source file passed to it until the last one, then it ends the program.

<br>
<br>

## 🏗️ **File Structure**

### **📄 Main Source Files (`.c`)**
- **`compiler.c`** – Contains the main function and handles source files.
- **`errors.c`** – Handles error logging and reporting.
- **`exportFiles.c`** – Manages the creation of output files like `.ob`, `.ent`, and `.ext`.
- **`firstRun.c`** – Contains functions specifically used by parse.c for handling the first pass of the assembler.
- **`secondRun.c`** – Contains functions specifically used by parse.c for handling the second pass of the assembler.
- **`preProccesor.c`** – Handles macro expansion.
- **`tables.c`** – Handles the symbol table, macro tables, and other typedef data structures used by the assembler.
- **`helpers.c`** – Contains utility functions for string manipulation and format conversions.
- **`memory.c`** – Manages memory allocation and image generation.
- **`operations.c`** – Manages operation definitions and their properties.
- **`parse.c`** – Implements both the first and second pass of the assembler, determining the parsing stage based on state.
- **`sharedStates.c`** – Manages global states and shared information across the assembler.
- **`utils.c`** – Provides additional utility functions for validation and general handling.

---

### **📂 Header Files (`headers/`)**
- **`headers/functions/`**
  - `compiler.h` – Header for main compiler functions.
  - `errors.h` – Defines error handling methods.
  - `exportFiles.h` – Contains function prototypes for file generation.
  - `firstRun.h` – Function definitions for the first assembler pass.
  - `functions.h` – General helper function declarations.
  - `helpers.h` – Utility functions for formatting and string operations.
  - `memory.h` – Handles memory allocation and management.
  - `operation.h` – Defines supported assembler operations.
  - `parse.h` – Function definitions for parsing assembler code.
  - `preProccesor.h` – Macro expansion function declarations.
  - `secondRun.h` – Function definitions for the second assembler pass.
  - `sharedStates.h` – Manages shared state variables.
  - `tables.h` – Defines structures for symbol and macro tables.
  - `utils.h` – Additional helper functions.

---

### **📜 Documentation (`docs/`)**
- **`spec.pdf`** – The project specification file detailing requirements and design.

---

### **📚 Library Files (`lib/`)**
- **`lib.h`** – Shared library definitions.

---

### **🔢 Variables (`variables/`)**
- **`complex_typedef.h`** – Complex typedef structures.
- **`constants.h`** – Defines global constants used throughout the assembler.
- **`flags.h`** – Manages assembler flags and settings.
- **`variables.h`** – Stores global variable definitions.

---

### **🛠️ Scripts**
- **`runTests.sh`** – Runs test cases for the assembler.

---

### **📁 Project Metadata**
- **`.gitignore`** – Specifies files to ignore in version control.
- **`LICENSE.md`** – Contains the project license.
- **`README.md`** – This documentation file.
- **`makefile`** – Defines build commands for compiling the assembler.

---

### **🧪 Test Files (`__test_files/`)**
- **`errors/`** – Contains test cases with intentional errors.
- **`mixed/`** – Test cases with a combination of valid and invalid instructions.
- **`valid/`** – Fully valid assembler test cases.
- **`errors.log`** – Stores error logs from test runs.

<br>
<br>

## 🛠️ Usage

To use the assembler, compile the source code and run the executable with the assembly files as arguments:

```sh
gcc -o assembler compiler.c data.h errors.c exportFiles.c firstRun.c secondRun.c preProccesor.c tables.c helpers.c memory.c operations.c parse.c sharedStates.c utils.c
./assembler file1.as file2.as ...
```

The assembler will generate the necessary output files in the same directory as the source files.

<br>

## 📜 License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
