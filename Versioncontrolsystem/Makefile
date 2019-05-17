client server: client.c server.c
	gcc -g -lm -Wall -Werror -fsanitize=address client.c -o client -lcrypto -lssl
	gcc -g -lm -Wall -Werror -fsanitize=address server.c -pthread -o server -lcrypto -lssl

clean:
	rm -f server client
