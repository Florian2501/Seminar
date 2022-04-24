#include <stdio.h>
#include <math.h>

#include "murmur.h"

//BF steht fuer Bloom-Filter
//False Positive Propability (FPP) --> Fehlerrate mit der ausgibt, dass Element entahlten, obwohl es nicht enthalten ist
//Benoetigte Speichergroesse m in Bit
//Anzahl zu speichernder Elemente n
//Optimale Anzahl Hash-Funktionen k
//Formeln:
//m = - n*ln(FPP)/(ln(2))^2
//k = m/n * ln(2)

#define FFP 0.05
#define n 100

//#define DEBUG

//Gibt den in bloom übergebenen BF binaer aus
// m in Bit
void printBF(char* bloom, int m)
{
    //durchlaufe den ganzen BF bitweise
    for(int i = 0; i < m; i++)
    {
        //1 ausgeben wenn bit gesetzt und 0 wenn nicht
        printf("%d", (bloom[ i / 8 ] & (1 << ( i % 8 ))) ? 1 : 0 );

        //zur besseren AUsgabe in der Console alle 4 Bit ein Leerzeichen und alle 8 einen Trennstrich ausgeben
        if ((i % 8) == 3) printf(" ");
        else if ((i % 8) == 7) printf(" | ");
    }
    printf("\n\n");
}

//Die BF-Groesse wird mittels der bekannten Formel berechnet
int berechneM()
{
    //ceil rundet dabei das Ergebnis auf die naechste Ganzzahl auf
    return (int)(ceil( (-1)*n*log(FFP)/(log(2)*log(2)) ));
}

//Die optimale Anzahl k Hash-Funktionen wird mittels der bekannten Formel berechnet
//m ist dabei die Groesse des BF in Bit
int berechneK(int m)
{
    //round rundet dabei das Ergebnis auf eine Ganzzahl
    return (int)(round( (double)m/n * log(2) ));
}

//Das in element uebergebene Element wird in den in bloom übergebenen Bloom-Filter eingefügt mittels k Hash-Funktionen
//m ist die Länge des BF in Bit
int einfuegen(char* bloom, char* element, int k, int m)
{    
    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF setzen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des einzufuegenden Elements wird berechnet und auf die maximal moegliche Laenge des BF heruntergebrochen
        unsigned int position = murmur2(element, sizeof(element), i) % m;

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        //Bit an der Stelle des Elements setzen
        bloom[char_position] |= (1<<bit_position);
        
        #ifdef DEBUG
        //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
        printf("Hash %d von %s: %d (Byte: %d, Bit: %d)\n", i+1, element, position, char_position, bit_position);
        printBF(bloom, m);
        #endif
    }
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

    //Sicherstellen, dass allo Werte im BF 0 sind
    for(int i = 0; i<m_in_byte; i++)
    {
        bloom[i] = 0;
    }

    printf("Fuellen des Bloom-Filters mit %d englischen Worten...\n", n);

    //Die Liste der verschiedenen englischen Worte ist von https://github.com/dwyl/english-words und ist frei verfuegbar

    char* filename = "words4.txt";
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
        //Da der uebrige Platz im letzten evtl. angerissenen Byte mitgenutzt werden soll, wird nicht die
        //genaue Anzahl Bit angegeben, die durch die Formel errechnet wurde, sondern die auf volle Byte 
        //aufgerundete Anzahl Bit, da der Platz im evtl. angerissenen Byte sonst verloren ginge
        einfuegen(bloom, word, k, m_in_byte*8);
    }

    printBF(bloom, m_in_byte*8);

    //words.txt schliessen
    fclose(file);
    //Bloom Filter freigeben
    free(bloom);

    return EXIT_SUCCESS;
}