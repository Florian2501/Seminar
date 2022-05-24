#include <stdio.h>
#include <math.h>
#include <time.h>

#include "bloomfilter.h"

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

int main(int argc, char** argv)
{
    int n;
    double FPP;
    int bereiche;

    //Parameter einlesen
    switch (argc)
    {
    case 2:
        n = atoi(argv[1]);
        FPP = FALSEPOSITIVE;
        bereiche = BEREICHE;
        break;
    case 3:
        n = atoi(argv[1]);
        FPP = atof(argv[2]);
        bereiche = BEREICHE;
        break;
    case 4:
        n = atoi(argv[1]);
        FPP = atof(argv[2]);
        bereiche = atoi(argv[3]);
        break;
    
    default:
        printf("Nutzung: bloom [Elementanzahl] [Falsch Positiv Rate] [Bereichsanzahl]\n");
        printf("Bei auslassen werden Standardwerte verwendet:\n");
        n = N;
        FPP = FALSEPOSITIVE;
        bereiche = BEREICHE;
        break;
    }

    printf("Elementanzahl: \t\t%d\n", n);
    printf("Falsch Positiv Rate: \t%.5f\n", FPP);
    printf("Bereiche: \t\t%d\n", bereiche);


    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Standard Bloom Filter
    printf(GRN "\nSTANDARD BLOOMFILTER:\n" RESET);
    bloomfilter* bloom = initBF(n, FPP, -1);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Floom Filter
    printf(GRN "\nFLOOMFILTER:\n" RESET);
    floomfilter* floom = initFF(n, FPP, bloom->m, bereiche);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //Big Bloom
    printf(GRN "\nBIG BLOOMFILTER:\n" RESET);

    //Bloom Filter mit zusatzlicher Groesse des Speicherbereichs vom FF
    bloomfilter* big_bloom = initBF(n, FPP, (bloom->m_in_byte + floom->speicher_groesse_in_byte) * 8);

    //Sicherstellen, dass alle Werte im BBF 0 sind

    printf("\n==============================================================================\n");
    printf("Big Bloom Filter mit %d Bytes wurde initialisiert.\n", big_bloom->m_in_byte);
    printf("Er hat die Groesse des BF ergaenzt um die Groesse des Bereichsspeichers.\n");

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    printf(GRN "\nBEFUELLEN:\n" RESET);
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
        //printf("Zeit pro Einfuegen BF: %.10f\n", (double)(end - begin));
        zeitEinfuegenBF += (double)(end - begin) / CLOCKS_PER_SEC;

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

    printf("Durchschnittliche Zeit pro Einfuegen BF: \t%.10f\n", zeitEinfuegenBF/n);
    printf("Durchschnittliche Zeit pro Einfuegen FF: \t%.10f\n", zeitEinfuegenFF/n);
    printf("Durchschnittliche Zeit pro Einfuegen BBF: \t%.10f\n", zeitEinfuegenBBF/n);


    //Gibt den gesamten BF aus
    //printBF(bloom, m_in_byte*8);

    //Zaehlvariablen fuer die Evaluation des BF
    int gesamt = 0;
    int falschpositivBF = 0;

    int falschpositivFF = 0;

    int falschpositivBBF = 0;

    //Zeitenmessung initialisieren
    double zeitPruefenBF = 0;
    double zeitPruefenFF = 0;
    double zeitPruefenBBF = 0;

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

        if(resultat)
        {
            falschpositivBF++;
        }

        begin = clock();
        resultat = pruefenFF(floom, word);
        end = clock();
        //printf("Zeit pro Pruefen FF: %f\n", (double)(end - begin));
        zeitPruefenFF += (double)(end - begin) / CLOCKS_PER_SEC;

        if(resultat)
        {
            falschpositivFF++;
        }

        begin = clock();
        resultat = pruefen(big_bloom, word);
        end = clock();
        //printf("Zeit pro Pruefen BBF: %f\n", (double)(end - begin));
        zeitPruefenBBF += (double)(end - begin) / CLOCKS_PER_SEC;

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

    printf("Durchschnittliche Zeit pro Pruefen BF: \t\t%.10f\n", zeitPruefenBF/n);
    printf("Durchschnittliche Zeit pro Pruefen FF: \t\t%.10f\n", zeitPruefenFF/n);
    printf("Durchschnittliche Zeit pro Pruefen BBF: \t%.10f\n", zeitPruefenBBF/n);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BF

    //Aus den gemssenen Anzahlen kann eine experimentell ermittelte FPP berechnet werden
    double fpp_gemessen = (double)falschpositivBF / gesamt;

    printf(GRN "\nANALYSE:\n" RESET);

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

    //Input Datei schliessen
    fclose(file);
    //Bloom Filter freigeben
    freeBF(bloom);
    freeFF(floom);
    freeBF(big_bloom);

    return EXIT_SUCCESS;
}
