all :
	g++ --std=c++11 ./logging/logger.cpp ./logging/logger.hpp ./logging/log.hpp ./logging/log.cpp map.hpp map.cpp game.hpp game.cpp -lpthread -o game.exe

all_obj : 
	g++ --std=c++11 -lstdthread -lpthread -c ./logging/logger.hpp ./logging/log.hpp ./logging/log.cpp game.hpp game.cpp main.cpp

regression : financial.o all_obj
	del main.o
	g++ --std=c++11 -I. ./regression/regression.cpp *.o -o regression.exe
	del *.o *.gch

