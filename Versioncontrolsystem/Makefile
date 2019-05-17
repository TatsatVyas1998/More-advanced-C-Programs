WTF WTFserver: WTF.c WTFserver.c
	gcc -g -lm -Wall -Werror -fsanitize=address WTF.c -o WTF -lcrypto -lssl
	gcc -g -lm -Wall -Werror -fsanitize=address WTFserver.c -pthread -o WTFserver -lcrypto -lssl

clean:
	rm -f WTFserver WTF
