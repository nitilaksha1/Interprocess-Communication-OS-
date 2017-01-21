
all: producer consumer main

producer: producer.c
	gcc producer.c -o producer -lpthread

consumer: consumer.c
	gcc consumer.c -o consumer -lpthread

main: main.c
	gcc main.c -o main -lpthread

cleanall:
	rm -rf Producer_BLACK.txt Producer_RED.txt Producer_WHITE.txt Consumer.txt producer consumer main
