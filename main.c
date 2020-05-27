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


pthread_cond_t Occupation_Grande_Piste;
pthread_cond_t Occupation_Petite_Piste;

int nbGAvionsPrioritaires = 0;
int nbSAvionsPrioritaires = 0;
int nbGAvionsAttente = 0;
int nbSAvionsAttente = 0;
int avionIndex = 0;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

Avion avions[nbMaxAvion];
Piste pistes[nbPistes];

int accordAtterissage(Avion *avion){

    //Demande de l'accord d'atterissage

    //On lock le mutex
    pthread_mutex_lock(&mutex);

    //printf("Demande d'atterissage de l'Avion n° %d\n",avion->numero);
    //fflush(stdout);
    int numPiste;

    //Différenciation entre Grande et Petite piste

    if (avion->gabarits == Grand){
        //Si la piste n'est pas libre...
        if (!pistes[0].disponible ){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met le thread en attente
            pthread_cond_wait(&Occupation_Grande_Piste,&mutex);
        }
        //S'il y a de Grands avions prioritaires et que l'avion n'a pas de pb
        //printf("%d",nbGAvionsPrioritaires);
        if(nbGAvionsPrioritaires > 0 && avion->probTechnique == false && avion->kerosene != Urgent){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met le thread en attente
            pthread_cond_wait(&Occupation_Grande_Piste,&mutex);
        }
        //Sinon l'avion peut atterir
        //printf("Atterissage de l'avion n° %d sur la grande piste\n", avion->numero);
        printf("\033[%d;62HAtterissage\033[%d;95HGrande",avion->index+3,avion->index+3);
        fflush(stdout);
        //La piste est indisponible
        pistes[0].disponible = false;
        //displayPistes(pistes);
        numPiste = 1;
    }else{
        //Si l'avion est petit et qu'aucunes des pistes n'est disponible
        if (pistes[0].disponible == false && pistes[1].disponible == false) {
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente
            pthread_cond_wait(&Occupation_Petite_Piste, &mutex);
        }
        //S'il y a de petits avions prioritaires et que l'avion n'a pas de pb...
        //printf("%d",nbSAvionsPrioritaires);
        if (nbSAvionsPrioritaires > 0 && avion->probTechnique == false && avion->kerosene != Urgent){
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente
            printf("\033[%d;0H%d",nbMaxAvion+9+avion->index,nbSAvionsPrioritaires > 0 && (avion->probTechnique == false || avion->kerosene != Urgent));
            pthread_cond_wait(&Occupation_Petite_Piste, &mutex);
        }
        //Si la petite piste est libre
        if (pistes[1].disponible == true){
            //printf("Atterissage de l'avion n° %d sur la petite piste\n",avion->numero);
            printf("\033[%d;62HAtterissage\033[%d;95HPetite",avion->index+3,avion->index+3);
            fflush(stdout);
            //On place la petite piste en indisponible
            pistes[1].disponible = false;
            //displayPistes(pistes);
            numPiste = 2;
        }else{
            //Sinon on place la grande piste en indisponible
            //printf("Atterissage de l'avion n° %d sur la grande piste\n",avion->numero);
            printf("\033[%d;62HAtterissage\033[%d;95HGrande",avion->index+3,avion->index+3);
            fflush(stdout);
            pistes[0].disponible = false;
            //displayPistes(pistes);
            numPiste = 1;
        }
    }

    //On unlock le mutex
    pthread_mutex_unlock(&mutex);
    return numPiste;
}

void Decollage(Avion *avion,int numPiste){
    //On lock le mutex
    pthread_mutex_lock(&mutex);
    if (numPiste == 1){
        //Si la piste est la grande piste
        //printf("Décollage de l'avion n° %d sur grande piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HEn vol     \033[%d;82HArrivée\033[%d;95H                       \033[%d;120Hended              ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        //La piste est mise disponible et on envoie un signal pour réveiller le thread
        pistes[0].disponible = true;
        //displayPistes(pistes);
        if (avion->gabarits == Grand){
            pthread_cond_signal(&Occupation_Grande_Piste);
        }else{
            pthread_cond_signal(&Occupation_Petite_Piste);
        }
    }else{
        //Sinon la petite piste est mise disponible et on envoie un signal pour réveiller le thread
        //printf("Décollage de l'avion n° %d sur petite piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HEn vol     \033[%d;82HArrivée\033[%d;95H                        \033[%d;120Hended             ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        pistes[1].disponible = true;
        //displayPistes(pistes);
        pthread_cond_signal(&Occupation_Petite_Piste);
    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex);
}

