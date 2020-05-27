/* Kevin Ngo
 * Public key crytosystem (Source file)
 */

/*public-key-cryptosystem.c
	This program takes either one or three inputs.
	Either it generates two text files "pubkey.txt" and "prikey.txt" if only "keygen" is given as an argument
	Or the user inputs a key, a plaintext/ciphertext file, and a boolean
	The first input is either "keygen" (as mentioned earlier), a public key (if encrypting), or a private key (if decrypting)
	The second input is either a plaintext file (if encrypting) or a ciphertext file (if decrypting)
	The third input is either "-e" for encryption or "-d" for decryption.
	The public key and private key used for encryption and decryption must be from the pair generated
		during "keygen"

	Notes:
	-If the total characters in the plaintext file is not divisible by 4, then there will be a buffer of zeros at the end
		-Upon decryption, this buffer of zeros will remain as hex "/00", it is safe to ignore.

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int keygen();
int millerRabin(unsigned long n);
void qGen(unsigned long* q);
int witness(unsigned long a, unsigned long n);
unsigned long modExpo(unsigned long a, unsigned long b, unsigned long n);
int encrypt(char* key, char* plain);
int decrypt(char* key, char* cipher);

int main(int argc, char* argv[]){
	if(argc != 2 && argc != 4){	//if unexpected number of arguments, reprompt
		printf("Usage:\n");
		printf("Generate key - './public-key-cryptosystem keygen'\n");
		printf("Encrypt Msg - './public-key-cryptosystem <key.txt> <plaintext.txt> -e'\n");
		printf("Decrypt Msg - './public-key-cryptosystem <key.txt> <ciphertext.txt> -d'\n");
		return 1;
	}

	//option 1: keygen
	//expected input:
	//$>./public-key-cryptosystem keygen
	//expected output:
	//two txt files, one named "pubkey.txt" and one named "prikey.txt"
	//pubkey.txt contains "p" "g" and "e" and prikey.txt "p" "g" "d"
	if(strncmp(argv[1], "keygen", 6) == 0){	//if first argument is "keygen"
		if(keygen()){
			return 1;
		}
		return 0;
	}

	//option 2: encrypt plaintext
	//expected input:
	//$>./public-key-cryptosystem key.txt plaintext.txt -e
	//expected output:
	//one txt file named "ctext.txt"
	if(strncmp(argv[3], "-e", 2) == 0){	//if encryption mode is set
		if(encrypt(argv[1], argv[2])){
			return 1;
		}
		return 0;
	}

	//option 3: decrypt ciphertext
	//expected input:
	//$>./public-key-cryptosystem key.txt ciphertext.txt -d
	//expected output:
	//one txt file named "dtext.txt"
	if(strncmp(argv[3], "-d", 2) == 0){	//if decryption mode is set
		if(decrypt(argv[1], argv[2])){
			return 1;
		}
		return 0;
	}

	//reprompt because of bad input
	printf("Usage:\n");
	printf("Generate key - './public-key-cryptosystem keygen'\n");
	printf("Encrypt Msg - './public-key-cryptosystem <key.txt> <plaintext.txt> -e'\n");
	printf("Decrypt Msg - './public-key-cryptosystem <key.txt> <ciphertext.txt> -d'\n");
	return 1;
}

int keygen(){
	//make sure pubkey.txt and prikey.txt doesn't already exist
	if(access("pubkey.txt", F_OK) != -1){
		printf("File 'pubkey.txt' already exists in directory. Move or remove before running.\n");
		return 1;
	}
	if(access("prikey.txt", F_OK) != -1){
		printf("File 'prikey.txt' already exists in directory. Move or remove before running.\n");
		return 1;
	}

	FILE* pubkeyText = fopen("pubkey.txt", "wb");	//open pubkey for writing
	if(pubkeyText == NULL){
		fprintf(stderr, "Error creating file 'pubkey.txt': %s\n", strerror(errno));
		return 1;
	}
	FILE* prikeyText = fopen("prikey.txt", "wb");	//open prikey for writing
	if(prikeyText == NULL){
		fprintf(stderr, "Error creating file 'prikey.txt': %s\n", strerror(errno));
		return 1;
	}

	unsigned long p; int g = 2; unsigned long e2; unsigned long d; unsigned long q = 0;
	int seed = -1;
	while(seed < 0 || seed > 10000){	//prompt user for a number to seed random generator
		printf("Enter a number between 0 and 10000 to seed the random generator.\n");
		scanf("%5d", &seed);
	}
	srand(seed);	//seed srand with given seed
	qGen(&q);	//get a q where (q % 12) == 5
	p = (2 * q) + 1;
	while(millerRabin(p)){	//while p is composite (not prime)
		qGen(&q);	//generate a new q
		p = (2 * q) + 1;	//generate a new p and test again
	}
	d = rand();	//make d and make sure it is 0 < d < p
	d = (d % p) + 1;	//+1 is if d % p == 0

	e2 = modExpo(g, d, p);	//e2 = (g ^ d) % p

	fprintf(pubkeyText, "%lu %d %lu", p, g, e2);	//output p g e2 to pubkey.txt
	printf("Generated Public Key\tp:%lu\tg:%d\te2:%lu\n", p, g, e2);
	fprintf(prikeyText, "%lu %d %lu", p, g, d);	//output p g d to prikey.txt
	printf("Generated Private Key\tp:%lu\tg:%d\td:%lu\n", p, g, d);
	fclose(pubkeyText);	//close pubkey
	fclose(prikeyText);	//close prikey
	return 0;
}

void qGen(unsigned long* q){	//runs q until (q % 12) == 5
	*q = rand();
	*q = (*q | (1 << 30));	//set bit 31 to 1
	*q = (*q & (~(1 << 31))); 	//make sure bit 32 is 0
	while(millerRabin(*q) || ((*q % 12) != 5)){
		*q = rand();
		*q = (*q | (1 << 30));	//set bit 31 to 1
		*q = (*q & (~(1 << 31))); 	//make sure bit 32 is 0
	}
}

int millerRabin(unsigned long n){	//returns 0 if prime, 1 if composite
	for(int i = 1; i <= 10; i++){	//test 1000 times
		unsigned long a = (rand() % (n - 1));
		if(witness(a, n)){
			//printf("Number %ld is Composite\n", n);
			return 1;	//composite
		}
	}
	//printf("Number %ld is probably Prime\n", n);
	return 0;	//prime
}

int witness(unsigned long a, unsigned long n){
	unsigned long u = n - 1;
	int t = 1;
	while(u % 2 == 0){
		u /= 2;
		t++;
	}
	unsigned long x = modExpo(a, u, n);
	unsigned long xPrev;
	for(int i = 1; i <= t; i++){
		xPrev = x;
		x = (xPrev * xPrev) % n;
		if((x == 1) && (xPrev != 1) && (xPrev != (n - 1))){
			return 1;
		}
	}
	if(x != 1){
		return 1;
	}
	return 0;
}


unsigned long modExpo(unsigned long a, unsigned long b, unsigned long n){
	unsigned long c = 0;
	unsigned long d = 1;
	for(int i = ((sizeof(int) * 8) - 1); i >= 0; i--){
		c *= 2;
		d = (d * d) % n;
		int b_i = (b >> i) & 1;
		if (b_i == 1){
			c++;
			d = (d * a) % n;
		}
	}
	return d;
}

int encrypt(char* key, char* plain){
	//open check inputs key.txt and plaintext.txt
	FILE* keyText = fopen(key, "r");
	if(keyText == NULL){
		fprintf(stderr, "Error opening file '%s': %s\n", key, strerror(errno));
		return 1;
	}
	FILE* plainText = fopen(plain, "r");
	if(plainText == NULL){
		fprintf(stderr, "Error opening file '%s': %s\n", plain, strerror(errno));
		fclose(keyText);
		return 1;
	}

	if(access("ctext.txt", F_OK) != -1){	//make sure existing ctext.txt doesn't already exist
		printf("File 'ctext.txt' already exists in directory. Move or remove before running.\n");
		return 1;
	}

	FILE* cipherText = fopen("ctext.txt", "wb");	//open ctext for writing
	if(cipherText == NULL){
		fprintf(stderr, "Error creating file 'ctext.txt': %s\n", strerror(errno));
		return 1;
	}

	unsigned long p = 0; unsigned long g = 0; unsigned long e2 = 0;
	char line[128];
	if(fgets(line, sizeof(line), keyText) == NULL){	//get line from plainText
		printf("Issue reading line from public key file '%s', check to see if it is correct.\n", key);
		return 1;
	}
	if(sscanf(line, "%lu %lu %lu", &p, &g, &e2) == EOF){	//read in the three expected unsigned longs p g and e2
		fprintf(stderr, "Error reading from public key file '%s': %s\n", key, strerror(errno));
		return 1;
	}
	//printf("Read p:%lu\tg:%lu\te2:%lu\n", p, g, e2);
	fclose(keyText);	//done reading from key, close file

	unsigned long ch1; unsigned long ch2; unsigned long ch3; unsigned long ch4;
	int endOfFile = 0;
	srand(getpid());	//seed rand
	while(!endOfFile){	//while endOfFile has not been hit
		if((ch1 = fgetc(plainText)) == EOF){	//grab a character
			break;	//exit immediately if EOF
		}
		if(!endOfFile){
			if((ch2 = fgetc(plainText)) == EOF){
				endOfFile = 1;
				ch2 = 0;
				ch3 = 0;
				ch4 = 0;
			}
		}
		if(!endOfFile){
			if((ch3 = fgetc(plainText)) == EOF){
				endOfFile = 1;
				ch3 = 0;
				ch4 = 0;
			}
		}
		if(!endOfFile){
			if((ch4 = fgetc(plainText)) == EOF){
				endOfFile = 1;
				ch4 = 0;
			}
		}
		unsigned long m = (ch1 << 24)|(ch2 << 16)|(ch3 << 8)|ch4;	//bit shift into single number
		m = (m & (~(1 << 31))); 	//make sure bit 32 is 0
		unsigned long k = rand();
		k = k % p;	//make sure k is no more than p
		unsigned long c1 = modExpo(g, k, p);	//c1 = g^k mod p
		//c2 = (e2^k * m )mod p, split to ((e2^k mod p) * m) mod p
		unsigned long c2 = modExpo(e2, k, p);	//((e2^k mod p)
		c2 = (c2 * m) % p;	//* m) mod p
		fprintf(cipherText, "%lu %lu ", c1, c2);	//output p g e2 to pubkey.txt
		printf("C1:%lu\tC2:%lu\n", c1, c2);
	}
	fclose(plainText);	//close plaintext
	fclose(cipherText);	//close ciphertext
	return 0;
}

int decrypt(char* key, char* cipher){
	//open check inputs key.txt and ciphertext.txt
	FILE* keyText = fopen(key, "r");
	if(keyText == NULL){
		fprintf(stderr, "Error opening file '%s': %s\n", key, strerror(errno));
		return 1;
	}
	FILE* cipherText = fopen(cipher, "r");
	if(cipherText == NULL){
		fprintf(stderr, "Error opening file '%s': %s\n", cipher, strerror(errno));
		fclose(keyText);
		return 1;
	}
	
	if(access("dtext.txt", F_OK) != -1){	//make sure dtext.txt doesn't already exist
		printf("File 'dtext.txt' already exists in directory. Move or remove before running.\n");
		return 1;
	}
	FILE* plainText = fopen("dtext.txt", "wb");	//open plaintext for writing
	if(cipherText == NULL){
		fprintf(stderr, "Error creating file 'dtext.txt': %s\n", strerror(errno));
		return 1;
	}

	unsigned long p = 0; unsigned long g = 0; unsigned long d = 0;
	char line[128];
	if(fgets(line, sizeof(line), keyText) == NULL){	//get line from plainText
		printf("Issue reading line from private key file '%s', check to see if it is correct.\n", key);
		return 1;
	}
	if(sscanf(line, "%lu %lu %lu", &p, &g, &d) == EOF){	//read in the three expected unsigned longs p g and d
		fprintf(stderr, "Error reading from public key file '%s': %s\n", key, strerror(errno));
		return 1;
	}
	//printf("Read p:%lu\tg:%lu\td:%lu\n", p, g, d);
	fclose(keyText);	//done reading from key

	unsigned long c1 = 0; unsigned long c2 = 0;
	while((fscanf(cipherText, "%lu %lu", &c1, &c2)) != EOF){
		//((c1^(p-1-d) mod p) * (c2 mod p) mod p)
		unsigned long long m = 0;
		m = modExpo(c1, (p-1-d), p);
		m = m * (c2 % p);
		m = m % p;
		//printf("m: %llu\n", m);
		m = (m & (~(1 << 31))); 	//make sure bit 32 is 0
		fputc(m >> 24, plainText);	//bitshift and put characters
		fputc(m >> 16, plainText);	//bitshift and put characters
		fputc(m >> 8, plainText);	//bitshift and put characters
		fputc(m, plainText);	//put character
	}
	fclose(plainText);	//close plaintext
	fclose(cipherText);	//close ciphertext
	return 0;
}