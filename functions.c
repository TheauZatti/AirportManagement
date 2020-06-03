//
// Created by theau on 12/05/2020.
//
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "fonctions.h"

void displayPistes(Piste *pistes){

    //Affichage des pistes

    printf("\033[%d;0HP1 = %d , P2 = %d \n",nbMaxAvion+5,pistes[0].disponible,pistes[1].disponible);
    fflush(stdout);
}

void displayAvion(Avion *a){

    //Affichage de l'avion

    printf("\033[%d;0H%d\033[%d;35H%s\033[%d;50H%s\n",a->index+3,a->numero,a->index+3,a->Depart.nom,a->index+3,a->Arrivee.nom);

    switch (a->gabarits){
        case Petit: printf("\033[%d;23HPetit",a->index+3);break;
        case Moyen: printf("\033[%d;23HMoyen",a->index+3);break;
        case Grand: printf("\033[%d;23HGrand",a->index+3);break;
    }

    switch (a->typeDemande){
        case demandeAtterissage: printf("\033[%d;62HEn vol\033[%d;82HArrivée",a->index+3,a->index+3);break;
        case demandeDecollage: printf("\033[%d;62HHangar\033[%d;82HDépart",a->index+3,a->index+3);break;
    }

    switch (a->kerosene){
        case Normal: printf("\033[%d;105HNormal",a->index+3);break;
        case Assure: printf("\033[%d;105HAssuré",a->index+3);break;
        case Urgent: printf("\033[%d;105HUrgent",a->index+3);break;
    }

    fflush(stdout);
}

void setTerminal(){
    printf("\033c");
    printf("\033[0;0HNuméro de l'avion\033[0;23HGabarit\033[0;35HProvenance\033[0;50HArrivée\033[0;62HEtat de l'avion\033[0;82HType\033[0;95HPiste\033[0;105HKerozène\n\n");

}

void computeFuel(Avion *a){
    a->fuelLvl -= ((10*80)/a->distance);
    if(a->fuelLvl<50 && a->fuelLvl>20){
        a->kerosene = Assure;
        printf("\033[%d;105HAssuré",a->index+3);
    }else if(a->fuelLvl<20){
        a->kerosene = Urgent;
        printf("\033[%d;105HUrgent",a->index+3);
    }else if(a->fuelLvl <= 0){
        a->status = crashed;
        printf("\033[%d;105H            \033[%d;120Hcrashed            ",a->index+3,a->index+3);
    }
}