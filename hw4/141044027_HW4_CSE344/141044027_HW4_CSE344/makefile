all: main

main: compile 
	
	
compile: 
	 gcc 141044027_main.c -lrt -lpthread

work:
	./a.out -lpthread -lrt
	
clean:
	rm *.o a.out

valgrind:
	valgrind -v ./a.out -lpthread -lrt

valgrindfull:
	valgrind --leak-check=full --show-leak-kinds=all ./a.out -lpthread -lrt 
	
	
