CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

TARGET = thc.exe
SRC = main.c level_select.c progress.c admin/admin_panel.c admin/cmd_edit.c admin/log_parser.c
OBJ = $(SRC:%.c=obj/%.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

obj/%.o: %.c
	@if not exist obj mkdir obj
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if exist obj rmdir /s /q obj
	@if exist $(TARGET) del $(TARGET)