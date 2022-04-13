/* Murmur3 example: hash the first command line argument. */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "murmur3.h"

#define arr_length 32

unsigned int FNVHash(char* str, unsigned int length) {
	const unsigned int fnv_prime = 0x811C9DC5;
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < length; str++, i++)
	{
		hash *= fnv_prime;
		hash ^= (*str);
	}

	return hash;
}

int main(int argc, char **argv) {
  uint32_t hash[4];                /* Output for the hash */
  uint32_t seed = 42;              /* Seed value for hash */
  uint32_t seed2 = 123;              /* Seed value for hash */
  uint32_t seed3 = 34523623;              /* Seed value for hash */

    char* wort = "hallo";
  
  MurmurHash3_x86_32(wort, strlen(wort), seed, hash);
  printf("x86_32:  %08x\n", hash[0]);

  /*MurmurHash3_x86_128(wort, strlen(wort), seed, hash);
  printf("x86_128: %08x %08x %08x %08x\n",
         hash[0], hash[1], hash[2], hash[3]);

  MurmurHash3_x64_128(wort, strlen(wort), seed, hash);
  printf("x64_128: %08x %08x %08x %08x\n",
         hash[0], hash[1], hash[2], hash[3]);*/

    //printf("%d", sizeof(unsigned int));
    printf("Murmur1: %u\n", hash[0]);

    MurmurHash3_x86_32(wort, strlen(wort), seed2, hash);
  
    printf("Murmur2: %u\n", hash[0]);

    MurmurHash3_x86_32(wort, strlen(wort), seed3, hash);

    printf("Murmur3: %u\n", hash[0]);

    printf("fnv: %u\n", FNVHash(wort, strlen(wort)));

    //const int arr_length = 16;

    uint8_t arr[arr_length];
    uint8_t arrFlo[arr_length];
    
    for(int i = 0; i<arr_length; i++){
        arr[i]=0;
        arrFlo[i] = 0;
    }

    char text[] = "In this lesson we'll learn how to split a string into several tokens using strtok function To split a string we need delimiters delimiters are characters which will be used to split the string Suppose we've the following string and we want to extract the individual words";
	int init_size = strlen(text);
	char delim[] = " ";

	char *ptr = strtok(text, delim);

	while(ptr != NULL)
	{
		printf("'%s', ", ptr);

        MurmurHash3_x86_32(ptr, strlen(ptr), seed, hash);
        int hash1 = hash[0]%arr_length;
        printf("Murmur1: %d, ", hash1);
        arrFlo[hash1] |= 1;
        arr[hash1] = 1;

        MurmurHash3_x86_32(ptr, strlen(ptr), seed2, hash);
        int hash2 = hash[0]%arr_length;
        printf("Murmur2: %d, ", hash2);
        arrFlo[hash2] |= 2;
        arr[hash2] = 1;

        MurmurHash3_x86_32(ptr, strlen(ptr), seed3, hash);
        int hash3 = hash[0]%arr_length;
        printf("Murmur3: %d, ", hash3);
        arrFlo[hash3] |= 4;
        arr[hash3] = 1;

        int hash4 = FNVHash(ptr, strlen(ptr)) % arr_length;
        printf("FNV: %d\n", hash4);
        arrFlo[hash4] |= 8;
        arr[hash4] = 1;

		ptr = strtok(NULL, delim);
	}

    for(int i = 0; i<arr_length; i++){
        printf("%d: Normal: %d, Flo: %d\n", i, arr[i], arrFlo[i]);
    }

    char input[30];
    while(1){
        printf("===================================================\n");

        printf("Enter word: ");
        fgets(input, sizeof(input), stdin);  // read string

        for (int i= 0; i< 30; i++){
            if(input[i]=='\0' && i>0){
                input[i-1] = '\0';
                break;
            }
        }

        //check if its inside the array
        char normal = 0;
        char flo = 0;

        MurmurHash3_x86_32(input, strlen(input), seed, hash);
        int hash1 = hash[0]%arr_length;
        printf("Murmur1: %d, ", hash1);
        if(arr[hash1]==0){
            normal = 1; 
        }

        if((arrFlo[hash1]&1)==0){
            flo = 1;
        }

        MurmurHash3_x86_32(input, strlen(input), seed2, hash);
        int hash2 = hash[0]%arr_length;
        printf("Murmur2: %d, ", hash2);
        if(arr[hash2]==0){
            normal = 1; 
        }

        if((arrFlo[hash2]&2)==0){
            flo = 1;
        }

        MurmurHash3_x86_32(input, strlen(input), seed3, hash);
        int hash3 = hash[0]%arr_length;
        printf("Murmur3: %d, ", hash3);
        if(arr[hash3]==0){
            normal = 1; 
        }

        if((arrFlo[hash3]&4)==0){
            flo = 1;
        }

        int hash4 = FNVHash(input, strlen(input)) % arr_length;
        printf("FNV: %d\n", hash4);
        if(arr[hash4]==0){
            normal = 1; 
        }

        if((arrFlo[hash4]&8)==0){
            flo = 1;
        }

        if(normal != 0){
            printf("Das Wort kam laut normalem Filter nicht vor.\n");
        }
        else printf("Das Wort kam laut normalem Filter vielleicht vor.\n");
        
        if(flo != 0){
            printf("Das Wort kam laut Flo-Filter nicht vor.\n");
        }
        else printf("Das Wort kam laut Flo-Filter vielleicht vor.\n");
    }


  return 0;
}
