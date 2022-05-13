#include <stdio.h>
#include <math.h>
#include <string.h>

#include "murmur.h"
//#define DEBUG
//#define PRESENT

//Struct, welches einen Bloom-Filter beschreibt
//False Positive Probability (FPP) --> Fehlerrate mit der ausgibt,
//dass Element entahlten, obwohl es nicht enthalten ist
//Benoetigte Speichergroesse m in Bit
//Anzahl zu speichernder Elemente n
//Optimale Anzahl Hash-Funktionen k
typedef struct
{
    int m;
    int m_in_byte;
    int n;
    int k;
    double FPP;
    char* filter;
} bloomfilter;

//Struct, welches einen Floom-Filter beschreibt
//bereiche gibt die Anzahl Untertielungen an
//speicher ist der zusätzlich benötigte Speicher fuer die
//Bereichsinformationen
//False Positive Probability (FPP) --> Fehlerrate mit der ausgibt,
//dass Element entahlten, obwohl es nicht enthalten ist
//Benoetigte Speichergroesse m in Bit
//Anzahl zu speichernder Elemente n
//Optimale Anzahl Hash-Funktionen k
typedef struct
{
    int bereiche;
    int speicher_groesse_in_byte;
    char* speicher;
    bloomfilter* bf;
} floomfilter;

//Die BF-Groesse wird mittels der bekannten Formel berechnet
//n Elementanzahl
//FPP gewuenschte Falsch Positiv Rate
int berechneM(int n, double FPP)
{
    //ceil rundet dabei das Ergebnis auf die naechste Ganzzahl auf
    return (int)(ceil( (-1)*n*log(FPP) / (log(2)*log(2)) ));
}

//Die optimale Anzahl k Hash-Funktionen wird mittels der bekannten Formel berechnet
//m ist dabei die Groesse des BF in Bit
//n Elementanzahl
int berechneK(int m, int n)
{
    //round rundet dabei das Ergebnis auf eine Ganzzahl
    return (int)(round( (double)m/n * log(2) ));
}

//initialisiert den Bloomfilter mit den Parametern
//n Anzahl einzufuegender Elemente
//FPP Falsch Positiv Rate
//m Groesse wenn nicht automatisch berechnet werden soll, sonst -1 uebergeben
bloomfilter* initBF(int n, double FPP, int m)
{
    //Platz allozieren
    bloomfilter* bf = malloc(sizeof(bloomfilter));

    //Parameter zuweisen
    bf->n = n;
    bf->FPP = FPP;

    if(m > 0)
    {
        //Wenn m > 0 übergeben wird, wird dies als Groesse festgelegt
        bf->m = m;
    }
    else
    {
        //Aus den gegebenen Groessen (Anzahl einzufuegende Elemente und maximale FPP) 
        //wird m berechnet
        bf->m = berechneM(bf->n, bf->FPP);
    }
    
    #ifdef PRESENT
    printf("\n==============================================================================\n");
    printf("Der Bloom-Filter wird mit der Groesse m=%d Bit initialisiert...\n", bf->m);
    #endif

    //optimale ANzahl Hash Funktionen k berechnen
    bf->k = berechneK(bf->m, bf->n);
   
   #ifdef PRESENT
    printf("Die optimale Anzahl Hash-Funktionen k=%d wurde berechnet!\n", bf->k);
    #endif

    //Da in C nur Bytes verarbeitet werden koennen muss in Bytes umgerechnet werden
    //ceil rundet auf, damit garantiert alle Bits reinpassen
    bf->m_in_byte = ceil(bf->m/8.0f);
    
    #ifdef PRESENT
    printf("Der Bloom-Filter wird erstellt mit %d Byte...\n", bf->m_in_byte);
    #endif

    //Bloom-Filter wird mit der berechneten Groesse in Byte erstellt
    //Die evtl. maximal 7 "zu vielen" Bits werden mitverwendet, da der Platz sowieso 
    //nicht anders verwendet werden kann
    bf->filter = malloc(bf->m_in_byte * sizeof(char));

    //Sicherstellen, dass alle Werte im BF 0 sind
    for(int i = 0; i<bf->m_in_byte; i++)
    {
        bf->filter[i] = 0;
    }

    return bf;
}

