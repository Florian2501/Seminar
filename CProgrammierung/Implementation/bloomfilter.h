#include <stdio.h>
#include <math.h>

#include "murmur.h"
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

//Das in element uebergebene Element wird in den in bloom übergebenen Bloom-Filter 
//eingefügt mittels k Hash-Funktionen
//m ist die Länge des BF in Bit
void einfuegen(char* bloom, char* element, int k, int m)
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
        printf("Bloom: Hash %d von %s: %d (Byte: %d, Bit: %d)\n", i+1, element, position, char_position, bit_position);
        printBF(bloom, m);
        #endif
    }
}

//Es wird geprueft, ob das in element uebergebene Element
//im in bloom übergebenen Bloom-Filter vorhanden ist
//m ist die Länge des BF in Bit
//k ist die Anzahl Hash Funktionen
//Gibt 0 aus wenn nicht enthalten
//Gibt 1 aus wenn enthalten
int pruefen(char* bloom, char* element, int k, int m)
{
    #ifdef DEBUG
    printf("\nWort: %s\n", element);
    #endif

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF pruefen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des zu pruefenden Elements wird berechnet und auf die maximal moegliche 
        //Laenge des BF heruntergebrochen
        unsigned int position = murmur2(element, sizeof(element), i) % m;

        //Umrechnung der Position des Elements in Byte und Einzelbits im dann referenzierten Byte
        unsigned int char_position = position / 8;
        unsigned int bit_position = position % 8;

        #ifdef DEBUG
        printf("%d: Byte: %d, Bit: %d\n", i, char_position, bit_position);
        #endif

        //Bit an der Stelle des Elements testen
        if(!(bloom[char_position] & (1<<bit_position)))
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

//Das in element uebergebene Element wird in den in floom übergebenen Floom-Filter 
//eingefügt mittels k Hash-Funktionen
//In den in speicher uebergebenen Bereich wird die Bereichsverteilung eingetragen
//Dabei gibt bereiche die Bereichsanzahl an
//m ist die Länge des BF in Bit
void einfuegenFF(char* floom, char* element, int k, int m, char* speicher, int bereiche)
{    
    //Initialisierung der zu setzenden Position im Speicher
    int speicher_position = 0;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF setzen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des einzufuegenden Elements wird berechnet und auf die maximal moegliche
        // Laenge des FF heruntergebrochen
        unsigned int position = murmur2(element, sizeof(element), i) % m;

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
        floom[char_position] |= (1<<bit_position);
        
        #ifdef DEBUG
        //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
        printf("Floom: Hash %d von %s: %d (Byte: %d, Bit: %d)\n", i+1, element, position, char_position, bit_position);
        printf("Speicherposition liegt in Bereich %d von %d.\n", j, bereiche);
        printf("Bereichsgroesse: %d Byte bzw. %d Bit.\n", m/bereiche/8, m/bereiche);
        printBF(floom, m);
        #endif
    }

    //Umrechnung der Speicherposition in Byte und Bit
    unsigned int char_position = speicher_position / 8;
    unsigned int bit_position = speicher_position % 8;
    //Bit im Speicher setzen in Abhaengigkeit von berechneter Position 
    speicher[char_position] |= (1 << bit_position);

    #ifdef DEBUG
    //Ausgabe der Hash Werte und des ganzen BF zur Verdeutlichung
    printf("%s wurde an der Bitstelle %d in den extra Speicher eingefuegt.\n", element, speicher_position);
    printBF(speicher, pow(2, (bereiche-3.0)) * 8);
    #endif
}

//Es wird geprueft, ob das in element uebergebene Element
//im in floom übergebenen Floom-Filter vorhanden ist
//m ist die Länge des FF in Bit
//k ist die Anzahl Hash Funktionen
//Die zusaetzliche Ueberpruefung der Mitgliedschaft des Elements
//erfolgt ueber den in speicher uebergebenen Bereichsspeicher
//Dabei gibt bereiche die Bereichsanzahl an
//Gibt 0 aus wenn nicht enthalten
//Gibt 1 aus wenn enthalten
int pruefenFF(char* floom, char* element, int k, int m, char* speicher, int bereiche)
{
    #ifdef DEBUG
    printf("\nWort: %s\n", element);
    #endif
    //Initialisiere Speicherposition
    int speicher_position = 0;

    //Pro Element entsprechend der Anzahlen der Hash Funktionen Bits im BF pruefen
    for(int i = 0; i < k; i++)
    {
        //Murmur2 Hash des zu pruefenden Elements wird berechnet und auf die maximal moegliche 
        //Laenge des FF heruntergebrochen
        unsigned int position = murmur2(element, sizeof(element), i) % m;

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
        if(!(floom[char_position] & (1<<bit_position)))
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
    if(!(speicher[char_position] & (1 << bit_position)))
    {
        #ifdef DEBUG
        printf("%s ist sicher nicht im FF enthalten.\n", element);
        printf("Feststellung dank extra Speicher!\n");
        printBF(speicher, pow(2, (bereiche-3.0)) * 8 );
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
