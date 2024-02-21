cc=mpic++

all: mpi_hello pi

pi: pi.o
	$(cc) pi.o -o pi

pi.o: ser_pi_calc.cpp
	$(cc) -c ser_pi_calc.cpp -o pi.o

mpi_hello: hello.o
	$(cc) hello.o -o mpi_hello

hello.o: hello.cpp
	$(cc) -c hello.cpp -o hello.o 
