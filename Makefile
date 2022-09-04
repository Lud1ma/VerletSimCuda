CC = nvcc
CFLAGS = -std=c++11 --compiler-options -Wall --compiler-options -Wextra --compiler-options -ggdb
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lcurand

all: clean

clean: clean.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean.o: clean.cu 
	$(CC) $(CFLAGS) $< -c
