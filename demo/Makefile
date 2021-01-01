CC=gcc
CFLAGS= -Wall -O -Wmaybe-uninitialized -I../includes -I.
CLEAN = rm

TARGET = demo
SRCDIR = ../src
OBJDIR = $(SRCDIR)
OBJECTS = $(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c)) main.o


$(TARGET): $(OBJECTS) $(LIBRARY)
	$(CC) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(CLEAN) -f *.o $(OBJDIR)/*.o $(TARGET)
