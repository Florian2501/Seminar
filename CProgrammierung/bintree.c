#include "stdio.h"
#include "stdlib.h"


typedef struct {
    int wert;
    struct Knoten* links;
    struct Knoten* rechts;
} Knoten;

int KnotenHinzufügen(Knoten* wurzel, int wert){
    if(wurzel->wert > wert){
        if(wurzel->links = NULL){
            Knoten* neuerKnoten = malloc(sizeof(Knoten));
            neuerKnoten->links = NULL;
            neuerKnoten->rechts = NULL;
            neuerKnoten->wert = wert;
            return 1;
        }
        else KnotenHinzufügen(wurzel->links, wert);
    }
    if(wurzel->wert > wert){
        if(wurzel->links = NULL){
            Knoten* neuerKnoten = malloc(sizeof(Knoten));
            neuerKnoten->links = NULL;
            neuerKnoten->rechts = NULL;
            neuerKnoten->wert = wert;
            return 1;
        }
        else KnotenHinzufügen(wurzel->links, wert);
    }
}

int main(){
    Knoten* wurzel = malloc(sizeof(Knoten));
    wurzel->links = NULL;
    wurzel->rechts = NULL;
    
    printf("Hallo, geben Sie den Wert des Wurzelknotens ein:");
    scanf("%d", wurzel->wert);

    printf("Der Baum wurde mit dem Wert %d in der Wrzel initialisiert.", wurzel->wert);

    return 0;
}