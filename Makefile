CC = gcc
CFLAGS = -O1 -Wall -Wextra
LIBRARY = libmlpt.a
SOURCES = mlpt.c
OBJECTS = mlpt.o

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $(LIBRARY) $(OBJECTS)

mlpt.o: mlpt.c mlpt.h config.h
	$(CC) $(CFLAGS) -c mlpt.c -o mlpt.o

clean:
	rm -f $(OBJECTS) $(LIBRARY)

.PHONY: all clean