void libererPiste(Avion *avion,int numPiste){
    pthread_mutex_lock(&mutex);
    if (numPiste == 1){
        //Si la piste est la grande piste
        //printf("Décollage de l'avion n° %d sur grande piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HA quai      \033[%d;82HDépart  \033[%d;95H                    \033[%d;120H                    ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        //La piste est mise disponible et on envoie un signal pour réveiller le thread
        pistes[0].disponible = true;
        //displayPistes(pistes);
        if (avion->gabarits == Grand){
            --nbGAvionsAttente;
            if(nbGAvionsAttente > 0){
                pthread_cond_signal(&Occupation_Grande_Piste);
            }
            pthread_cond_signal(&Occupation_Petite_Piste);
        }else{
            --nbSAvionsAttente;
            pthread_cond_signal(&Occupation_Petite_Piste);
        }

    }else{
        //Sinon la petite piste est mise disponible et on envoie un signal pour réveiller le thread
        //printf("Décollage de l'avion n° %d sur petite piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HA quai      \033[%d;82HDépart  \033[%d;95H                    \033[%d;120H                    ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        pistes[1].disponible = true;
        //displayPistes(pistes);
        --nbSAvionsAttente;
        pthread_cond_signal(&Occupation_Petite_Piste);
    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex);
}

