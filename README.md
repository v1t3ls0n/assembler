# 🏆 **Assembler Project**  

**🌐 Course:** [20465 – System Programming Lab](https://www.openu.ac.il/courses/20465.htm)  
**👨‍🏫 Instructor:** Danny Calfon  
**📅 Submission Date:** 28.03.2022  
**📑 Project Specification:** [docs/spec.pdf](docs/spec.pdf)  

---

## 🎯 **Project Overview**  
This project implements a fully functional **assembler** for a custom assembly language, as specified in the Open University's **System Programming Lab** course.  

### **Features**
- **Macro Expansion** – Replaces macros with their corresponding definitions.  
- **Symbol Management** – Resolves and defines labels.  
- **Machine Code Generation** – Converts assembly instructions into binary format.  
- **Auxiliary File Creation** – Produces essential `.ob`, `.ent`, and `.ext` output files.  
- **Robust Error Handling** – Detects and logs all errors at every stage (macro expansion, first pass, second pass) without premature termination.  

Designed for **efficiency and reliability**, the assembler ensures **comprehensive error detection** while maintaining structured and modular code.

---

## 📋 **Table of Contents**
- [🎯 Project Overview](#-project-overview)
- [⚙️ Implementation Details](#%EF%B8%8F-implementation-details)
  - [💡 Design and Approach](#-design-and-approach)
  - [🔧 Processing Pipeline](#-processing-pipeline)
  - [🔄 File Processing Flow](#-file-processing-flow)
- [🏗️ File Structure](#️-file-structure)
  - [📄 Main Source Files (`.c`)](#-main-source-files-c)
  - [📂 Header Files (`headers/`)](#-header-files-headers)
  - [📖 Documentation (`docs/`)](#-documentation-docs)
  - [📚 Library Files (`lib/`)](#-library-files-lib)
  - [🔢 Variables (`variables/`)](#-variables-variables)
  - [📁 Project Metadata](#-project-metadata)
  - [🧪 Test Files (`__test_files/`)](#-test-files-__test_files)
  - [🗒️ Scripts](#%EF%B8%8F-scripts)
- [🚀 Usage](#-usage)
- [⚖️ License](#️-license)

---

## ⚙️ **Implementation Details**  
The assembler follows a structured pipeline to process assembly source files, expand macros, handle symbols, and generate machine code. It ensures **efficient memory management**, **clear separation of concerns**, and **robust error detection** across multiple stages.  

### 💡 **Design and Approach**  
1. **Memory Efficiency** – Dynamically allocates memory to optimize resource usage.  
2. **Separation of Concerns** – Implements modular components with structured shared states for data management.  
3. **Abstracted Data Structures** – Uses typedefs, enums, and constants for efficient handling of assembler data.  

---

## 🔧 **Processing Pipeline**  
Each source file undergoes the following stages:

### **1️⃣ Macro Expansion**  
- Reads the `.as` source file and generates an expanded `.am` file.  
- Replaces all macro definitions with their corresponding content.  

### **2️⃣ First Pass**  
- Scans the expanded source file to determine **image sizes** (no memory allocated yet).  
- Adds all symbols to the **symbol table**.  
- Performs **syntax and semantic validation**.  

### **3️⃣ Address Adjustment** *(Before the Second Pass)*  
- Updates **data symbol addresses** in the symbol table.  
- Allocates **exact memory** needed for the final binary image.  

### **4️⃣ Second Pass**  
- Converts assembly instructions into **binary machine code**.  
- Inserts binary words into memory at their correct locations.  
- Resolves **symbol references**; logs errors for unresolved labels.  

### **5️⃣ Output File Generation**  
If no errors occur, the assembler creates:  
- **`.ob`** – Object file with assembled binary code.  
- **`.ent`** – Lists "entry" symbols.  
- **`.ext`** – Lists external references.

---

## 🔄 **File Processing Flow**  
After these five stages, the assembler moves to the next file until all inputs are processed. It then terminates.

---

## 🏗️ **File Structure**

### **📄 Main Source Files (`.c`)**

- **`compiler.c`**  
  - **Role**: Contains the main function, reads user-supplied source files, and calls `handleSingleFile()` to run the entire assembler process for each file.

- **`errors.c`**  
  - **Role**: Handles error logging (to `stderr` and `errors.log`) at every assembly stage. Provides functions like `yieldError()` and `yieldWarning()`.

- **`exportFiles.c`**  
  - **Role**: Generates the **output files** (`.ob`, `.ent`, `.ext`) once the second pass completes successfully.

- **`parse.c`**  
  - **Role**: **Central parsing logic** for both the first and second pass, calling functions in `firstRun.c` and `secondRun.c` based on global state.

- **`firstRun.c`**  
  - **Role**: Implements the **first pass** routines—verifying syntax/semantic rules, updating counters (`IC`, `DC`), and detecting errors.

- **`secondRun.c`**  
  - **Role**: Implements the **second pass**, writing machine code to memory, resolving labels, and managing external references.

- **`preProccesor.c`**  
  - **Role**: Handles **macro expansion** (parsing `.as` files and creating `.am` files). If macro syntax errors occur, logs them and stops further assembly.

- **`tables.c`**  
  - **Role**: Manages all **tables**—symbol table, macro table, and external operands. Uses a **hash table** approach for lookups/installs.

- **`helpers.c`**  
  - **Role**: Utility functions for string manipulation, trimming, cloning, and numeric conversions (e.g., binary to hex).

- **`memory.c`**  
  - **Role**: Manages **memory counters** (`IC`, `DC`) and constructs the **memory image** (instruction/data). Allocates the exact space after the first pass.

- **`operations.c`**  
  - **Role**: Stores **operation definitions** (opcodes, `funct`, addressing modes) and provides lookup methods like `getOperationByName()`.

- **`sharedStates.c`**  
  - **Role**: Maintains **global assembler state**—which pass is active, current file name, line number, etc. Provides getters/setters to ensure controlled access.

- **`utils.c`**  
  - **Role**: General **validation** helpers—detecting instructions, macros, registers, label names, etc. Used heavily by `parse.c`.

---

### **📂 Header Files (`headers/`)**

- **`data.h`**  
  - A **“master aggregator”** header that includes:
    - **`./headers/lib/lib.h`** – Standard library includes (`<stdio.h>`, `<stdlib.h>`, etc.).
    - **`./headers/variables/variables.h`** – Core typedefs, enums, flags, and states.
    - **`./headers/functions/functions.h`** – Consolidates major function prototypes (`compiler.h`, `errors.h`, `exportFiles.h`, etc.).
  - **Purpose**: Ensures a **single include** can pull in all essential definitions for the assembler.

- **`headers/functions/*.h`**  
  - **`compiler.h`** – Main compiler workflow declarations (`handleSingleFile`, `handleSourceFiles`).  
  - **`errors.h`** – Error/warning handling prototypes.  
  - **`exportFiles.h`** – Output file generation functions.  
  - **`firstRun.h`** – First pass functions.  
  - **`secondRun.h`** – Second pass functions.  
  - **`preProccesor.h`** – Macro expansion functions.  
  - **`tables.h`** – Symbol/macro table structures and management.  
  - **`helpers.h`** – String/numeric utility functions.  
  - **`memory.h`** – Memory counters, `.ob` file writing, data/instruction images.  
  - **`operation.h`** – Operation table (opcodes, `funct`, addressing modes).  
  - **`parse.h`** – Core parsing routines bridging the passes.  
  - **`sharedStates.h`** – Global state (file name, line number, pass).  
  - **`utils.h`** – Validation (label names, instructions, registers, etc.).

---

### 📖 **Documentation (`docs/`)**
- **`spec.pdf`** – Official project specification.

---

### 📚 **Library Files (`lib/`)**
- **`lib.h`** – Shared library definitions (standard includes, macros, etc.).

---

### 🔢 **Variables (`variables/`)**
- **`complex_typedef.h`** – Key structs (e.g., `BinaryWord`, `HexWord`, `Operation`, `Item`) for memory images, operations, and tables.  
- **`constants.h`** – Defines constants (`BINARY_WORD_SIZE`, `MEMORY_START`, register codes, etc.).  
- **`flags.h`** – Enums for errors, warnings, assembler states, and booleans.  
- **`variables.h`** – Aggregates these enums/structs into one header.

---

### 📁 **Project Metadata**
- **`.gitignore`** – Lists files/folders to ignore in version control.  
- **`LICENSE.md`** – MIT License details.  
- **`README.md`** – This documentation.  
- **`makefile`** – Build instructions.

---

### 🧪 **Test Files (`__test_files/`)**
- **`errors/`** – Intentional error cases.  
- **`mixed/`** – Mix of valid/invalid cases.  
- **`valid/`** – Fully valid assembler tests.  
- **`errors.log`** – Stores error logs generated during tests.

---

### 🗒️ **Scripts**
- **`runTests.sh`** – Runs assembler test cases.

---

## 🚀 **Usage**
Compile and run the assembler with:

```sh
gcc -o assembler compiler.c data.h errors.c exportFiles.c firstRun.c secondRun.c preProccesor.c tables.c helpers.c memory.c operations.c parse.c sharedStates.c utils.c
./assembler file1.as file2.as ...
```

The assembler will generate the necessary output files in the same directory as the source files.

---

# ⚖️ **License**
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.

---
****
