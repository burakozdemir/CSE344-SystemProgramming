all: main

main: compile 
	
	
compile: 
	 gcc 141044027_main.c -lrt -lpthread -lm

work:
	./a.out data.dat
	
clean:
	rm *.o a.out

valgrind:
	valgrind -v ./a.out data.dat

valgrindfull:
	valgrind --leak-check=full --show-leak-kinds=all ./a.out data.datm
	
	
