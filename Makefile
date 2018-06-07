all: motord example_cli

motord: motord.c motor.o
	gcc -o motord motord.c motor.o -lpthread -lpigpio

example_cli: example_cli.c motor_api.o
	gcc -o example_cli example_cli.c motor_api.o

motor.o: motor.h motor.c
	gcc -c motor.c

motor_api.o: motor_api.h motor_api.c
	gcc -c motor_api.c

clean:
	rm -fv *.o motord example_cli
