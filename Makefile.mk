checkers : consttypes.o functions.o main.o
	cc -o checkers consttypes.o functions.o main.o

main.o : main.c consttypes.h functions.h
	cc -c main.c
consttypes.o : consttypes.c consttypes.h
	cc -c consttypes.c
functions.o : functions.c consttypes.h functions.h 
	cc -c functions.c


.PHONY : clean
clean :
	rm edit consttypes.o functions.o main.o