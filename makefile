CC = gcc
OBJSS = drone4.c
CFLAGS = -g -Wall
LIBS = -lm

all: drone4

drone4: $(OBJSS)
	$(CC) $(CFLAGS) -o $@ $(OBJSS) $(LIBS)

clean:
	rm -f drone4