//initialisiert den Floomfilter mit den Parametern
//n Anzahl einzufuegender Elemente
//FPP Falsch Positiv Rate
//m Groesse wenn nicht automatisch berechnet werden soll, sonst -1 uebergeben
//bereiche gibt die Bereichsanzahl an
floomfilter* initFF(int n, double FPP, int m, int bereiche)
{
    //speicher allozieren
    floomfilter* ff = malloc(sizeof(floomfilter));

    //greift auf BF zurueck
    ff->bf = initBF(n, FPP, m);
    ff->bereiche = bereiche;

    //in Byte
    ff->speicher_groesse_in_byte = pow(2, (ff->bereiche-3.0));

    ff->speicher = malloc( ff->speicher_groesse_in_byte * sizeof(char) );

    for(int i = 0; i<ff->speicher_groesse_in_byte; i++)
    {
        ff->speicher[i] = 0;
    }
    
    #ifdef PRESENT
    printf("\n==============================================================================\n");
    printf("Zusaetzlicher Speicherbereich mit %d Bits bzw. %d Bytes wurde initialisiert.\n", ff->speicher_groesse_in_byte*8, ff->speicher_groesse_in_byte);
    printf("Darin werden die Belegungsinformationen der %d Bereiche gespeichert.\n", ff->bereiche);
    #endif

    return ff;
}

//Befreit den Speicher des BF
void freeBF(bloomfilter* bf)
{
    free(bf->filter);
    free(bf);
}

//Befreit den Speicher des FF
void freeFF(floomfilter* ff)
{
    free(ff->speicher);
    free(ff->bf);
    free(ff);
}

//Gibt den in bf übergebenen BF binaer aus
void printBF(bloomfilter* bf)
{
    //durchlaufe den ganzen BF bitweise
    for(int i = 0; i < bf->m; i++)
    {
        //1 ausgeben wenn bit gesetzt und 0 wenn nicht
        printf("%d", (bf->filter[ i / 8 ] & (1 << ( i % 8 ))) ? 1 : 0 );

        //zur besseren Ausgabe in der Console alle 4 Bit ein Leerzeichen und alle 8 einen Trennstrich ausgeben
        if ((i % 8) == 3) printf(" ");
        else if ((i % 8) == 7) printf(" | ");
    }
    printf("\n\n");
}

//Das in element uebergebene Element wird in den in bf übergebenen Bloom-Filter 
void einfuegen(bloomfilter* bf, char* element)
{    
    int k = bf->k;
    int m = bf->m;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF setzen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des einzufuegenden Elements wird berechnet und auf die maximal moegliche Laenge des BF heruntergebrochen
        unsigned int position = murmur2(element, strlen(element), i) % m;

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        //Bit an der Stelle des Elements setzen
        bf->filter[char_position] |= (1<<bit_position);
        
        #ifdef DEBUG
        //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
        printf("Bloom: Hash %d von %s: %d (Byte: %d, Bit: %d)\n", i+1, element, position, char_position, bit_position);
        printBF(bf);
        #endif
    }
}

//Es wird geprueft, ob das in element uebergebene Element
//im in bf übergebenen Bloom-Filter vorhanden ist
//Gibt 0 aus wenn nicht enthalten
//Gibt 1 aus wenn enthalten
int pruefen(bloomfilter* bf, char* element)
{
    #ifdef DEBUG
    printf("\nWort: %s\n", element);
    #endif
    
    int k = bf->k;
    int m = bf->m;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF pruefen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des zu pruefenden Elements wird berechnet und auf die maximal moegliche 
        //Laenge des BF heruntergebrochen
        unsigned int position = murmur2(element, strlen(element), i) % m;

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        #ifdef DEBUG
        printf("%d: Byte: %d, Bit: %d\n", i, char_position, bit_position);
        #endif

        //Bit an der Stelle des Elements testen
        if(!(bf->filter[char_position] & (1<<bit_position)))
        {
            #ifdef DEBUG
            printf("%s ist sicher nicht im BF enthalten.\n", element);
            #endif

            //Wenn das Bit an der Stelle 0 ist kann das Element nicht enthalten sein --> Abbruch
            return 0;
        }
        
    }

    #ifdef DEBUG
    printf("Es wird vermutet, dass %s im BF enthalten ist.\n", element);
    #endif

    //Wird die Schleife vollstaendig durchlaufen, muessen immer Einsen an den Stellen gestanden
    //haben --> Vermutung, dass enthalten
    return 1;
}

