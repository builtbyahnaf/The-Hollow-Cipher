# Developer Documentation: The Hollow Cipher (THC)
**Academic Project: KUET CSE 1-1 (Manual Coded)**  
**Document Version:** 1.0.0  
**Target Audience:** Junior Developers, Maintainers, Code Reviewers  
**Language / Environment:** C (C99 / C11 standard), GCC (MinGW for Windows), Make  

---

## Table of Contents
1. [Project Overview & Game Concept](#1-project-overview--game-concept)
2. [Repository Architecture & Directory Layout](#2-repository-architecture--directory-layout)
3. [System Architecture & Execution Flow](#3-system-architecture--execution-flow)
4. [Module-by-Module Code Analysis](#4-module-by-module-code-analysis)
   - [4.1 Main Entry & Flow Control (`main.c`, `level_select.c`)](#41-main-entry--flow-control-mainc-level_selectc)
   - [4.2 Game State & Progress Tracking (`progress.c`)](#42-game-state--progress-tracking-progressc)
   - [4.3 Evidence Parser (`evidence_parser.c`)](#43-evidence-parser-evidence_parserc)
   - [4.4 Interrogation Parser (`interrogation_parser.c`)](#44-interrogation-parser-interrogation_parserc)
   - [4.5 Past History Parser (`past_history_parser.c`)](#45-past-history-parser-past_history_parserc)
   - [4.6 Answer Validator & Solution Engine (`ans_validator.c`)](#46-answer-validator--solution-engine-ans_validatorc)
   - [4.7 Vault Security & Procedural Generator (`vault_pass.c`)](#47-vault-security--procedural-generator-vault_passc)
   - [4.8 Admin Subsystem (`admin/`)](#48-admin-subsystem-admin)
5. [C Standard Library Deep Dive (Junior Dev Masterclass)](#5-c-standard-library-deep-dive-junior-dev-masterclass)
   - [`strcspn` — String Complement Span](#51-strcspn--string-complement-span)
   - [`memmove` vs `memcpy` — Overlapping Memory Safety](#52-memmove-vs-memcpy--overlapping-memory-safety)
   - [`malloc`, `realloc`, `calloc`, `free` — Dynamic Heap Management](#53-malloc-realloc-calloc-free--dynamic-heap-management)
   - [`rewind` vs `fseek` — File Pointer Manipulation](#54-rewind-vs-fseek--file-pointer-manipulation)
   - [Additional Standard Library Functions in the Codebase](#55-additional-standard-library-functions-in-the-codebase)
6. [Syntax Patterns, Idioms & Low-Level C Mechanics](#6-syntax-patterns-idioms--low-level-c-mechanics)
   - [Struct Bit-Fields](#61-struct-bit-fields)
   - [The `goto RETRY` Pattern vs Loop Constructs](#62-the-goto-retry-pattern-vs-loop-constructs)
   - [Dynamic Buffer Exponential Growth (Amortized $O(1)$)](#63-dynamic-buffer-exponential-growth-amortized-o1)
   - [Two-Pointer In-Place String Trimming](#64-two-pointer-in-place-string-trimming)
   - [File Signature Verification Pattern](#65-file-signature-verification-pattern)
7. [The Makefile Demystified: Build System Breakdown](#7-the-makefile-demystified-build-system-breakdown)
   - [Line-by-Line Breakdown](#71-line-by-line-breakdown)
   - [Variables & Automatic Variables (`$@`, `$<`)](#72-variables--automatic-variables)
   - [Pattern Rules & Path Substitution](#73-pattern-rules--path-substitution)
   - [Windows Shell Compatibility Considerations](#74-windows-shell-compatibility-considerations)
8. [Code Review, Edge Cases & Roadmap for Juniors](#8-code-review-edge-cases--roadmap-for-juniors)

---

## 1. Project Overview & Game Concept

**The Hollow Cipher (THC)** is a detective mystery / investigative text-based puzzle game developed in procedural C. The project simulates a noir-style investigative environment where players take on the role of an investigator solving intricate criminal cases (e.g., *Level 1: The Silent Patient*).

### Gameplay Core Mechanics:
- **Case Dossier Reading:** Inspecting evidence items, suspect interrogation transcripts, and background histories from formatted text files.
- **Deduction & Solution Checking:** Players analyze contradictions, deduce the true culprit, and input their verdict. The engine parses the user response, performs case-insensitive and whitespace-tolerant matching against encrypted or plain solution files, and reveals the resolution.
- **Vault Mini-Game / Security Mechanics:** Cryptographic or procedural numerical passcodes generated via bitwise bit-shifting and mask manipulations.
- **Persistent Progress:** Tracking unlocked cases through a proprietary flat-file database format (`.thcdb`) with file header signature validation.
- **Backstage Administrator Suite:** A command-line administration suite providing administrative functions: editing level data directly in external editors (Windows Notepad), maintaining a reverse-chronological audit log of edits, and credentials management.

---

## 2. Repository Architecture & Directory Layout

The workspace is organized into modular functional units separating source files, public header declarations, game data, and build artifacts:

```
THC Manual Coded/
├── .vscode/                     # VS Code debug configurations & compiler paths
├── admin/                       # Administration subsystem
│   ├── admin_panel.c            # Admin console menu and navigation
│   ├── cmd_edit.c               # External editor launcher & audit log generator
│   ├── credentials_manager.c    # Password check, update, and changelog tracker
│   └── log_parser.c             # Audit log viewer
├── data/                        # Persistent storage and level assets
│   ├── credentials.thcdb        # Admin credentials database
│   ├── edit_log.thcdb           # Audit trail of level file modifications
│   ├── progress.thcdb          # Game progression state (unlocked levels)
│   └── level_1/                 # Level 1 case files ("The Silent Patient")
│       ├── evidence.txt         # Numbered list of physical evidence
│       ├── interrogation.txt    # Suspect dialogue and interview transcripts
│       ├── past_history.txt     # Criminal records and character background
│       ├── solution.txt         # Official culprit and case explanation
│       └── statement.txt        # Initial police report / case statement
├── include/                     # C Header files (.h) with prototypes & typedefs
│   ├── admin_panel.h
│   ├── ans_validator.h
│   ├── cmd_edit.h
│   ├── credentials_manager.h
│   ├── evidence_parser.h
│   ├── interrogation_parser.h
│   ├── level_select.h
│   ├── log_parser.h
│   ├── past_history_parser.h
│   ├── progress.h
│   └── vault_pass.h
├── obj/                         # Object files (.o) generated during compilation
│   └── admin/
├── ans_validator.c              # Core game: Answer parsing and validation engine
├── evidence_parser.c            # Core game: Evidence file text parsing & storage
├── interrogation_parser.c       # Core game: Suspect statement parser
├── level_select.c               # Player menu: Level selection interface
├── main.c                       # Application entry point (CLI routing & main menu)
├── Makefile                     # GNU Make build script for compiling the binary
├── past_history_parser.c        # Core game: Suspect background history parser
├── progress.c                   # Core game: Save state management
└── vault_pass.c                 # Core game: Procedural passcode generator
```

---

## 3. System Architecture & Execution Flow

The following sequence and state diagrams illustrate how the engine routes control from program launch through gameplay or administration:

```mermaid
flowchart TD
    Start([main: argc, argv]) --> AdminCheck{argv[1] == 'admin_mode'<br/>&& argv[2] == 'admin123'?}
    
    AdminCheck -- Yes --> AdminPanel[admin/admin_panel.c: show_panel]
    AdminCheck -- No --> MainMenu[main.c: Main Menu]
    
    AdminPanel --> AdminMenu{Admin Options}
    AdminMenu -->|E| EditLevel[cmd_edit.c: select_level & open_editor]
    AdminMenu -->|L| ViewLogs[log_parser.c: view_edit_logs]
    AdminMenu -->|P| ChangePass[credentials_manager.c: set_new_pass]
    AdminMenu -->|X| ExitAdmin([Exit Admin])
    
    EditLevel --> LaunchNotepad[notepad.exe filepath]
    LaunchNotepad --> WriteAuditLog[cmd_edit.c: write_log -> edit_log.thcdb]

    MainMenu --> MenuSelect{Menu Selection}
    MenuSelect -->|P| LevelSelect[level_select.c: show_level_selection]
    MenuSelect -->|O| Options[Options - Stub]
    MenuSelect -->|A| About[About & Credits - Stub]
    MenuSelect -->|E| Terminate([Exit Game])
    
    LevelSelect --> ParseData[Parsers: evidence, interrogation, past_history]
    ParseData --> Investigate[Player Analysis]
    Investigate --> ValidateAnswer[ans_validator.c: is_ans_correct]
    ValidateAnswer -->|Correct| UnlockNext[progress.c: set_level_unlocked]
    ValidateAnswer -->|Incorrect| RetryAnswer[Try Again / Return]
```

---

## 4. Module-by-Module Code Analysis

### 4.1 Main Entry & Flow Control (`main.c`, `level_select.c`)

#### `main.c`
- **Role:** Handles application bootstrapping, command-line argument evaluation, and top-level user navigation.
- **Command-Line Administrative Trap:**
  ```c
  if (argc >= 3 && strcmp(argv[1], "admin_mode") == 0 && strcmp(argv[2], "admin123") == 0) {
      show_panel();
      return 0;
  } else {
      printf("Invalid parameters!\n");
  }
  ```
  If run with `thc.exe admin_mode admin123`, the program jumps directly into administrative mode and exits upon completion. If arguments are missing or incorrect, it outputs `"Invalid parameters!"` and falls through to the standard game main menu.
- **Menu Handling:** Uses `scanf("%c", &selection)` to read character commands (`'P'`, `'O'`, `'A'`, `'E'`).
- **Input Validation Pattern:** Checks character range (`'A'` through `'z'`) and converts uppercase characters to lowercase via `tolower()` from `<ctype.h>`. If input fails validation, control branches back via `goto RETRY`.
- **Screen Clearing:** Invokes `system("cls")` to clear the Windows console before rendering new submenus.

#### `level_select.c`
- **Role:** Displays available chapters/cases to the player.
- **Key Functions:**
  - `void show_level_selection()`: Displays cases (`LEVEL 1: THE SILENT PATIENT`, `LEVEL 2: THE MISSING NOTEBOOK`). Prompts for an integer level choice (`1-3`), validating the lower and upper bounds using a `goto RETRY` construct on invalid numbers.

---

### 4.2 Game State & Progress Tracking (`progress.c`)

- **Role:** Manages persistent progress storage in `data/progress.thcdb`.
- **Database Signature:**
  ```c
  bool is_signature_valid(FILE *pfile) {
      char signature[10];
      if (fscanf(pfile, "%9s", signature) != 1) return false;
      return strcmp(signature, "THCDB_V1") == 0;
  }
  ```
  Ensures that the opened database file matches the format version `THCDB_V1` before reading records, guarding against format incompatibility or corrupted save files.
- **State Storage:**
  Maintains an internal static array:
  ```c
  #define MAX_LEVELS 2
  static bool level_states[MAX_LEVELS] = {false};
  ```
- **Read & Write Operations:**
  - `load_progress()`: Opens `data/progress.thcdb` in read mode (`"r"`), verifies the file signature, then iterates through index-state pairs using `fscanf(pfile, "%d %d", &level_index, &state)`. Level 0 is always enforced as unlocked (`level_states[0] = true`).
  - `is_level_unlocked(int level_index)`: Bounds-checked accessor returning whether a given level index is accessible.
  - `set_level_unlocked(int level_index)`: Sets the flag to `true`, then rewrites `data/progress.thcdb` starting with the `THCDB_V1` header followed by all level indices and their state (`1` or `0`).

---

### 4.3 Evidence Parser (`evidence_parser.c`)

- **Role:** Dynamically loads and structures clues from `evidence.txt`.
- **Data Structure:**
  ```c
  typedef struct {
      char *name;
      char *description;
  } Evidence;
  ```
- **Parsing Mechanics:**
  1. `is_evidence_title(const char *line)`: Detects title lines formatted as numbers followed by a period (e.g., `1. The Murder Knife`).
  2. Splits the title at the period character using `strchr(line, '.')`, skips leading whitespace, and allocates heap memory for the name.
  3. `realloc(evidence_list, (total_evidence + 1) * sizeof(Evidence))`: Expands the global dynamic array of clues by 1 entry each time a new title is detected.
  4. Concatenates subsequent non-empty lines into `description` by reallocating `current->description` to hold `old_len + line_len + 2` bytes (accommodating a newline delimiter and the null terminator `\0`), appending the new line using `memcpy`.
- **Memory Cleanup:**
  - `void free_evidence(void)`: Iterates through each index, freeing both `name` and `description` heap buffers, then frees `evidence_list` and sets the pointer to `NULL`.

---

### 4.4 Interrogation Parser (`interrogation_parser.c`)

- **Role:** Parses multi-line suspect interview logs from `interrogation.txt`.
- **Data Structure:**
  ```c
  typedef struct {
      char *name;
      char *statement;
  } SuspectStatements;
  ```
- **Parsing Logic:**
  - Identifies suspect header tags using `is_suspect_header()`:
    ```c
    return line[0] == '[' && strchr(line, ']') != NULL && strcmp(line, "[Interrogation]") != 0;
    ```
    This identifies lines enclosed in brackets `[...]` while ignoring the generic file header `[Interrogation]`.
  - Replaces the closing bracket `]` with `\0` in-place, creating a clean substring for the suspect's name.
  - Dynamically resizes the `statements` array via `realloc()`.
  - Appends subsequent dialogue lines to `statements[i].statement`, inserting `\n` linebreaks between paragraphs.
  - Exposes `statements_count()` and `free_statements()` for clean lifecycle management.

---

### 4.5 Past History Parser (`past_history_parser.c`)

- **Role:** Extracts background files, motives, and character histories from `past_history.txt`.
- **Data Structure:**
  ```c
  typedef struct {
      char *name;
      char *history;
  } PastHistory;
  ```
- **Parsing Logic:**
  - Scans for bracketed tags `[Name]`.
  - Explicitly filters out structural section headers:
    ```c
    if (strcmp(name, "Past History") == 0 || strcmp(name, "Investigative Direction") == 0) {
        continue;
    }
    ```
  - Allocates `history` initially with `calloc(1, sizeof(char))` (representing an empty, null-terminated string `""`).
  - Appends historical narrative lines using `realloc()` and `memcpy()`.
  - Exposes `past_history_count()` and `free_past_history()`.

---

### 4.6 Answer Validator & Solution Engine (`ans_validator.c`)

- **Role:** Validates player deductions and dynamically loads case solutions.
- **Key Functions:**
  1. `bool is_ans_correct(char *response, FILE *solution_path)`:
     - Calls `rewind(solution_path)` to ensure reading starts from byte 0.
     - Scans line by line until it locates the `[Answer]` section header.
     - Reads the immediately following line containing the official solution.
     - Strips trailing `\r\n` line breaks using `strcspn(line, "\r\n") = '\0'`.
     - **Bidirectional Whitespace Stripping:** Strips leading spaces by advancing pointer `response++` / `correct++`, and strips trailing spaces by adjusting length and inserting null characters `\0`.
     - **Case-Insensitive Character Comparison:**
       ```c
       for (size_t i = 0; i < response_len; i++) {
           if (tolower((unsigned char)response[i]) != tolower((unsigned char)correct[i])) {
               return false;
           }
       }
       ```
       Ensures answers like `"arindom kumar biswas"`, `"Arindom Kumar Biswas"`, and `"   ARINDOM KUMAR BISWAS  "` are all evaluated as correct.

  2. `char *parse_solution_desc(FILE *sol)`:
     - Dynamically reads the entirety of the solution file into a growing heap buffer.
     - Implements dynamic capacity doubling (`capacity = capacity * 2`), starting at 500 bytes and doubling as needed with `realloc`.
     - Enforces a safety cutoff limit of `1,000,000 / 2` bytes (500 KB) to prevent unbounded memory allocation from corrupted or malicious files.
     - Appends a final `'\0'` and returns the heap buffer, which the caller is responsible for freeing.

---

### 4.7 Vault Security & Procedural Generator (`vault_pass.c`)

- **Role:** Implements a numerical vault locking mechanism for investigative secrets.
- **Procedural Code Generation Algorithm:**
  ```c
  int generate_pass(void) {
      long seed = 10000000L + rand() % 90000000L;

      long step1 = seed >> 3;
      long step2 = step1 << 2;
      long step3 = step2 ^ 0xABCD;
      long step4 = step3 & 0x7FFFFFFF;

      int result = 10000 + (int)(step4 % 90000);
      return result;
  }
  ```
  - **Bitwise Shifting (`>> 3`, `<< 2`):** Discards the 3 least significant bits and shifts left by 2 bits, introducing mathematical bit diffusion.
  - **XOR Masking (`^ 0xABCD`):** Inverts specific bits according to the hexadecimal pattern `0xABCD` (`1010 1011 1100 1101` in binary).
  - **Bitwise AND Mask (`& 0x7FFFFFFF`):** Clears the sign bit, guaranteeing the intermediate number is non-negative.
  - **Modular Arithmetic (`10000 + (step4 % 90000)`):** Guarantees the output is strictly a 5-digit integer between `10000` and `99999`.
- **Vault State Management:**
  - `check_vault_unlocked(FILE *file)`: Reads an integer status flag from the file; returns `true` if `1`.
  - `set_vault_unlocked(FILE *file)`: Rewinds the file and writes `"1\n"`, followed by `fflush(file)` to force immediate flushing of the internal buffer to disk.

---

### 4.8 Admin Subsystem (`admin/`)

The `admin/` module provides backstage tooling for managing game assets, credentials, and audit tracking:

- **`admin_panel.c` (`show_panel`):**
  Renders the administrator control board (`EDIT LEVELS [E]`, `VIEW EDIT LOGS [L]`, `CHANGE ADMIN PASSWORD [P]`, `EXIT [X]`).
- **`cmd_edit.c`:**
  - Bridges the C runtime with the Windows environment using `system("notepad.exe <filepath>")`.
  - Uses an `enum OPTIONS` mapping component choices (`EVIDENCE = 11`, `INTERROGATION = 22`, `PAST_HISTORY = 33`, `SOLUTION = 44`, `STATEMENT = 55`) to their corresponding filenames on disk.
  - **Prepend Audit Logging (`write_log`):**
    When a file is edited, it captures the current system timestamp via `time()` and `strftime()`, reads the existing contents of `data/edit_log.thcdb` into memory, and writes the **newest log at the top** (reverse chronological order), followed by the historical entries.
- **`credentials_manager.c`:**
  - `is_pass_valid()`: Compares user input against the password on line 1 of `data/credentials.thcdb`.
  - `set_new_pass()`: Prompts for a new password twice, validates match and non-emptiness, then writes the password and an update timestamp (`LAST_CHANGED: ...`) back to disk.
  - `pass_changelog()`: Reads and displays the last modification timestamp.
- **`log_parser.c` (`view_edit_logs`):**
  Reads `data/edit_log.thcdb` line-by-line using `fgets()` and prints numbered audit entries (`[1] Edited ...`) to the terminal.

---

## 5. C Standard Library Deep Dive (Junior Dev Masterclass)

This section details the primary C standard library functions utilized throughout this codebase, explaining their behavior, internal mechanics, common pitfalls, and exact usage in this project.

---

### 5.1 `strcspn` — String Complement Span

#### Prototype:
```c
#include <string.h>
size_t strcspn(const char *s, const char *reject);
```

#### What It Does:
Calculates the length of the initial segment of string `s` which consists entirely of characters **not** present in the `reject` string. In plain terms: it searches `s` from left to right and returns the index of the **first** character that appears in `reject`. If none of the rejected characters are found, it returns the length of the string `strlen(s)`.

#### Usage in this Codebase:
Found in `ans_validator.c`, `evidence_parser.c`, `interrogation_parser.c`, and `past_history_parser.c`:
```c
line[strcspn(line, "\r\n")] = '\0';
```

#### Why This Idiom is Used:
When reading text files across different operating systems, newline conventions differ:
- **Windows (CRLF):** `"\r\n"` (0x0D, 0x0A)
- **Linux/Unix (LF):** `"\n"` (0x0A)
- **Old Mac (CR):** `"\r"` (0x0D)

`fgets()` retains the newline character in the destination buffer. If you only look for `\n` on a Windows file, a trailing `\r` remains attached to the end of your string, causing `strcmp("Answer", "Answer\r")` to fail unexpectedly!

By passing `"\r\n"` as the `reject` set:
1. If `\r` is encountered first (as in Windows `\r\n`), `strcspn` returns the index of `\r`.
2. If `\n` is encountered first (as in Unix `\n`), it returns the index of `\n`.
3. If neither is found, it returns the null terminator's index.
Replacing that position with `'\0'` safely strips any newline format in a single, portable line of code.

---

### 5.2 `memmove` vs `memcpy` — Overlapping Memory Safety

#### Prototypes:
```c
#include <string.h>
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
```

#### What They Do:
Both functions copy `n` bytes from `src` to `dest`.
- **`memcpy`** assumes that `src` and `dest` occupy completely disjoint (non-overlapping) memory regions. If they overlap, its behavior is **undefined** according to the C standard (CPU vectorization or register copy order can overwrite bytes before they are read).
- **`memmove`** explicitly guarantees correct copying even when `src` and `dest` **overlap**. It accomplishes this by conceptually copying the bytes into a temporary buffer first (or internally checking if `dest < src` and copying forward, or `dest > src` and copying backward).

#### Usage in this Codebase:
In `evidence_parser.c`, `interrogation_parser.c`, and `past_history_parser.c`:
```c
static void trim_spaces(char *text)
{
    char *start = text;
    while (isspace((unsigned char)*start))
        start++;

    if (start != text)
        memmove(text, start, strlen(start) + 1); // <--- OVERLAPPING REGIONS!
    ...
}
```

#### Why `memmove` is Mandatory Here:
Consider the string `"  HELLO"` stored in buffer `text`:
```
Index:    0    1    2    3    4    5    6    7
Value:   ' '  ' '  'H'  'E'  'L'  'L'  'O'  '\0'
         ^         ^
        text     start
```
Here, `dest` is `text` (index 0) and `src` is `start` (index 2). The source and destination ranges overlap:
- `dest` covers indices `[0 .. 5]`
- `src` covers indices `[2 .. 7]`

If you used `memcpy(text, start, ...)`, copying byte-by-byte from left to right could overwrite characters in `src` before they are read. `memmove` detects this overlap and safely shifts the memory leftward, producing `"HELLO"` starting at index 0.

> [!IMPORTANT]
> Always use `memmove` whenever shifting substrings within the same buffer. Use `memcpy` only when copying between completely separate memory allocations.

---

### 5.3 `malloc`, `realloc`, `calloc`, `free` — Dynamic Heap Management

#### Prototypes:
```c
#include <stdlib.h>
void *malloc(size_t size);
void *calloc(size_t num, size_t size);
void *realloc(void *ptr, size_t new_size);
void  free(void *ptr);
```

#### How Heap Allocation Works:
- **Stack memory:** Automatic storage duration, limited size (typically 1-8 MB), managed automatically by function stack frames.
- **Heap memory:** Dynamic storage duration, managed explicitly by the programmer via runtime requests to the operating system's memory allocator.

#### Usage in this Codebase:

1. **`malloc` (Memory Allocation):**
   Allocates `size` uninitialized bytes on the heap.
   Used in `copy_text()`:
   ```c
   char *copy = malloc(strlen(text) + 1);
   if (copy != NULL) strcpy(copy, text);
   ```
   *Junior note:* Always add `+ 1` to `strlen()` to allocate space for the terminating null character `'\0'`.

2. **`calloc` (Contiguous Allocation):**
   Allocates `num * size` bytes and initializes every bit to zero (`0`).
   Used in `past_history_parser.c`:
   ```c
   history_list[total_history].history = calloc(1, sizeof(char));
   ```
   This creates an empty string where the first byte is guaranteed to be `\0`.

3. **`realloc` (Re-allocation):**
   Resizes an existing heap allocation pointed to by `ptr` to `new_size` bytes. If there is enough contiguous space at the current memory block, it extends it in-place. If not, it allocates a new block elsewhere, copies the existing contents over, frees the old block automatically, and returns the new pointer.

   **The Safe `realloc` Idiom Used in the Code:**
   ```c
   Evidence *temp = realloc(evidence_list, (total_evidence + 1) * sizeof(Evidence));
   if (temp == NULL) {
       free_evidence();
       return NULL;
   }
   evidence_list = temp;
   ```
   > [!WARNING]
   > Never write `evidence_list = realloc(evidence_list, ...)` directly!
   > If `realloc()` fails, it returns `NULL`, but the original memory block remains allocated on the heap. Overwriting your only pointer to that block with `NULL` creates an irrecoverable memory leak. Always assign the result to a temporary pointer (`temp`) first.

4. **`free` (Deallocation):**
   Releases heap memory back to the memory allocator.
   Notice the deep free pattern in `free_evidence()`:
   ```c
   for (int i = 0; i < total_evidence; i++) {
       free(evidence_list[i].name);        // Free child buffer
       free(evidence_list[i].description); // Free child buffer
   }
   free(evidence_list);                   // Free parent array
   evidence_list = NULL;                  // Prevent dangling pointer
   ```

---

### 5.4 `rewind` vs `fseek` — File Pointer Manipulation

#### Prototypes:
```c
#include <stdio.h>
void rewind(FILE *stream);
int  fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
```

#### What `rewind` Does:
`rewind(stream)` sets the file position indicator for `stream` to the beginning of the file. It is equivalent to:
```c
(void)fseek(stream, 0L, SEEK_SET);
clearerr(stream);
```
Crucially, `rewind` also clears the end-of-file (`EOF`) and error flags on the stream.

#### Usage in this Codebase:
- In `ans_validator.c`: `rewind(solution_path)` is executed before scanning for `[Answer]`. This ensures that even if another function previously read from `solution_path`, the parser begins reading from byte 0.
- In `cmd_edit.c`:
  ```c
  fseek(log_file, 0, SEEK_END);      // Move cursor to file end
  existing_size = ftell(log_file);   // Get file size in bytes
  rewind(log_file);                  // Reset cursor back to start
  ```
  Here, `fseek` and `ftell` measure the exact byte size of the log file so the program can allocate a heap buffer (`malloc(existing_size + 1)`), after which `rewind` resets the stream so `fread` can read the data.

---

### 5.5 Additional Standard Library Functions in the Codebase

| Function | Header | Code Location | Purpose & Mechanics |
| :--- | :--- | :--- | :--- |
| `fgets` | `<stdio.h>` | Parsers, Credentials | Safely reads up to $N-1$ characters from a stream into a buffer, stopping at a newline or EOF. Prevents buffer overflows compared to deprecated `gets()`. |
| `fgetc` | `<stdio.h>` | `ans_validator.c` | Reads a single unsigned character converted to an `int` from the stream. Returns `EOF` (-1) upon reaching end-of-file. |
| `snprintf` | `<stdio.h>` | `cmd_edit.c` | Formats output into a fixed-size char buffer with guaranteed null-termination and bounds truncation safety. |
| `system` | `<stdlib.h>` | `main.c`, `cmd_edit.c` | Hands a command string to the host operating system's command processor (`cls` to clear console, `notepad.exe` to open file). |
| `time`, `localtime`, `strftime` | `<time.h>` | `cmd_edit.c`, `credentials_manager.c` | `time()` queries the Unix epoch timestamp; `localtime()` converts it into a `struct tm` breakdown (day, month, year); `strftime()` formats it into human-readable strings. |
| `isspace`, `isdigit`, `tolower` | `<ctype.h>` | Parsers, Menus | Character inspection and manipulation. Always cast arguments to `(unsigned char)` when passing to `ctype.h` functions to avoid undefined behavior with negative `char` values. |
| `fflush` | `<stdio.h>` | `vault_pass.c` | Forces any unwritten buffered data in the user-space stream buffer to be written immediately to the underlying file descriptor. |

---

## 6. Syntax Patterns, Idioms & Low-Level C Mechanics

### 6.1 Struct Bit-Fields

In `admin/cmd_edit.c`, you will encounter this struct definition:

```c
typedef struct {
    // Holds for only 0-63. Which is enough for here.
    int selected_menu : 6;

    // WARNING: Holds only 0-2. TODO: Replace later to extend compatibility
    int selected_level : 2;
} EditSelection;
```

#### What is this syntax?
The `: 6` and `: 2` syntax declares **bit-fields**. It instructs the C compiler to allocate only a specific number of binary bits for that member within the enclosing memory word, rather than allocating a full 32-bit (4-byte) integer:
- `selected_menu : 6` uses exactly 6 bits. Since $2^6 = 64$, it can store values from `0` to `63` (if unsigned) or `-32` to `31` (if signed).
- `selected_level : 2` uses only 2 bits ($2^2 = 4$).

#### Junior Dev Warning:
In standard C, plain `int` in a bitfield is implementation-defined whether it is signed or unsigned. If signed, a 2-bit field has a range of `-2` to `+1` in two's complement! Writing `2` into a 2-bit signed integer can cause an arithmetic overflow (wrapping to `-2`).
> [!TIP]
> For bit-fields representing small non-negative counters or IDs, always explicitly declare them as `unsigned int`:
> ```c
> unsigned int selected_menu : 6;
> unsigned int selected_level : 2;
> ```

---

### 6.2 The `goto RETRY` Pattern vs Loop Constructs

In `main.c`, `level_select.c`, and `admin_panel.c`, input validation is structured using labels and `goto`:

```c
RETRY:
printf("ENTER SELECTION: ");
scanf("%c", &selection);

if (selection < 'A' || selection > 'z') {
    printf("Invalid selection. Please try again.\n");
    goto RETRY;
}
```

#### Why did the author write this?
In early C programming and academic assignments, `goto` is often used to jump backward to repeat an input prompt upon receiving invalid user data.

#### How to refactor to modern C idioms:
While legal in C, jumping backwards with `goto` can lead to spaghetti code in larger programs. The idiomatic alternative is an explicit loop:
```c
do {
    printf("ENTER SELECTION: ");
    if (scanf(" %c", &selection) != 1) {
        // Handle input failure
        continue;
    }
    if (selection >= 'A' && selection <= 'Z') {
        selection = tolower((unsigned char)selection);
    }
    if (selection == 'p' || selection == 'o' || selection == 'a' || selection == 'e') {
        break; // Valid input received
    }
    printf("Invalid selection. Please try again.\n");
} while (1);
```

---

### 6.3 Dynamic Buffer Exponential Growth (Amortized $O(1)$)

In `ans_validator.c` (`parse_solution_desc`), characters are read dynamically from a file without knowing the file size in advance:

```c
int capacity = 500;
int length = 0;
char *buffer = malloc((size_t)capacity);

while ((ch = fgetc(sol)) != EOF) {
    if (length + 1 >= capacity) {
        int new_capacity = capacity * 2; // <-- EXPONENTIAL DOUBLING
        char *temp = realloc(buffer, (size_t)new_capacity);
        if (temp == NULL) { free(buffer); return NULL; }
        buffer = temp;
        capacity = new_capacity;
    }
    buffer[length++] = (char)ch;
}
buffer[length] = '\0';
```

#### Why double the capacity?
If you grow the buffer by adding just 1 byte each time (`capacity + 1`), reading an $N$-byte file requires $N$ reallocations and copies, resulting in $O(N^2)$ quadratic time complexity.
By **doubling** the capacity (`capacity * 2`), reallocations occur exponentially less frequently ($1, 2, 4, 8, \dots$). The total copy work across all steps forms a geometric series bounded by $2N$, achieving an optimal **amortized $O(1)$** insertion time per character.

---

### 6.4 Two-Pointer In-Place String Trimming

The `trim_spaces()` function implemented across the parser modules uses a two-pointer technique to strip whitespace without allocating secondary buffers:

```c
static void trim_spaces(char *text)
{
    // Step 1: Scan forward past leading whitespace
    char *start = text;
    while (isspace((unsigned char)*start))
        start++;

    // Step 2: Shift substring leftward to start of buffer
    if (start != text)
        memmove(text, start, strlen(start) + 1);

    // Step 3: Chop trailing whitespace from the end
    size_t len = strlen(text);
    while (len > 0 && isspace((unsigned char)text[len - 1]))
        text[--len] = '\0';
}
```
1. `start` advances past initial whitespace characters.
2. `memmove` shifts the valid content (including the `\0` terminator) back to the base pointer `text`.
3. The trailing loop steps backward from `len - 1`, writing `'\0'` over spaces until a non-space character is reached.

---

### 6.5 File Signature Verification Pattern

In `progress.c`, `is_signature_valid()` checks for `"THCDB_V1"` before processing records:
- This is a standard database engineering practice known as a **Magic Number / File Signature**.
- It prevents the program from attempting to parse unrelated text files or corrupted databases, which could lead to segmentation faults or logic errors.

---

## 7. The Makefile Demystified: Build System Breakdown

The repository includes a root `Makefile` configured for compiling the project using GCC on Windows.

### 7.1 Line-by-Line Breakdown

```makefile
1:  CC = gcc
2:  CFLAGS = -Wall -Wextra -Iinclude
3:  
4:  TARGET = thc.exe
5:  SRC = main.c level_select.c progress.c admin/admin_panel.c admin/cmd_edit.c admin/log_parser.c admin/credentials_manager.c
6:  OBJ = $(SRC:%.c=obj/%.o)
7:  
8:  .PHONY: all clean
9:  
10: all: $(TARGET)
11: 
12: $(TARGET): $(OBJ)
13: 	$(CC) $(OBJ) -o $(TARGET)
14: 
15: obj/%.o: %.c
16: 	@if not exist obj mkdir obj
17: 	$(CC) $(CFLAGS) -c $< -o $@
18: 
19: clean:
20: 	@if exist obj rmdir /s /q obj
21: 	@if exist $(TARGET) del $(TARGET)
```

| Line | Code | Explanation |
| :--- | :--- | :--- |
| **1** | `CC = gcc` | Defines the C compiler executable. Can be overridden on the command line (e.g. `make CC=clang`). |
| **2** | `CFLAGS = -Wall -Wextra -Iinclude` | Compilation flags passed to GCC:<br>• `-Wall`: Enables all standard compiler warnings.<br>• `-Wextra`: Enables additional diagnostic warnings.<br>• `-Iinclude`: Adds the `include/` directory to the preprocessor search path for header files. |
| **4** | `TARGET = thc.exe` | Name of the final executable binary to produce. |
| **5** | `SRC = main.c ...` | Space-separated list of all C source implementation files to compile. |
| **6** | `OBJ = $(SRC:%.c=obj/%.o)` | **Substitution Reference:** Transforms each `.c` filename in `SRC` into its corresponding object file located inside the `obj/` folder (e.g., `main.c` $\to$ `obj/main.o`, `admin/cmd_edit.c` $\to$ `obj/admin/cmd_edit.o`). |
| **8** | `.PHONY: all clean clean-obj debug build-debug run` | Informs Make that `all`, `clean`, `clean-obj`, `debug`, `build-debug`, and `run` are **actions/commands**, not actual files on disk. |
| **10** | `all: $(TARGET)` | Default rule when running `make` with no arguments. It depends on `$(TARGET)`. |
| **Debug** | `build-debug`<br>`debug` | • `make build-debug`: Cleans objects and compiles with `-g -O0 -DTHC_DEBUG=1 -DDEBUG` without running.<br>• `make debug`: Compiles with debug flags and runs `thc.exe` to trigger debug diagnostics. |
| **Run** | `run: $(TARGET)` | Builds (if needed) and executes the executable. |
| **12-13**| `$(TARGET): $(OBJ)`<br>`$(CC) $(OBJ) -o $(TARGET)` | **Linking Phase:** Takes all compiled object files (`.o`) and invokes GCC to link them together into the final executable `thc.exe`. |
| **15-17**| `obj/%.o: %.c`<br>`@if not exist obj mkdir obj`<br>`$(CC) $(CFLAGS) -c $< -o $@` | **Pattern Compilation Rule:** Converts any `.c` source file into an object file (`.o`). Runs Windows command `mkdir obj` if the directory does not exist. `-c` instructs GCC to compile only without linking. |
| **19-21**| `clean:`<br>`@if exist obj rmdir /s /q obj`<br>`@if exist $(TARGET) del $(TARGET)` | Deletes the `obj/` build artifacts directory and the compiled executable to force a clean rebuild. |

---

### 7.2 Variables & Automatic Variables

Inside rule recipes, Make provides special **automatic variables**:
- **`$@` (Target):** Evaluates to the name of the target being generated.
  - In line 17: `-o $@` resolves to `-o obj/main.o`.
- **`$<` (First Prerequisite):** Evaluates to the source dependency that triggered the rule.
  - In line 17: `-c $<` resolves to `-c main.c`.
- **`$^` (All Prerequisites):** Resolves to all dependencies listed for the target (e.g., in line 13, `$(OBJ)` could also be written as `$^`).

---

### 7.3 Pattern Rules & Path Substitution

The expression `$(SRC:%.c=obj/%.o)` demonstrates Make's string pattern substitution:
- `%.c` matches the base filename with a `.c` extension.
- `obj/%.o` replaces the matched pattern, prefixing it with the `obj/` path.
- For `admin/cmd_edit.c`, `%` matches `admin/cmd_edit`, producing `obj/admin/cmd_edit.o`.

---

### 7.4 Windows Shell Compatibility Considerations

The Makefile is tailored for the native Windows command shell (`cmd.exe`):
- Directory checks use `@if not exist obj mkdir obj` instead of Unix `mkdir -p obj`.
- File removal uses Windows `del` and `rmdir /s /q` instead of `rm -rf`.

> [!NOTE]
> Notice that for `admin/admin_panel.c`, the target object file is `obj/admin/admin_panel.o`. The command `@if not exist obj mkdir obj` creates `obj`, but if `obj/admin/` does not already exist, GCC will fail with an error because the `admin` subfolder inside `obj` is missing.
> **Fix for Junior Devs:** Ensure directory creation handles subfolders:
> ```makefile
> @if not exist "obj\admin" mkdir "obj\admin"
> ```

---

## 8. Code Review, Edge Cases & Roadmap for Juniors

As a junior developer maintaining or extending this codebase, keep the following technical considerations and improvement opportunities in mind:

### 1. Connecting the Parsers to the Main Game Loop
In the current repository state:
- `ans_validator.c`, `evidence_parser.c`, `interrogation_parser.c`, `past_history_parser.c`, and `vault_pass.c` are fully written and have matching headers in `include/`.
- However, they are not yet listed in `SRC` inside `Makefile`, nor are their headers included in `level_select.c` or `main.c`.
- **Next Task:** Add these parser source files to `SRC` in `Makefile` and call `parse_evidence()`, `parse_interrogation()`, and `parse_past_history()` when the player selects Level 1 in `level_select.c`.

### 2. Standard Input (`stdin`) Trailing Newline Traps
In `main.c`:
```c
scanf("%c", &selection);
```
`scanf("%c")` reads the next raw byte from standard input, including leftover newline characters (`\n`) from previous inputs.
**Solution:** Always place a leading space before `%c`:
```c
scanf(" %c", &selection);
```
The leading whitespace character tells `scanf` to automatically discard any preceding whitespace, newlines, or carriage returns.

### 3. Header Include Syntax Consistency
In `admin/admin_panel.c` line 5:
```c
#include <cmd_edit.h> // Uses angle brackets
```
In `admin/admin_panel.c` line 2:
```c
#include "admin_panel.h" // Uses quotes
```
While `-Iinclude` in `CFLAGS` allows both forms to compile, local project headers should consistently use double quotes (`"cmd_edit.h"`), reserving angle brackets (`<...>`) for C standard library headers.

### 4. Memory Ownership and Deallocation
Whenever you call:
- `parse_evidence()`
- `parse_interrogation()`
- `parse_past_history()`
- `parse_solution_desc()`

Remember that dynamic heap allocations are made. Always make sure to call their corresponding cleanup functions (`free_evidence()`, `free_statements()`, `free_past_history()`, `free(buffer)`) before exiting the level or terminating the program to prevent memory leaks.

---

*Documentation compiled for KUET CSE 1-1 academic project training. Keep this guide as a reference for C memory management, file parsing patterns, and Makefile structure.*
