# ==============================================================================
# Makefile - The Hollow Cipher (Retro Edition)
#
# Builds the Win32 GDI Retro GUI build with decoupled C backend logic.
#
# Usage:
#   make        - Compile object files and link TheHollowCipher.exe
#   make run    - Compile and launch TheHollowCipher.exe
#   make clean  - Remove compiled object files and target executable
# ==============================================================================

CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -O2
LDFLAGS = -mwindows -lgdi32 -luser32 -lkernel32

TARGET = TheHollowCipher.exe

SRCS = src/main.c \
        src/game.c \
        src/ui_engine.c \
        src/screens/boot.c \
        src/screens/home.c \
        src/screens/options.c \
        src/screens/about.c \
        src/screens/stub_screen.c \
        src/screens/level_select.c \
        src/screens/level_main.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	-copy /Y $(TARGET) HollowCipher.exe 2>nul || cp $(TARGET) HollowCipher.exe 2>/dev/null || true

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

ifeq ($(OS),Windows_NT)
clean:
	-del /F /Q $(subst /,\,$(OBJS)) $(TARGET) HollowCipher.exe 2>nul || exit 0
	-del /F /Q src\include\*.o 2>nul || exit 0
else
clean:
	rm -f $(OBJS) $(TARGET) HollowCipher.exe
	rm -f src/include/*.o
endif

.PHONY: all run clean