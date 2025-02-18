# ⚙️ **Assembler Project**  

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
- [⚙️ Implementation Details](#-implementation-details)
  - [📋 Design and Approach](#-design-and-approach)
  - [🛠️ Processing Pipeline](#-processing-pipeline)
  - [🔄 File Processing Flow](#-file-processing-flow)
- [🏗️ File Structure](#-file-structure)
  - [📄 Main Source Files (`.c`)](#-main-source-files-c)
  - [📂 Header Files (`headers/`)](#-header-files-headers)
  - [📜 Documentation (`docs/`)](#-documentation-docs)
  - [📚 Library Files (`lib/`)](#-library-files-lib)
  - [🔢 Variables (`variables/`)](#-variables-variables)
  - [🛠️ Scripts](#-scripts)
  - [📁 Project Metadata](#-project-metadata)
  - [🧪 Test Files (`__test_files/`)](#-test-files-__test_files)
- [🛠️ Usage](#-usage)
- [📜 License](#-license))


---

## ⚙️ **Implementation Details**  

The assembler follows a structured pipeline to process assembly source files, expand macros, handle symbols, and generate machine code. It ensures **efficient memory management**, **clear separation of concerns**, and **robust error detection** across multiple stages.  

### 📋 **Design and Approach**  

#### 🎯 **Key Objectives**  
1. **Memory Efficiency** – Dynamically allocates memory to optimize resource usage.  
2. **Separation of Concerns** – Implements modular components with structured shared states for data management.  
3. **Abstracted Data Structures** – Uses typedefs, enums, and constants for efficient handling of assembler data.  

---

## 🛠️ **Processing Pipeline**  

Each source file undergoes the following stages:  

### **1️⃣ Macro Expansion**  
- Reads the `.as` source file and generates an expanded `.am` file.  
- Replaces all macro definitions with their corresponding content.  

### **2️⃣ First Pass**  
- Scans the expanded source file to determine **image sizes** (without memory allocation).  
- Adds all symbols to the **symbol table**.  
- Performs **syntax and semantic validation**.  

### **3️⃣ Address Adjustment** *(Before the Second Pass)*  
- Updates **data symbol addresses** in the symbol table.  
- Allocates **exact memory** needed for the final binary image.  

### **4️⃣ Second Pass**  
- Converts assembly instructions into **binary machine code**.  
- Inserts binary words into memory at their correct locations.  
- Resolves **symbol references** and logs errors for unresolved labels.  

### **5️⃣ Output File Generation**  
If no errors are detected, the assembler generates:  
- **`.ob` (Object File)** – Assembled binary code.  
- **`.ent` (Entries File)** – Lists "entry" symbols.  
- **`.ext` (Externals File)** – Lists external references.  

---

## 🔄 **File Processing Flow**  
After completing these five steps, the assembler processes the next file until all input files are assembled. The program then terminates.

---

## 🏗️ **File Structure**

### **📄 Main Source Files (`.c`)**
- **`compiler.c`** – Handles main execution flow.  
- **`errors.c`** – Manages error logging and reporting.  
- **`exportFiles.c`** – Generates `.ob`, `.ent`, and `.ext` files.  
- **`parse.c`** – Implements **both the first and second pass**, controlling parsing state.  
- **`firstRun.c`** – Provides **helper functions** for the first pass, called by `parse.c`.  
- **`secondRun.c`** – Provides **helper functions** for the second pass, called by `parse.c`.  
- **`preProccesor.c`** – Handles macro expansion.  
- **`tables.c`** – Manages **symbol tables, macro tables, and typedef data structures**.  
- **`helpers.c`** – Provides string manipulation utilities.  
- **`memory.c`** – Handles memory allocation.  
- **`operations.c`** – Defines assembler operations.  
- **`sharedStates.c`** – Manages global assembler states.  
- **`utils.c`** – Provides validation and additional helper functions.  

---

### **📂 Header Files (`headers/`)**
Located in **`headers/functions/`**:
- `compiler.h` – Main compiler functions.  
- `errors.h` – Error handling.  
- `exportFiles.h` – File generation functions.  
- `firstRun.h` – Functions for the first assembler pass.  
- `secondRun.h` – Functions for the second assembler pass.  
- `parse.h` – Parsing function declarations.  
- `tables.h` – Symbol table and macro structure definitions.  
- `utils.h` – Additional helper functions.  

---

### **📜 Documentation (`docs/`)**
- **`spec.pdf`** – Official project specification.  

---

### **📚 Library Files (`lib/`)**
- **`lib.h`** – Shared library definitions.  

---

### **🔢 Variables (`variables/`)**
- **`constants.h`** – Global constants.  
- **`flags.h`** – Assembler flags and settings.  
- **`variables.h`** – Stores global variables.  

---

### **🛠️ Scripts**
- **`runTests.sh`** – Runs assembler test cases.  

---

### **📁 Project Metadata**
- **`.gitignore`** – Files to ignore in version control.  
- **`LICENSE.md`** – MIT License details.  
- **`README.md`** – Project documentation.  
- **`makefile`** – Compilation instructions.  

---

### **🧪 Test Files (`__test_files/`)**
- **`errors/`** – Intentional error cases.  
- **`mixed/`** – Mix of valid/invalid cases.  
- **`valid/`** – Fully valid assembler tests.  
- **`errors.log`** – Stores error logs.  

---

## 🛠️ **Usage**
To use the assembler, compile the source code and run the executable with the assembly files as arguments:

```sh
gcc -o assembler compiler.c data.h errors.c exportFiles.c firstRun.c secondRun.c preProccesor.c tables.c helpers.c memory.c operations.c parse.c sharedStates.c utils.c
./assembler file1.as file2.as ...
```

The assembler will generate the necessary output files in the same directory as the source files.

---

# 📜 License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
