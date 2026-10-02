CC = gcc

CFLAGS = -Wall -Wextra -Iinclude

TARGET = HollowCipher

SRC = src/main.c \
      src/game.c \
      src/screens/boot.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)