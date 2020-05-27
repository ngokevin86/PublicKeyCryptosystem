#Kevin Ngo
#Public Key Cryptosystem (Makefile)

all: public-key-cryptosystem.c
	gcc -Wall -Werror -o public-key-cryptosystem public-key-cryptosystem.c

clean:
	rm -f public-key-cryptosystem *.o