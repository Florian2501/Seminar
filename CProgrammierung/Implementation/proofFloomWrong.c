#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


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

void compare(int n, double FPP, int bereiche, FILE* einfuegen_file)
{

    fprintf(einfuegen_file, "%d;%f;%d;", n, FPP, bereiche);
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

    //Sicherstellen, dass alle Werte im BBF 0 sind


    /////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            break;
        }
        //Wort in Bloom-Filter einfuegen
        //Da der uebrige Platz im letzten evtl. angerissenen Byte mitgenutzt werden soll, wird nicht die
        //genaue Anzahl Bit angegeben, die durch die Formel errechnet wurde, sondern die auf volle Byte 
        //aufgerundete Anzahl Bit, da der Platz im evtl. angerissenen Byte sonst verloren ginge

        //Zeitmessungsschnipsel stammt von Thomas Pornin von Stackoverflow
        //https://stackoverflow.com/questions/5248915/execution-time-of-c-program

        einfuegen(bloom, word);
        //printf("Zeit pro Einfuegen BF: %.10f\n", (double)(end - begin));

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
    double fpp_gemessen = (double)falschpositivBF / gesamt;

    fprintf(einfuegen_file, "%.15f;", fpp_gemessen);

    /*//Prozentuale Abweichung berechnen
    double abweichung = ((fpp_gemessen / FPP) - 1) * 100.0;
    fprintf(einfuegen, "%f;", abweichung);
    if(abweichung > 0)
    {
        printf(RED);
    }
    else if(abweichung < 0)
    {
        printf(GRN);
    }
    printf("\t%.2f %%\n" RESET, abweichung);*/

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //FF

    //Ermittelte FPP Rate berechnen
    double fpp_gemessenFF = (double)falschpositivFF / gesamt;
    
    fprintf(einfuegen_file, "%.15f;", fpp_gemessenFF);

    //Prozentuale Abweichung berechnen
    /*double abweichungFF = ((fpp_gemessenFF / FPP) - 1) * 100.0;
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
    printf("\t%.2f %%\n" RESET, abweichung-abweichungFF);*/

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //BBF

    //Ermittelte FPP Rate berechnen
    double fpp_gemessenBBF = (double)falschpositivBBF / gesamt;
    
    fprintf(einfuegen_file, "%.15f\n", fpp_gemessenBBF);

    /*//Prozentuale Abweichung berechnen
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
    printf("\t%.2f %%\n" RESET, abweichung-abweichungBBF);*/

    //Input Datei schliessen
    fclose(file);
    //Bloom Filter freigeben
    freeBF(bloom);
    freeFF(floom);
    freeBF(big_bloom);

    return;
}



int main(){

    FILE* einfuegen = fopen("floom.csv", "w");

    int n_arr[10] = {500, 1000, 10000, 20000, 30000, 50000, 100000, 200000, 300000, 500000};
    double FPP_arr[5] = { 0.005, 0.01, 0.02, 0.03, 0.05};
    int bereiche_ar[6] = {8, 10, 12, 14, 16, 20};

    for(int b = 0; b<6; b++){
        for(int f = 0; f< 5; f++){
            for (int k = 0; k< 10; k++){
                compare(n_arr[k], FPP_arr[f], bereiche_ar[b], einfuegen);
                fflush(einfuegen);
            }
        }
    }

        
    


    fclose(einfuegen);
}