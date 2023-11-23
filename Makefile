#Makefile

CC = g++
CFLAGS = -Wall -std=c++11
LIBS = -lpng

SRCS = GameOfLife.cpp
OBJS = GameOfLife.o

all: GameOfLife

GameOfLife: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o GameOfLife $(LIBS)

GameOfLife.o: $(SRCS)
	$(CC) $(CFLAGS) -c $(SRCS) -o GameOfLife.o

clean:
	rm -f $(OBJS) GameOfLife
