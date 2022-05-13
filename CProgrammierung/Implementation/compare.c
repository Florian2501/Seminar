#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "bloomfilter.h"

//BF steht fuer Bloom-Filter
//False Positive Probability (FPP) --> Fehlerrate mit der ausgibt,
//dass Element entahlten, obwohl es nicht enthalten ist

//Benoetigte Speichergroesse m in Bit

//Anzahl zu speichernder Elemente n

//Optimale Anzahl Hash-Funktionen k

//Formeln:
//m = - n*ln(FPP)/(ln(2))^2
//k = m/n * ln(2)

#define filename "numbers.txt"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"
/*

void compare(int n, double FPP, int bereiche, double* fpp_floom, double* fpp_big_bloom)
{
    printf("Elementanzahl: \t\t%d\n", n);
    printf("Falsch Positiv Rate: \t%.5f\n", FPP);
    printf("Bereiche: \t\t%d\n", bereiche);


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

        //Zeitmessungsschnipsel stammt von Thomas Pornin von Stackoverflow
        //https://stackoverflow.com/questions/5248915/execution-time-of-c-program

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
    while(fscanf(file, "%s", word) != EOF)// && gesamt < n) //Anzahl 
    //einzulesender Elemente beschraenken fuer lesbarere Ausgabe
    {
        gesamt++;

        //Da in der Liste keine Zahl doppelt steht, ist die korrekte Antwort immer 
        //"Nicht enthalten". Darum ist jede Antwort "Enthalten" automatisch falsch
        int resultat = pruefen(bloom, word, k, m_in_byte*8);

        if(resultat)
        {
            falschpositivBF++;
        }
        resultat = pruefenFF(floom, word, k, m_in_byte*8, speicher, bereiche);


        if(resultat)
        {
            falschpositivFF++;
        }

        resultat = pruefen(big_bloom, word, k, (m_in_byte + speicher_groesse_in_byte) * 8);

        if(resultat)
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
    printf("GESAMT: %d\n", gesamt);

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
    printf("FPP FF: %d\n", falschpositivFF);

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
    printf("FPP BBF: %d\n", falschpositivBBF);

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

    *fpp_floom = fpp_gemessenFF;
    *fpp_big_bloom = fpp_gemessenBBF;
}


void compare2(int n, double FPP, int bereiche, double* fpp_floom, double* fpp_big_bloom)
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Standard Bloom Filter

    //Aus den gegebenen Groessen (Anzahl einzufuegende Elemente und maximale FPP) 
    //werden m und k berechnet
    int m = berechneM(n, FPP);

    int k = berechneK(m, n);

    //Da in C nur Bytes verarbeitet werden koennen muss in Bytes umgerechnet werden
    //ceil rundet auf, damit garantiert alle Bits reinpassen
    int m_in_byte = ceil(m/8.0f);

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Big Bloom

    //Bloom Filter mit zusatzlicher Groesse des Speicherbereichs vom FF
    char* big_bloom = malloc((m_in_byte + speicher_groesse_in_byte) * sizeof(char));
    //Sicherstellen, dass allo Werte im BBF 0 sind
    for(int i = 0; i< (m_in_byte + speicher_groesse_in_byte); i++)
    {
        big_bloom[i] = 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    FILE* file = fopen(filename, "r");

    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        return;
    }

    //Puffer fuer aktuell hinzuzufuegendes Wort
    char word[255];

    for(int i = 0; i < n; i++)
    {
        //Wort einlesen und pruefen, ob das Dateiende erreicht wird
        if(fscanf(file, "%s", word) == EOF)
        {
            break;
        }

        //Wort in Bloom-Filter einfuegen
        //Da der uebrige Platz im letzten evtl. angerissenen Byte mitgenutzt werden soll, wird nicht die
        //genaue Anzahl Bit angegeben, die durch die Formel errechnet wurde, sondern die auf volle Byte 
        //aufgerundete Anzahl Bit, da der Platz im evtl. angerissenen Byte sonst verloren ginge
        einfuegenFF(floom, word, k, m_in_byte*8, speicher, bereiche);

        einfuegen(big_bloom, word, k, (m_in_byte + speicher_groesse_in_byte) * 8);
    }

    //Gibt den gesamten BF aus
    //printBF(bloom, m_in_byte*8);

    //Zaehlvariablen fuer die Evaluation des BF
    int gesamt = 0;

    int falschpositivFF = 0;

    int falschpositivBBF = 0;

    //Restliche Zahlen durchlaufen und auf Mitgliedschaft im BF testen
    while(fscanf(file, "%s", word) != EOF)// && gesamt < n)
    {
        gesamt++;

        //Da in der Liste keine Zahl doppelt steht, ist die korrekte Antwort immer 
        //"Nicht enthalten". Darum ist jede Antwort "Enthalten" automatisch falsch

        if(pruefenFF(floom, word, k, m_in_byte*8, speicher, bereiche))
        {
            falschpositivFF++;
        }

        if(pruefen(big_bloom, word, k, (m_in_byte + speicher_groesse_in_byte) * 8))
        {
            falschpositivBBF++;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //FF

    //Ermittelte FPP Rate berechnen
    //printf("GESAMT: %d\n", gesamt);
    //printf("FPP FF: %d\n", falschpositivFF);
    *fpp_floom = (double)falschpositivFF / gesamt;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BBF

    //Ermittelte FPP Rate berechnen
    //printf("FPP BBF: %d\n", falschpositivBBF);
    *fpp_big_bloom = (double)falschpositivBBF / gesamt;

    //words.txt schliessen
    fclose(file);
    //Bloom Filter freigeben
    free(floom);
    free(big_bloom);
    free(speicher);

}


*/
void compare3(int n , double* fpp_bloom, double* fpp_floom, double* fpp_big_bloom)
{
    double FPP =0.05;
    int bereiche=16;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Standard Bloom Filter
    bloomfilter* bloom = initBF(n, FPP, -1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Floom Filter
    floomfilter* floom = initFF(n, FPP, bloom->m, bereiche);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Big Bloom

    //Bloom Filter mit zusatzlicher Groesse des Speicherbereichs vom FF
    bloomfilter* big_bloom = initBF(n, FPP, (bloom->m_in_byte + floom->speicher_groesse_in_byte) * 8);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    FILE* file = fopen(filename, "r");

    //Testen, ob Oeffnen funktioniert hat
    if(file == NULL)
    {
        printf("Fehler beim oeffnen der Textdatei %s. Abbruch des Fuellens!\n", filename);
        return;
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

        //Zeitmessungsschnipsel stammt von Thomas Pornin von Stackoverflow
        //https://stackoverflow.com/questions/5248915/execution-time-of-c-program

        einfuegen(bloom, word);
        einfuegenFF(floom, word);
        einfuegen(big_bloom, word);
    }
    //Gibt den gesamten BF aus
    //printBF(bloom, m_in_byte*8);

    //Zaehlvariablen fuer die Evaluation des BF
    int gesamt = 0;
    int falschpositivBF = 0;

    int falschpositivFF = 0;

    int falschpositivBBF = 0;

    //Restliche Zahlen durchlaufen und auf Mitgliedschaft im BF testen
    while(fscanf(file, "%s", word) != EOF)// && gesamt < n) //Anzahl 
    //einzulesender Elemente beschraenken fuer lesbarere Ausgabe
    {
        gesamt++;

        //Da in der Liste keine Zahl doppelt steht, ist die korrekte Antwort immer 
        //"Nicht enthalten". Darum ist jede Antwort "Enthalten" automatisch falsch
        int resultat = pruefen(bloom, word);

        if(resultat)
        {
            falschpositivBF++;
        }

        resultat = pruefenFF(floom, word);

        if(resultat)
        {
            falschpositivFF++;
        }

        resultat = pruefen(big_bloom, word);
        if(resultat)
        {
            falschpositivBBF++;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BF

    //Aus den gemssenen Anzahlen kann eine experimentell ermittelte FPP berechnet werden
    *fpp_bloom = (double)falschpositivBF / gesamt;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //FF

    //Ermittelte FPP Rate berechnen
    *fpp_floom = (double)falschpositivFF / gesamt;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BBF

    //Ermittelte FPP Rate berechnen
    *fpp_big_bloom = (double)falschpositivBBF / gesamt;
    
    //Input Datei schliessen
    fclose(file);
    //Bloom Filter freigeben
    freeBF(bloom);
    freeFF(floom);
    freeBF(big_bloom);

    return;
}

int main(){
    /*double fpp[6] = {0.05, 0.02, 0.01, 0.005, 0.002, 0.001};

    FILE* write = fopen("compare.txt", "w");

    for(int b=8; b<24; b+=2){
        for(int i=10; i<500000; i += 100){
            for(int j=0; j<6; j++){
                if(compare(i, fpp[j], b) < 0){
                    fprintf(write, "%d;%f;%d\n", i, fpp[j], b);
                }
            }
        }
    }*/

    FILE* write = fopen("graph_0.05_16.txt", "w");


    int b = 8;
    double rate = 0.05;
    double fpp_bloom;
    double fpp_floom;
    double fpp_big_bloom;

        for(int i=1000; i<500000; i += 1000){
                compare3(i,&fpp_bloom, &fpp_floom, &fpp_big_bloom);
                fprintf(write, "%d %f %f %f\n", i,fpp_bloom, fpp_floom, fpp_big_bloom);
                //printf("%d %f %f %f\n", i,fpp_bloom, fpp_floom, fpp_big_bloom);

                if(i % 50000 == 0)
                {
                    printf("%d\n", i);
                    fflush(write);
                }
        }
    

    fflush(write);

    fclose(write);
    //compare(2000, 0.05, 8, &fpp_floom, &fpp_big_bloom);
    //compare2(2000, 0.05, 8, &fpp_floom, &fpp_big_bloom);

    //printf("floom: %f\nbig: %f\n", fpp_floom, fpp_big_bloom);
}