all: motord

motord: motord.c motor.o
	gcc -o motord motord.c motor.o -lpthread

motor.o: motor.h motor.c
	gcc -c motor.c

clean:
	rm -fv *.o motord
