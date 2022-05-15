#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include "bloomfilter.h"

//BF steht fuer Bloom-Filter
//False Positive Probability (FPP) --> Fehlerrate mit der ausgibt,
//dass Element entahlten, obwohl es nicht enthalten ist
#define FPP 0.01

//Benoetigte Speichergroesse m in Bit

//Anzahl zu speichernder Elemente n

//Optimale Anzahl Hash-Funktionen k

//Formeln:
//m = - n*ln(FPP)/(ln(2))^2
//k = m/n * ln(2)
#define bereiche 8

#define filename "numbers.txt"

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"


void compare(int n , double* fpp_bloom, double* fpp_floom, double* fpp_big_bloom)
{
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

void compareTime(int n , double* in_bloom, double* in_floom, double* in_big_bloom, double* check_bloom, double* check_floom, double* check_big_bloom)
{

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
    //Sicherstellen, dass allo Werte im BBF 0 sind

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

    //Zeitmessung initialisieren
    double zeitEinfuegenBF = 0;
    double zeitEinfuegenFF = 0;
    double zeitEinfuegenBBF = 0;


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

        clock_t begin = clock();
        einfuegen(bloom, word);
        clock_t end = clock();
        //printf("Zeit pro Einfuegen BF: %f\n", (double)(end - begin));
        zeitEinfuegenBF += (double)(end - begin) / CLOCKS_PER_SEC;
        //printf("%f\n", zeitEinfuegenBF);
        begin = clock();
        einfuegenFF(floom, word);
        end = clock();
        //printf("Zeit pro Einfuegen FF: %.10f\n", (double)(end - begin));
        zeitEinfuegenFF += (double)(end - begin) / CLOCKS_PER_SEC;

        begin = clock();
        einfuegen(big_bloom, word);
        end = clock();
        //printf("Zeit pro Einfuegen BBF: %.10f\n", (double)(end - begin));
        zeitEinfuegenBBF += (double)(end - begin) / CLOCKS_PER_SEC;

    }


    *in_bloom = zeitEinfuegenBF/n;
    *in_floom = zeitEinfuegenFF/n;
    *in_big_bloom = zeitEinfuegenBBF/n;

    //Zeitenmessung initialisieren
    double zeitPruefenBF = 0;
    double zeitPruefenFF = 0;
    double zeitPruefenBBF = 0;
    int gesamt =0;
    //Restliche Zahlen durchlaufen und auf Mitgliedschaft im BF testen
    while(fscanf(file, "%s", word) != EOF)// && gesamt < n) //Anzahl 
    //einzulesender Elemente beschraenken fuer lesbarere Ausgabe
    {
        gesamt++;
        //Da in der Liste keine Zahl doppelt steht, ist die korrekte Antwort immer 
        //"Nicht enthalten". Darum ist jede Antwort "Enthalten" automatisch falsch
        clock_t begin = clock();
        int resultat = pruefen(bloom, word);
        clock_t end = clock();
        //printf("Zeit pro Pruefen BF: %f\n", (double)(end - begin));
        zeitPruefenBF += (double)(end - begin) / CLOCKS_PER_SEC;

        begin = clock();
        resultat = pruefenFF(floom, word);
        end = clock();
        //printf("Zeit pro Pruefen FF: %f\n", (double)(end - begin));
        zeitPruefenFF += (double)(end - begin) / CLOCKS_PER_SEC;

        begin = clock();
        resultat = pruefen(big_bloom, word);
        end = clock();
        //printf("Zeit pro Pruefen BBF: %f\n", (double)(end - begin));
        zeitPruefenBBF += (double)(end - begin) / CLOCKS_PER_SEC;

        #ifdef DEBUG
            printf("\nGesamtanzahl: %d, Bislang falsch-positive beim BF: %d\n",  gesamt, falschpositivBF);
            printf("Gesamtanzahl: %d, Bislang falsch-positive beim FF: %d\n",  gesamt, falschpositivFF);
            printf("Gesamtanzahl: %d, Bislang falsch-positive beim BBF: %d\n",  gesamt, falschpositivBBF);

        #endif
    }
    *check_bloom = zeitPruefenBF/n;
    *check_floom = zeitPruefenFF/n;
    *check_big_bloom = zeitPruefenBBF/n;

    //Input Datei schliessen
    fclose(file);

    //Bloom Filter freigeben
    freeBF(bloom);

    freeFF(floom);

    freeBF(big_bloom);

}

int main(){

    FILE* einfuegen = fopen("einfuegen_0.01_8.txt", "w");
    FILE* pruefen = fopen("pruefen_0.01_8.txt", "w");


    double in_bloom;
    double in_floom;
    double in_big_bloom;
    double check_bloom;
    double check_floom;
    double check_big_bloom;

        for(int i=1000; i<500000; i += 1000){

                compareTime(i,&in_bloom, &in_floom, &in_big_bloom, &check_bloom, &check_floom, &check_big_bloom);
                //fprintf(write, "%d %f %f %f\n", i,fpp_bloom, fpp_floom, fpp_big_bloom);

                fprintf(einfuegen, "%d %.15f %.15f %.15f\n", i,in_bloom, in_floom, in_big_bloom);
                fprintf(pruefen, "%d %.15f %.15f %.15f\n", i,check_bloom, check_floom, check_big_bloom);


                //printf("%d %.15f %.15f %.15f\n", i,in_bloom, in_floom, in_big_bloom);

                if(i % 50000 == 0)
                {
                    printf("%d\n", i);
                    fflush(einfuegen);
                    fflush(pruefen);

                }
        }
    

    fflush(einfuegen);
   fflush(pruefen);

    fclose(einfuegen);
    fclose(pruefen);
}