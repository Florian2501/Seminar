#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(){
       time_t t;
srand((unsigned) time(&t));

char *arr = malloc(367522*255);
    char* filename = "words4.txt";
    FILE* file = fopen("wordsrandom.txt", "r");


    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        printf("Fehler beim oeffnen der Wortdatei %s. Abbruch des Fuellens!\n", filename);
        return EXIT_FAILURE;
    }

    char word[255];
    int i=0;
    while(i<367522){
        i++;
//Wort einlesen
    fscanf(file, "%s", word);
    //printf("%s", word);
    
    strcpy(&arr[255*i], word);
    //printf("%s", &arr[255*i]);
    }
    //Puffer fuer aktuell hinzuzufuegendes Wort
    char puff[255];

    for(int i=0; i<100000000; i++){

//int r= (int)((double)rand() / ((double)RAND_MAX + 1) * 367522);
//int z= (int)((double)rand() / ((double)RAND_MAX + 1) * 367522);
        int r = rand();
        while(r>367522){
        r = rand();
        }
        int z = rand();
        while(z>367522){
        z = rand();
        

        }       

        r=367522-r;
        z=367522-z;
        
        //printf("Tausche: %d mit %d\n", r,z);

        strcpy(puff, &(arr[r*255]));
        strcpy(&(arr[r*255]), &(arr[z*255]));
        strcpy(&(arr[z*255]), puff);

    }

fclose(file);

    FILE* write = fopen("wordsrandom.txt", "w");

    for(int i = 0; i<367522; i++){
        fprintf(write, "%s\n", &arr[i*255]);
        //printf("%s", &arr[i]);
    }
        
fflush(write);
free(arr);
fclose(write);



}