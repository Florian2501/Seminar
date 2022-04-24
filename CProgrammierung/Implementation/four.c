#include <stdio.h>
#include <stdlib.h>

int main(){


    char* filename = "words.txt";
    FILE* file = fopen(filename, "r");

    FILE* write = fopen("words4.txt", "w");

    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        printf("Fehler beim oeffnen der Wortdatei %s. Abbruch des Fuellens!\n", filename);
        return EXIT_FAILURE;
    }
    char word[255];
int i=0;
    while(i<370105){
        i++;
//Wort einlesen
    fscanf(file, "%s", word);
    int j=0;
    char tmp = word[0];
    int length=0;
    while(tmp != '\0') {length++;
    j++;
    tmp=word[j];
    }
    
    if(length >= 4){
        fprintf(write, "%s\n", word);
    }
    }
    //Puffer fuer aktuell hinzuzufuegendes Wort

        





}