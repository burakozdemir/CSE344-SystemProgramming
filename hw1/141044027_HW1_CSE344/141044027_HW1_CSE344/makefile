all: main

main: compile link 
	
	
compile: 
	gcc -c 141044027_main.c  -std=gnu11
	
link: 
	gcc 141044027_main.o -o tiffprocessor -std=gnu11

work:
	./tiffprocessor TEST2.TIF
	
clean:
	rm *.o tiffprocessor
	
	
