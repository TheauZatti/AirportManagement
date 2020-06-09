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

   // printf("\033[%d;0HP1 = %d , P2 = %d \n",nbMaxAvion+5,pistes[0].disponible,pistes[1].disponible);
    //fflush(stdout);
}

void displayAvion(Avion *a){

    for (int i = 0; i < nbMaxAvion; ++i) {
//Affichage de l'avion
        printf("\033[%d;0H%d\033[%d;35H%s\033[%d;50H%s",a[i].index+3,a[i].numero,a[i].index+3,a[i].Depart.nom,a[i].index+3,a[i].Arrivee.nom);

        fflush(stdout);
        switch (a[i].gabarits){
            case Petit: printf("\033[%d;23HPetit",a[i].index+3);break;
            case Moyen: printf("\033[%d;23HMoyen",a[i].index+3);break;
            case Grand: printf("\033[%d;23HGrand",a[i].index+3);break;
            default: printf("\033[%d;23Hdef",a[i].index+3);
        }

        fflush(stdout);
        switch (a[i].typeDemande){
            case demandeAtterissage: printf("\033[%d;62HEn vol\033[%d;82HArrivée",a[i].index+3,a[i].index+3);break;
            case demandeDecollage: printf("\033[%d;62HHangar\033[%d;82HDépart",a[i].index+3,a[i].index+3);break;
            default: printf("\033[%d;62Hdef",a[i].index+3);
        }

        fflush(stdout);
        switch (a[i].kerosene){
            case Normal: printf("\033[%d;105HNormal",a[i].index+3);break;
            case Assure: printf("\033[%d;105HAssuré",a[i].index+3);break;
            case Urgent: printf("\033[%d;105HUrgent",a[i].index+3);break;
            default: printf("\033[%d;105Hdef",a[i].index+3);
        }

        fflush(stdout);
    }

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
    }else if(a->fuelLvl<20 && a->fuelLvl > 0){
        a->kerosene = Urgent;
        printf("\033[%d;105HUrgent",a->index+3);
    }else if(a->fuelLvl<=0){
        a->status = crashed;
        printf("\033[%d;105H            \033[%d;120Hcrashed            ",a->index+3,a->index+3);
    }
}

void getCloser(Avion *a,coordonnes *coords){
    int distance = a->tempDist;
    for (int i = 0; i < nbAeroports; ++i) {
        int temp;
        int angle;
        if(i != a->Arrivee.index && i != a->Depart.index){
            temp = sqrt(pow(coords[i].longitude - a->Depart.longitude,2) + pow(coords[i].lattitude - a->Depart.lattitude,2));
            if(temp < distance){
                distance = temp;
                angle = acos((((a->Arrivee.longitude-a->Depart.longitude)*(coords[i].longitude-a->Depart.longitude))+((a->Arrivee.lattitude-a->Depart.lattitude)*(coords[i].lattitude-a->Depart.lattitude)))/(sqrt(pow(a->Arrivee.longitude-a->Depart.longitude,2)+pow(a->Arrivee.lattitude-a->Depart.lattitude,2))*sqrt(pow(coords[i].longitude-a->Depart.longitude,2)+pow(coords[i].lattitude-a->Depart.lattitude,2))));
                a->Arrivee = coords[i];
                a->tempDist = (int)sqrt((int)pow(a->distance-a->tempDist,2)+(int)pow(distance,2)-(2*(a->distance-a->tempDist)*distance)*sin(angle));
            }
        }
    }
}