//Das in element uebergebene Element wird in den in ff übergebenen Floom-Filter 
void einfuegenFF(floomfilter* ff, char* element)
{    
    //Initialisierung der zu setzenden Position im Speicher
    int speicher_position = 0;

    int k = ff->bf->k;
    int m = ff->bf->m;
    int bereiche = ff->bereiche;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF setzen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des einzufuegenden Elements wird berechnet und auf die maximal moegliche
        // Laenge des FF heruntergebrochen
        unsigned int position = murmur2(element, strlen(element), i) % m;

        //Zaehlvariable j fuer Bereichsermittlung
        int j=1;
        //Bereichsgrenzen durchlaufen, um zu testen in welchem der Hash Wert liegt
        while(position > j*m/bereiche){
            j++;
        }

        //Speicherpositionsbit setzen je nach Bereich in dem Hash lag
        speicher_position = speicher_position | (1<<(j-1));

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        //Bit an der Stelle des Elements setzen
        ff->bf->filter[char_position] |= (1<<bit_position);
        
        #ifdef DEBUG
        //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
        printf("Floom: Hash %d von %s: %d (Byte: %d, Bit: %d)\n", i+1, element, position, char_position, bit_position);
        printf("Speicherposition liegt in Bereich %d von %d.\n", j, bereiche);
        printf("Bereichsgroesse: %d Byte bzw. %d Bit.\n", m/bereiche/8, m/bereiche);
        printBF(ff->bf);
        #endif
    }

    //Umrechnung der Speicherposition in Byte und Bit
    unsigned int char_position = speicher_position / 8;
    unsigned int bit_position = speicher_position % 8;
    //Bit im Speicher setzen in Abhaengigkeit von berechneter Position 
    ff->speicher[char_position] |= (1 << bit_position);

    #ifdef DEBUG
    //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
    printf("%s wurde an der Bitstelle %d in den extra Speicher eingefuegt.\n", element, speicher_position);
    printBF(ff->speicher, ff->speicher_groesse_in_byte*8);
    #endif
}

//Es wird geprueft, ob das in element uebergebene Element
//im in ff übergebenen Floom-Filter vorhanden ist
//Gibt 0 aus wenn nicht enthalten
//Gibt 1 aus wenn enthalten
int pruefenFF(floomfilter* ff, char* element)
{
    #ifdef DEBUG
    printf("\nWort: %s\n", element);
    #endif
    //Initialisiere Speicherposition
    int speicher_position = 0;

    int k = ff->bf->k;
    int m = ff->bf->m;
    int bereiche = ff->bereiche;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF pruefen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des zu pruefenden Elements wird berechnet und auf die maximal moegliche 
        //Laenge des FF heruntergebrochen
        unsigned int position = murmur2(element, strlen(element), i) % m;

        //Zaehlvariable j fuer Bereichsermittlung
        int j=1;
        //Bereichsgrenzen durchlaufen, um zu testen in welchem der Hash Wert liegt
        while(position > j*m/bereiche){
            j++;
        }

        //Speicherpositionsbit setzen je nach Bereich in dem Hash lag
        speicher_position = speicher_position | (1<<(j-1));

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        #ifdef DEBUG
        printf("%d: Byte: %d, Bit: %d\n", i, char_position, bit_position);
        #endif

        //Bit an der Stelle des Elements testen
        if(!(ff->bf->filter[char_position] & (1<<bit_position)))
        {
            #ifdef DEBUG
            printf("%s ist sicher nicht im FF enthalten.\n", element);
            #endif

            //Wenn das Bit an der Stelle 0 ist kann das Element nicht enthalten sein --> Abbruch
            return 0;
        }
        
    }

    //Umrechnung der Speicherposition in Byte und Bit
    unsigned int char_position = speicher_position / 8;
    unsigned int bit_position = speicher_position % 8;
    //Zusaetzliche Bereichspruefung
    if(!(ff->speicher[char_position] & (1 << bit_position)))
    {
        #ifdef DEBUG
        printf("%s ist sicher nicht im FF enthalten.\n", element);
        printf("Feststellung dank extra Speicher!\n");
        printBF(ff->speicher, ff->speicher_groesse_in_byte * 8 );
        #endif
        return 0;
    }


    #ifdef DEBUG
    printf("Es wird vermutet, dass %s enthalten ist.\n", element);
    #endif
    //Wird die Schleife vollstaendig durchlaufen, muessen immer Einsen an den Stellen gestanden
    //haben und wenn im Speicher an der Bereichsstelle auch eine Eins war --> Vermutung, dass enthalten
    return 1;
}
