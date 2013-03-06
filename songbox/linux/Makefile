# Put x86-Linux-specific makefile and libs here
CC = g++
CFLAGS = -I../src/include/
LDFLAGS = -L./lib/ -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system

SRCPATH = ../src/

songbox: songbox.o sfml_utils.o
	$(CC) -o songbox songbox.o sfml_utils.o $(LDFLAGS)
    
songbox.o: $(SRCPATH)songbox.cpp $(SRCPATH)songbox.hpp
	$(CC) -c $(CFLAGS) $(SRCPATH)songbox.cpp

sfml_utils.o: $(SRCPATH)sfml_utils.cpp
	$(CC) -c $(CFLAGS) $(SRCPATH)sfml_utils.cpp

clean:
	rm -rf *.o songbox

