Kevin Ngo
Public key crytosystem (README)

Included files:
README.txt	(this file)
public-key-cryptosystem.c 	(project source code)
Makefile	(a make file for compiling the source code)

To compile, have required files
	public-key-cryptosystem.c
	Makefile
in the same directory, and use the command
	make
	OR
	gcc -o public-key-cryptosystem public-key-cryptosystem.c
to compile public-key-cryptosystem.c

To run, use the command format
	./public-key-cryptosystem keygen
	OR
	./public-key-cryptosystem <key.txt> <plaintext.txt/ciphertext.txt> -<e/d>
where
	keygen generates a public key and private key text files named
		pubkey.txt
	and
		prikey.txt
and
	<key.txt>
	is either your public key (for ENCRYPTION), likely named "pubkey.txt"
	or your private key (for DECRYPTION), likely named "prikey.txt"
and
	<plaintext.txt/ciphertext.txt>
	is your plaintext file or a generated ciphertext file
and
	-<e/d>
	is either encrypt (e)
	or decrypt (d)
Example commands for key generation, encryption, and decryption are included at the bottom of this README.

Output:
Output for encryption will be named "ctext.txt"
while for decryption will be named "dtext.txt"

Notes:
The program will warn accordingly if either already exist.
If the total characters are not exactly divisible by four, then it will be padded with extra characters in the ciphertext.
	This also means that upon decrypting there will be extra hex "00" characters at the end.

Example commands:
Example command for key generation:
	./public-key-cryptosystem keygen
Output is "pubkey.txt" which contains p, g, and e2
and "prikey.txt" which contains p, g, and d

Example command for encryption:
	./public-key-cryptosystem pubkey.txt plaintext.txt -e
where "pubkey.txt" is a key from keygen
where "plaintext.txt" is a text file to be encrypted
where "-e" is the mode for encryption
Output is "ctext.txt" which is the ciphertext generated from encryption.

Example command for decryption:
	./public-key-cryptosystem prikey.txt ctext.txt -d
where "prikey.txt" is a key from keygen
where "ctext.txt" is a text file that is encrypted
where "-d" is the mode for decryption
Output is "dtext.txt" which is the plaintext generated from decryption.
