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
#include "fonctions.h"

coordonnes coords[10] = {{"Paris",4800,200},{"Marseille",4300,500},{"Nice",4300,700},{"Lyon",4500,500},{"Toulouse",4300,100},{"Bordeaux",4400,-100},{"Strasbourg",4800,700},{"Brest",4800,-400},{"Metz",4900,600},{"La Rochelle",4600,-100}};


void initAvion(Avion *a){
    //Initialisation des avions
    int depart,arrivee;
    for (int i = 0; i < nbMaxAvion; ++i) {
        a[i].kerosene = Normal;
        a[i].probTechnique = false;
        a[i].numero = rand()%10000;
        a[i].index = i;
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
                    do{
                        depart = rand()%10;
                    }while(depart == 0);
                    a[i].Depart = coords[depart];
                    a[i].Arrivee = coords[Paris];
                    break;
            case 1: a[i].typeDemande = demandeDecollage;
                    a[i].Depart = coords[Paris];
                    do{
                        arrivee = rand()%10;
                    }while(arrivee == 0);
                    a[i].Arrivee = coords[arrivee];
                    break;
        }

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

void initPistes(Piste *pistes){

    //Initialisation des pistes

    pistes[0].taille = 4000;
    pistes[0].disponible = true;
    pistes[1].taille = 2500;
    pistes[1].disponible = true;
}