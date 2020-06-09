//
// Created by theau on 07/05/2020.
//

#ifndef PROJET_STRUCTURES_H
#define PROJET_STRUCTURES_H

#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>

typedef struct c{
    char* nom;
    int lattitude;
    int longitude;
    int index;
}coordonnes;

typedef struct a{
    int kerosene;
    bool probTechnique;
    int gabarits;
    int numero;
    int typeDemande;
    int typeArrivee;
    coordonnes Depart;
    coordonnes Arrivee;
    int index;
    int status;
    int distance;
    int fuelLvl;
    bool incremented;
    int tempDist;
}Avion;

typedef struct p{
    int taille;
    bool disponible;
}Piste;




#endif //PROJET_STRUCTURES_H
