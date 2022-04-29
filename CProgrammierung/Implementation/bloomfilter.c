#include <stdio.h>
#include <math.h>

#include "murmur.h"

//BF steht fuer Bloom-Filter
//False Positive Probability (FPP) --> Fehlerrate mit der ausgibt,
//dass Element entahlten, obwohl es nicht enthalten ist
#define FALSEPOSITIVE 0.01

//Benoetigte Speichergroesse m in Bit

//Anzahl zu speichernder Elemente n
#define N 50000

//Optimale Anzahl Hash-Funktionen k

//Formeln:
//m = - n*ln(FPP)/(ln(2))^2
//k = m/n * ln(2)

#define BEREICHE 8

#define filename "numbers.txt"

//#define DEBUG

//Farbwerte von https://stackoverflow.com/questions/3585846/color-text-in-terminal-applications-in-unix
//aus dem Beitrag von David Guyon
//aufgerufen am 28.04.2022 um 18:45 Uhr
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


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

int main(int argc, char** argv)
{
    int n;
    double FPP;
    int bereiche;

    //Parameter einlesen
    switch (argc)
    {
    case 1:
        n = (int)*argv[0];
        FPP = FALSEPOSITIVE;
        bereiche = BEREICHE;
        break;
    case 2:
        n = (int)*argv[0];
        FPP = (double)*argv[1];
        bereiche = BEREICHE;
        break;
    case 3:
        n = (int)*argv[0];
        FPP = (int)*argv[1];
        bereiche = (int)*argv[2];
        break;
    
    default:
        printf("Nutzung: bloom [Elementanzhal] [Falsch Positiv Rate] [Bereichsanzahl]\n");
        printf("Bei auslassen werden Standardwerte verwendet:\n");
        printf("Elementanzhal: \t%d", N);
        printf("Falsch Positiv Rate: \t%f", FALSEPOSITIVE);
        printf("Bereiche: \t%d", BEREICHE);
        n = N;
        FPP = FALSEPOSITIVE;
        bereiche = BEREICHE;
        break;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Standard Bloom Filter

    //Aus den gegebenen Groessen (Anzahl einzufuegende Elemente und maximale FPP) 
    //werden m und k berechnet
    int m = berechneM(n, FPP);
    printf("Der Bloom-Filter wird mit der Groesse m=%d Bit initialisiert...\n", m);

    int k = berechneK(m, n);
    printf("Die optimale Anzahl Hash-Funktionen k=%d wurde berechnet!\n", k);

    //Da in C nur Bytes verarbeitet werden koennen muss in Bytes umgerechnet werden
    //ceil rundet auf, damit garantiert alle Bits reinpassen
    int m_in_byte = ceil(m/8.0f);

    printf("Der Bloom-Filter wird erstellt mit %d Byte...\n", m_in_byte);

    //Bloom-Filter wird mit der berechneten Groesse in Byte erstellt
    //Die evtl. maximal 7 "zu vielen" Bits werden mitverwendet, da der Platz sowieso 
    //nicht anders verwendet werden kann
    char* bloom = malloc(m_in_byte * sizeof(char));

    //Sicherstellen, dass allo Werte im BF 0 sind
    for(int i = 0; i<m_in_byte; i++)
    {
        bloom[i] = 0;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Floom Filter

    char* floom =  malloc(m_in_byte * sizeof(char));

    //Sicherstellen, dass allo Werte im FF 0 sind
    for(int i = 0; i<m_in_byte; i++)
    {
        floom[i] = 0;
    }

    //in Byte
    int speicher_groesse_in_byte = pow(2, (bereiche-3.0));
    char* speicher = malloc( speicher_groesse_in_byte * sizeof(char) );

    for(int i = 0; i<speicher_groesse_in_byte; i++)
    {
        speicher[i] = 0;
    }

    printf("\n==============================================================================\n");
    printf("Zusaetzlicher Speicherbereich mit %d Bits bzw. %d Bytes wurde initialisiert.\n", speicher_groesse_in_byte*8, speicher_groesse_in_byte);
    printf("Darin werden die Belegungsinformationen der %d Bereiche gespeichert.\n", bereiche);



    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Big Bloom

    //Bloom Filter mit zusatzlicher Groesse des Speicherbereichs vom FF
    char* big_bloom = malloc((m_in_byte + speicher_groesse_in_byte) * sizeof(char));
    //Sicherstellen, dass allo Werte im BBF 0 sind
    for(int i = 0; i< (m_in_byte + speicher_groesse_in_byte); i++)
    {
        big_bloom[i] = 0;
    }

    printf("\n==============================================================================\n");
    printf("Big Bloom Filter mit %d Bytes wurde initialisiert.\n", speicher_groesse_in_byte + m_in_byte);
    printf("Er hat die Groesse des BF ergaenzt um die Groesse des Bereichsspeichers.\n");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    printf("\n==============================================================================\n");
    printf("Fuellen des Bloom-Filters mit %d zufaelligen Zahlen...\n", n);

    FILE* file = fopen(filename, "r");

    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        printf("Fehler beim oeffnen der Textdatei %s. Abbruch des Fuellens!\n", filename);
        return EXIT_FAILURE;
    }

    //Puffer fuer aktuell hinzuzufuegendes Wort
    char word[255];

    for(int i = 0; i < n; i++)
    {
        //Wort einlesen und pruefen, ob das Dateiende erreicht wird
        if(fscanf(file, "%s", word) == EOF)
        {
            printf("=============================================================\n");
            printf("Die Anzahl einzulesender Elemente ueberschreitet die Anzahl ");
            printf("vorhandener Elemente in der Datei.\n");
            printf("Abbruch nach %d gelesenen Elementen.\n", i);
            break;
        }

        //Wort in Bloom-Filter einfuegen
        //Da der uebrige Platz im letzten evtl. angerissenen Byte mitgenutzt werden soll, wird nicht die
        //genaue Anzahl Bit angegeben, die durch die Formel errechnet wurde, sondern die auf volle Byte 
        //aufgerundete Anzahl Bit, da der Platz im evtl. angerissenen Byte sonst verloren ginge
        einfuegen(bloom, word, k, m_in_byte*8);

        einfuegenFF(floom, word, k, m_in_byte*8, speicher, bereiche);

        einfuegen(big_bloom, word, k, (m_in_byte + speicher_groesse_in_byte) * 8);
    }

    //Gibt den gesamten BF aus
    //printBF(bloom, m_in_byte*8);

    //Zaehlvariablen fuer die Evaluation des BF
    int gesamt = 0;
    int falschpositivBF = 0;

    int falschpositivFF = 0;

    int falschpositivBBF = 0;

    //Restliche Zahlen durchlaufen und auf Mitgliedschaft im BF testen
    while(fscanf(file, "%s", word) != EOF) // && gesamt < n)
    {
        gesamt++;

        //Da in der Liste keine Zahl doppelt steht, ist die korrekte Antwort immer 
        //"Nicht enthalten". Darum ist jede Antwort "Enthalten" automatisch falsch
        if(pruefen(bloom, word, k, m_in_byte*8))
        {
            falschpositivBF++;
        }

        if(pruefenFF(floom, word, k, m_in_byte*8, speicher, bereiche))
        {
            falschpositivFF++;
        }

        if(pruefen(big_bloom, word, k, (m_in_byte + speicher_groesse_in_byte) * 8))
        {
            falschpositivBBF++;
        }

        #ifdef DEBUG
            printf("\nGesamtanzahl: %d, Bislang falsch-positive beim BF: %d\n",  gesamt, falschpositivBF);
            printf("Gesamtanzahl: %d, Bislang falsch-positive beim FF: %d\n",  gesamt, falschpositivFF);
            printf("Gesamtanzahl: %d, Bislang falsch-positive beim BBF: %d\n",  gesamt, falschpositivBBF);

        #endif
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BF

    //Aus den gemssenen Anzahlen kann eine experimentell ermittelte FPP berechnet werden
    double fpp_gemessen = (double)falschpositivBF / gesamt;

    printf("\n==============================================================================\n");
    printf("Vergleich der theoretischen und der gemessenen False Positive Probability:\n");
    printf("Vorgegebene FPP: \t%1.5f\n", FPP);
    printf("Gemessene FPP im BF: \t%1.5f\n", fpp_gemessen);

    //Prozentuale Abweichung berechnen
    double abweichung = ((fpp_gemessen / FPP) - 1) * 100.0;
    printf("Abweichung in Prozent: ");
    if(abweichung > 0)
    {
        printf(RED);
    }
    else if(abweichung < 0)
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichung);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //FF

    //Ermittelte FPP Rate berechnen
    double fpp_gemessenFF = (double)falschpositivFF / gesamt;
    
    printf("\nGemessene FPP im FF: \t%1.5f\n", fpp_gemessenFF);

    //Prozentuale Abweichung berechnen
    double abweichungFF = ((fpp_gemessenFF / FPP) - 1) * 100.0;
    printf("Abweichung in Prozent: ");
    if(abweichungFF > 0)
    {
        printf(RED);
    }
    else if(abweichungFF < 0)
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichungFF);

    printf("\nVerbeserung durch extra Speicher: ");
    if(abweichung-abweichungFF > 0) 
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichung-abweichungFF);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BBF

    //Ermittelte FPP Rate berechnen
    double fpp_gemessenBBF = (double)falschpositivBBF / gesamt;
    
    printf("\nGemessene FPP im BBF: \t%1.5f\n", fpp_gemessenBBF);

    //Prozentuale Abweichung berechnen
    double abweichungBBF = ((fpp_gemessenBBF / FPP) - 1) * 100.0;
    printf("Abweichung in Prozent: ");
    if(abweichungBBF > 0)
    {
        printf(RED);
    }
    else if(abweichungBBF < 0)
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichungBBF);

    printf("\nVerbeserung durch groesseren BF: ");
    if(abweichung-abweichungBBF > 0) 
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichung-abweichungBBF);

    //words.txt schliessen
    fclose(file);
    //Bloom Filter freigeben
    free(bloom);
    free(floom);
    free(big_bloom);
    free(speicher);

    return EXIT_SUCCESS;
}
