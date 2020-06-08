//
// Created by theau on 12/05/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include "fonctions.h"

void initAvion(Avion *a,coordonnes coords[]){
    //Initialisation des avions
    int depart,arrivee;
    for (int i = 0; i < nbMaxAvion; ++i) {
        a[i].kerosene = Normal;
        a[i].probTechnique = false;
        a[i].numero = rand()%10000;
        a[i].index = i;
        a[i].incremented = false;
        a[i].status = inprogress;
        //Choix aléatoire du gabarits de l'avion

        int x = rand()%3;
        switch (x){
            case 0: a[i].gabarits = Petit;break;
            case 1: a[i].gabarits = Moyen;break;
            case 2: a[i].gabarits = Grand;break;
        }

        //Choix aléatoire du type de demande

        int y = rand()%2;
        switch (y){
            case 0: a[i].typeDemande = demandeAtterissage;
                    break;
            case 1: a[i].typeDemande = demandeDecollage;
                    break;
        }
        depart = rand()%nbAeroports;
        a[i].Depart = coords[depart];
        do{
            arrivee = rand()%nbAeroports;
        }while(arrivee == depart);
        a[i].Arrivee = coords[arrivee];

        //Choix aléatoire du type d'arrivée

        int j = rand()%2;
        if (y == 0){
            switch (j){
                case 0: a[i].typeArrivee = hangar;break;
                case 1: a[i].typeArrivee = decollage;
                        break;
            }
        }
        a[i].distance = sqrt(pow(a[i].Arrivee.longitude - a[i].Depart.longitude,2) + pow(a[i].Arrivee.lattitude - a[i].Depart.lattitude,2));
        a[i].fuelLvl = 100;
    }
}

void initPistes(Piste pistes[][nbAeroports]){

    //Initialisation des pistes
    for (int i = 0; i < nbAeroports; ++i) {
        pistes[0][i].taille = 4000;
        pistes[0][i].disponible = true;
        pistes[1][i].taille = 2500;
        pistes[1][i].disponible = true;
    }
}