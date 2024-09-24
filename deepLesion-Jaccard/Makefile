CC=gcc
CFLAGS=
INCLUDEPATH=../src/include
LIBPATH=-L../build
INCLUDE=-I$(INCLUDEPATH)
LIBS=-lstdc++ -lm -larboretum -lm
SRC= mainDeepLesion.cpp appDeepLesion.cpp deepLesion.cpp
OBJS=$(subst .cpp,.o,$(SRC))

STD=-std=c++2a

# Implicit Rules
%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(STD) -c $< -o $@ $(INCLUDE)

UsCities: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o DeepLesion $(INCLUDE) $(LIBPATH) $(LIBS)

clean:
	rm -f *.o
	rm -f DeepLesion
	rm -f SlimTree.dat