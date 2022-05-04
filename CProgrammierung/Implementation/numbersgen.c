/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int main(){
       time_t t;
srand((unsigned) time(&t));


    FILE* write = fopen("numbers.txt", "w");

    for(int i = 0; i<1000000; i++){

        int r = rand() + (rand()<<15);
        fprintf(write, "%d\n", r);
        //printf("%s", &arr[i]);
    }
        
fflush(write);
fclose(write);



}
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(){


       time_t t;
srand((unsigned) time(&t));

int *arr = malloc(1000000*4);

for(int i = 0; i<1000000; i++){
    arr[i] = i;
}


   /* for(int i=0; i<10000000; i++){

//int r= (int)((double)rand() / ((double)RAND_MAX + 1) * 367522);
//int z= (int)((double)rand() / ((double)RAND_MAX + 1) * 367522);
        int r = rand();
        while(r>1000000){
        r = rand();
        }
        int z = rand();
        while(z>1000000){
        z = rand();
        }     
        
        //printf("Tausche: %d mit %d\n", r,z);
        int puff = arr[r];
        arr[r]=arr[z];
        arr[z] = puff;


    }*/

    FILE* write = fopen("numbers2.txt", "w");
    for(int i = 0; i<1000000; i++){
        fprintf(write, "%d\n", arr[i]);
        //printf("%s", &arr[i]);
    }
        
fflush(write);
free(arr);
fclose(write);



}