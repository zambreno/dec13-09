CFLAGS = -I./SFML2.0-src/include/
LDFLAGS = -L./SFML2.0-bin/lib/ -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system

SRCPATH = ../src/

songbox: songbox.o sfml_utils.o
	g++ -o songbox songbox.o sfml_utils.o $(LDFLAGS)
    
songbox.o: $(SRCPATH)songbox.cpp $(SRCPATH)songbox.hpp
	g++ -c $(CFLAGS) $(SRCPATH)songbox.cpp

sfml_utils.o: $(SRCPATH)sfml_utils.cpp
	g++ -c $(CFLAGS) $(SRCPATH)sfml_utils.cpp

clean:
	rm -rf *.o songbox

