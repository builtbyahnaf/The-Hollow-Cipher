# ==========================================
# THE HOLLOW CIPHER - Makefile
# ==========================================

# Compiler used to compile C source files
CC = gcc

# Standard Compiler flags:
# -Wall       : Enable most common warnings
# -Wextra     : Enable additional warnings
# -Iinclude   : Search for header files inside include/
CFLAGS = -Wall -Wextra -Iinclude

# Debug flags:
# -g          : Generate debugging information for GDB
# -O0         : Disable compiler optimizations for accurate debugging
# -DTHC_DEBUG=1 : Activates repository debugging codes (GC sweep diagnostics & secret vault pass)
# -DDEBUG     : Standard preprocessor debug symbol
DEBUG_FLAGS = -g -O0 -DTHC_DEBUG=1 -DDEBUG

# Optional command-line build flag: "make DEBUG=1"
ifeq ($(DEBUG), 1)
CFLAGS += $(DEBUG_FLAGS)
endif

# Name of the final executable
TARGET = thc.exe


# ==========================================
# SOURCE FILES
# ==========================================

# Automatically find all .c files in these directories.
# wildcard searches only the specified directory level,
# not its subdirectories recursively.
SRC = main.c \
      $(wildcard gameplay/*.c) \
      $(wildcard utils/*.c) \
      $(wildcard admin/*.c)


# ==========================================
# OBJECT FILES
# ==========================================

# Convert each source path into an object-file path.
#
# Example:
# main.c                -> obj/main.o
# gameplay/level.c      -> obj/gameplay/level.o
# admin/panel.c         -> obj/admin/panel.o
#
# patsubst means "pattern substitution".
# It replaces .c with obj/... .o while preserving directories.
OBJ = $(patsubst %.c,obj/%.o,$(SRC))


# ==========================================
# SPECIAL TARGETS
# ==========================================

# These names are commands, not actual files.
# Make should always execute these targets when requested.
.PHONY: all clean clean-obj debug build-debug run


# ==========================================
# BUILD EVERYTHING (DEFAULT RELEASE)
# ==========================================

# Running "make" or "make all" builds the executable.
all: $(TARGET)


# ==========================================
# DEBUG TARGETS
# ==========================================

# "make build-debug" compiles the project with debug flags without launching it.
build-debug:
	@$(MAKE) clean-obj
	@$(MAKE) $(TARGET) CFLAGS="$(CFLAGS) $(DEBUG_FLAGS)"

# "make debug" builds with the debug flags and runs the executable to execute debugging codes.
debug:
	@$(MAKE) build-debug
	@echo -----------------------------------------------------------------
	@echo  RUNNING IN DEBUG MODE (THC_DEBUG=1)
	@echo -----------------------------------------------------------------
	$(TARGET)

# "make run" builds (if needed) and executes the game.
run: $(TARGET)
	$(TARGET)


# ==========================================
# LINK OBJECT FILES
# ==========================================

# The executable depends on all object files.
# If an object file needs rebuilding, Make rebuilds it first.
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# $@ = target being built: thc.exe
# $(OBJ) = all compiled .o files
# -o specifies the name of the output executable.


# ==========================================
# COMPILE EACH SOURCE FILE
# ==========================================

# General rule for converting a source file into an object file.
#
# Example:
# gameplay/level.c -> obj/gameplay/level.o
#
# % is a wildcard pattern matching the corresponding path/name.
obj/%.o: %.c

# Create the destination directory if it does not exist.
# $@ = complete output path, e.g. obj/gameplay/level.o
# $(@D) = directory containing the output, e.g. obj/gameplay
	@if not exist "$(@D)" mkdir "$(@D)"

# Compile the source file into an object file.
# $< = source file, e.g. gameplay/level.c
# $@ = output file, e.g. obj/gameplay/level.o
	$(CC) $(CFLAGS) -c $< -o $@


# ==========================================
# CLEAN GENERATED FILES
# ==========================================

# Delete only compiled object files
clean-obj:
	@if exist obj rmdir /s /q obj

# Running "make clean" removes generated build files.
clean: clean-obj
# Delete the executable if it exists.
	@if exist $(TARGET) del $(TARGET)
