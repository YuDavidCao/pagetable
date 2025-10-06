CC = gcc
CFLAGS = -O1 -Wall -Wextra
TARGET = mlpt
SOURCES = mlpt.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean