SRC = $(wildcard *.cpp)
OBJ = main.o

.PHONY : all clean

all : ${OBJ}

${OBJ} : ${SRC}
	g++ ${SRC} -o ${OBJ} -levent -lpthread -lmysqlclient -ljsoncpp -std=c++11


clean:
	rm ${OBJ}