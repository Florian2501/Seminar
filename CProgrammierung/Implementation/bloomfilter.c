#include <stdio.h>
#include <math.h>

#include "murmur.h"

//False Positive Propability (FPP) --> Fehlerrate mit der ausgibt, dass Element entahlten, obwohl es nicht enthalten ist
//Benoetigte Speichergroesse m in Bit
//Anzahl zu speichernder Elemente n
//Optimale Anzahl Hash-Funktionen k
//Formeln:
//m = - n*ln(FPP)/(ln(2))^2
//k = m/n * ln(2)

#define FFP 0.001
#define n 999

//Die BF-Groesse wird mittels der bekannten Formel berechnet
int berechneM()
{
    //ceil rundet dabei das Ergebnis auf die naechste Ganzzahl auf
    return (int)(ceil( (-1)*n*log(FFP)/(log(2)*log(2)) ));
}

//Die optimale Anzahl k Hash-Funktionen wird mittels der bekannten Formel berechnet, m ist dabei die Groesse des BF
int berechneK(int m)
{
    //round rundet dabei das Ergebnis auf eine Ganzzahl
    return (int)(round( (double)m/n * log(2) ));
}

int einfuegen(char* bloom, char* element)
{
    printf("Einfuegen: %s\n", element);
    //TODO
}

int main(void)
{
    //Aus den gegebenen Groessen (Anzahl einzufuegende Elemente und maximale FFP) werden m und k berechnet
    int m = berechneM();
    printf("Der Bloom-Filter wird mit der Groesse m=%d Bit initialisiert...\n", m);

    int k = berechneK(m);
    printf("Die optimale Anzahl Hash-Funktionen k=%d wurde berechnet!\n", k);

    //Da in C nur Bytes verarbeitet werden koennen muss in Bytes umgerechnet werden
    //ceil rundet auf, damit garantiert alle Bits reinpassen
    int m_in_byte = ceil(m/8.0f);

    printf("Der Bloom-Filter wird erstellt mit %d Byte...\n", m_in_byte);

    //Bloom-Filter wird mit der berechneten Groesse in Byte erstellt
    //Die evtl. maximal 7 "zu vielen" Bits werden mitverwendet, da der Platz sowieso nicht anders verwendet werden kann
    char* bloom = malloc(m_in_byte * sizeof(char));

    printf("Fuellen des Bloom-Filters mit %d englischen Worten...\n", n);

    //Die Liste der verschiedenen englischen Worte ist von https://github.com/dwyl/english-words und ist frei verfuegbar

    char* filename = "words.txt";
    FILE* file = fopen(filename, "r");

    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        printf("Fehler beim oeffnen der Wortdatei %s. Abbruch des Fuellens!\n", filename);
        return EXIT_FAILURE;
    }

    //Puffer fuer aktuell hinzuzufuegendes Wort
    char word[255];

    for(int i = 0; i < n; i++)
    {
        //Wort einlesen
        fscanf(file, "%s", word);
        //Wort in Bloom-Filter einfuegen
        einfuegen(bloom, word);
    }


    //words.txt schliessen
    fclose(file);
    //Bloom Filter freigeben
    free(bloom);

    return EXIT_SUCCESS;
}