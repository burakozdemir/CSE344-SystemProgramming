all: compile

compile:
	gcc -o server server.c -lrt -pthread -lm
	gcc -o client client.c -lrt -pthread -lm
	chmod +x script.sh

workserver:
	./server 5555 data.dat log.dat

workclient:
	./client hileci1 C 45 127.0.0.1 5555

workscript:
	./script.sh

clean:
	rm *.o server.out client.out

valgrind:
	valgrind ./server 5555 data.dat log.dat

valgrindfull:
	valgrind --leak-check=full --show-leak-kinds=all ./server 5555 data.dat log.dat
