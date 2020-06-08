//
// Created by theau on 07/05/2020.
//

#ifndef PROJET_FONCTIONS_H
#define PROJET_FONCTIONS_H

#include "structures.h"
#include "constantes.h"

void initAvion(Avion*, coordonnes[]);
void initPistes(Piste[][nbAeroports]);
void displayPistes(Piste*);
void displayAvion(Avion*);
void setTerminal();
void computeFuel(Avion*);

#endif //PROJET_FONCTIONS_H