void RamenerHangar(Avion *avion,int numPiste){
    //On lock le mutex
    pthread_mutex_lock(&mutex);
    // Si la piste est la grande piste
    if (numPiste == 1){
        if(avion->probTechnique){
            if(avion->gabarits == Grand){
                //Si l'avion a un pb technique on diminue le nombre de grands avions avec un pb
                --nbGAvionsPrioritaires;
                if(nbGAvionsAttente > 0){
                    pthread_cond_signal(&Occupation_Grande_Piste);
                }
                pthread_cond_signal(&Occupation_Petite_Piste);
            }else{
                --nbSAvionsPrioritaires;
            }
        }
        //printf("L'avion n° %d a été ramené au hangar, la grande piste est libre\n",avion->numero);
        printf("\033[%d;62HMise hangar\033[%d;82H           \033[%d;95H                    \033[%d;120Hended               ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        //On met a piste en disponible et on réveil le thread
        pistes[0].disponible = true;
        avion->status = ended;
        //displayPistes(pistes);
        if (avion->gabarits == Grand){
            --nbGAvionsAttente;
            pthread_cond_signal(&Occupation_Grande_Piste);
        }else{
            --nbSAvionsAttente;
            pthread_cond_signal(&Occupation_Petite_Piste);
        }

    }else{
        //Sinon si l'avion a un pb technique on diminue le nombre d'avion avec un pb de type standard
        if(avion->probTechnique){
            --nbSAvionsPrioritaires;
        }
        //printf("L'avion n° %d a été ramené au hangar, la grande piste est libre\n",avion->numero);
        printf("\033[%d;62HMise hangar\033[%d;82H            \033[%d;95H                   \033[%d;120Hended               ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        //On met a piste en disponible et on réveil le thread
        pistes[1].disponible = true;
        avion->status = ended;
        //displayPistes(pistes);
        --nbSAvionsAttente;
        pthread_cond_signal(&Occupation_Petite_Piste);

    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex);
}

int accordDecollage(Avion *avion){
    //On lock le mutex
    pthread_mutex_lock(&mutex);
    //printf("Demande de décollage de l'Avion n° %d\n",avion->numero);
    //fflush(stdout);
    int numPiste;

    //SI l'avion est grand
    if (avion->gabarits == Grand){

        //Si la piste n'est pas disponible ou qu'il y de grands avions priritaires
        if (!pistes[0].disponible){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met ke thread en attente
            pthread_cond_wait(&Occupation_Grande_Piste,&mutex);
        }
        if(nbGAvionsPrioritaires > 0 || nbGAvionsAttente > 0){
            pthread_cond_wait(&Occupation_Grande_Piste,&mutex);
        }
        //printf("Décollage de l'avion n° %d sur la grande piste\n", avion->numero);
        printf("\033[%d;62HDécollage\033[%d;82HDépart\033[%d;95HGrande",avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        //Sinon on met la piste en indisponible
        pistes[0].disponible = false;
        //displayPistes(pistes);
        numPiste = 1;
    }else{

        //Sinon si aucunes pistes sont disponibles ou que des avions standards ont un pb
        if (!pistes[0].disponible && !pistes[1].disponible) {
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente
            pthread_cond_wait(&Occupation_Petite_Piste, &mutex);
        }
        if(nbSAvionsPrioritaires > 0){
            pthread_cond_wait(&Occupation_Petite_Piste, &mutex);
        }
        if (pistes[1].disponible){
            //Si la petite piste est disponible
            //printf("Décollage de l'avion n° %d sur la petite piste\n",avion->numero);
            printf("\033[%d;62HDécollage\033[%d;82HDépart\033[%d;95HPetite",avion->index+3,avion->index+3,avion->index+3);
            fflush(stdout);
            //On la rend indisponible
            pistes[1].disponible = false;
            //displayPistes(pistes);
            numPiste = 2;
        }else{
            //Sinon on rend la grande piste indisponible
            //printf("Décollage de l'avion n° %d sur la grande piste\n",avion->numero);
            printf("\033[%d;62HDécollage\033[%d;82HDépart\033[%d;95HGrande",avion->index+3,avion->index+3,avion->index+3);
            fflush(stdout);
            pistes[0].disponible = false;
            //displayPistes(pistes);
            numPiste = 1;
        }
    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex);
    return numPiste;
}

void *threadAvion(void *args){
    //On crée un nouvel avion dans le thread
    Avion *avion;
    //On l'affecte au tableau d'avion initialisés dans le main
    avion = avions+avionIndex;
    //On l'affiche
    displayAvion(avion);
    ++avionIndex;

    //Si l'avion demande a atterir
    if (avion->typeDemande == demandeAtterissage){

        int distance = sqrt(pow(avion->Arrivee.longitude - avion->Depart.longitude,2) + pow(avion->Arrivee.lattitude - avion->Depart.lattitude,2));
        int temp = distance;
        int fuelLvl = 100;
        while(temp > 10){
            sleep(1);
            temp -=10;
            printf("\033[%d;135H%d        ",avion->index+3,temp);
            fuelLvl -= ((10*80)/distance);
            if(fuelLvl < 50 && fuelLvl > 20){
                avion->kerosene = Assure;
                printf("\033[%d;105HAssuré  ",avion->index+3);
            }else if(fuelLvl < 20) {
                avion->kerosene = Urgent;
                if (avion->gabarits == Grand) {
                    ++nbGAvionsPrioritaires;
                } else {
                    ++nbSAvionsPrioritaires;
                }
                printf("\033[%d;105HUrgent  ", avion->index + 3);
            }
        }
        if (avion->gabarits == Grand){
            ++nbGAvionsAttente;
        }else{
            ++nbSAvionsAttente;
        }

        //On lui accorde l'atterissage
        int numPiste = accordAtterissage(avion);
        //Si l'avion a finit sont trajet il retourne au hangar
        if(avion->typeArrivee == hangar){
            sleep(5);
            RamenerHangar(avion,numPiste);
        }else{
            sleep(5);
            libererPiste(avion,numPiste);
            sleep(10);
            numPiste = accordDecollage(avion);
            sleep(10);
            Decollage(avion,numPiste);
        }
    }else{
        //Sinon on lui accorde le décollage
        sleep(5);
        int numPiste = accordDecollage(avion);
        sleep(10);
        //Et il décolle
        Decollage(avion,numPiste);
    }
    sleep(2);

    return NULL;
};

void erreur (const char *msg)
{
    fprintf (stderr, "%s\n", msg);
}

void problemeMoteur(int num){
    int rndIndex;
    //On vérifie le type de signal ici Ctrl-Z
    if (SIGTSTP != num){
        erreur("Pb signal...");
    }
    //On affecte une valeur aléatoire pour un index d'avions
    do{
        rndIndex = rand()%nbMaxAvion;
        //On vérifie que l'avion demande bien un atterissage
    }while(avions[rndIndex].typeDemande != demandeAtterissage && avions[rndIndex].status != ended);
    //On affiche et modifie les valeurs
    avions[rndIndex].probTechnique = true;
    avions[rndIndex].typeArrivee = hangar;
    printf("\033[%d;120HPb Technique",avions[rndIndex].index+3);
    fflush(stdout);
    //Si l'avion est grand, on augmente le nombre de grands avions avec un pb
    if(avions[rndIndex].gabarits == Grand){
        ++nbGAvionsPrioritaires;
        //printf("%d",nbGAvionsPrioritaires);
    }else{
        //Sinon on augmente celui des petits avions
        ++nbSAvionsPrioritaires;
        //printf("%d",nbSAvionsPrioritaires);
    }
}

int main(){

    srand(time(NULL));

    //printf("\nBienvenue dans le système de gestion de fret de l'aéroport\n\n");
    //On init les pistes et les avions
    initPistes(pistes);
    initAvion(avions);

    setTerminal();
    //displayPistes(pistes);

    pthread_t avion[nbMaxAvion];

    //On init les conditions
    pthread_cond_init(&Occupation_Petite_Piste,0);
    pthread_cond_init(&Occupation_Grande_Piste,0);

    struct sigaction action;
    action.sa_handler = problemeMoteur;
    action.sa_flags = SA_RESTART;
    sigaction(SIGTSTP,&action,NULL);

    //On crée les threads
    for (int i = 0; i < nbMaxAvion; ++i) {
        pthread_create(avion+i, NULL, threadAvion, NULL);
    }

    for (int j = 0; j < nbMaxAvion; ++j) {
        pthread_join(avion[j],NULL);
    }

    return(0);
}
