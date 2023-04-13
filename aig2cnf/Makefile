all:
	g++ --std=c++20 ./main.cc -o aig2cnf
	rm -rf *.o

release:
	g++ --std=c++20 ./main.cc --static -O3 -o aig2cnf
	rm -rf *.o
