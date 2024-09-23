CC=gcc
CFLAGS=
INCLUDEPATH=../src/include
LIBPATH=-L../build
INCLUDE=-I$(INCLUDEPATH)
LIBS=-lstdc++ -lm -larboretum
SRC= main.cpp app.cpp city.cpp
OBJS=$(subst .cpp,.o,$(SRC))

STD=-std=c++20

# Implicit Rules
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(STD) -c $< -o $@ $(INCLUDE)

Cities: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o Cities $(INCLUDE) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o
	rm -f Cities
	rm -f SlimTree.dat