//
// Created by theau on 12/05/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#include "fonctions.h"


pthread_cond_t Occupation_Grande_Piste[nbAeroports];
pthread_cond_t Occupation_Petite_Piste[nbAeroports];

int nbGAvionsPrioritaires[nbAeroports] = {0,0,0,0,0,0,0,0,0,0};
int nbSAvionsPrioritaires[nbAeroports] = {0,0,0,0,0,0,0,0,0,0};
int nbGAvionsAttente[nbAeroports] = {0,0,0,0,0,0,0,0,0,0};
int nbGAvionsDecollage[nbAeroports] = {0,0,0,0,0,0,0,0,0,0};
int nbSAvionsAttente[nbAeroports] = {0,0,0,0,0,0,0,0,0,0};

coordonnes coords[nbAeroports] = {{"Paris",4800,200,0},{"Marseille",4300,500,1},{"Nice",4300,700,2},{"Lyon",4500,500,3},{"Toulouse",4300,100,4},{"Bordeaux",4400,-100,5},{"Strasbourg",4800,700,6},{"Brest",4800,-400,7},{"Metz",4900,600,8},{"La Rochelle",4600,-100,9}};

int avionIndex = 0;

pthread_mutex_t mutex[nbAeroports] = PTHREAD_MUTEX_INITIALIZER;

Avion avions[nbMaxAvion];
Piste pistes[nbPistes][nbAeroports];

int accordAtterissage(Avion *avion){

    //Demande de l'accord d'atterissage

    //On lock le mutex
    pthread_mutex_lock(&mutex[avion->Arrivee.index]);

    //printf("Demande d'atterissage de l'Avion n° %d\n",avion->numero);
    //fflush(stdout);
    int numPiste;
    int res = 0;
    struct timespec ts;

    //Différenciation entre Grande et Petite piste
    //printf("%d;%d;%d;%d",nbGAvionsPrioritaires[avion->Arrivee.index],nbSAvionsPrioritaires[avion->Arrivee.index],nbGAvionsAttente[avion->Arrivee.index],nbSAvionsAttente[avion->Arrivee.index]);
    if (avion->gabarits == Grand){
        //Si la piste n'est pas libre...
        if (!pistes[0][avion->Arrivee.index].disponible ){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met le thread en attente avec une durée
            do {
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                res = pthread_cond_timedwait(&Occupation_Grande_Piste[avion->Arrivee.index], &mutex[avion->Arrivee.index], &ts);
                //On calcule le niveau de kérozène
                computeFuel(avion);
                //Si l'avion est en niveau urgent ou a un pb technique et que la piste est libre on le sort de la boucle
            } while (res == ETIMEDOUT && (avion->kerosene != Urgent || !avion->probTechnique) && !pistes[0][avion->Arrivee.index].disponible);
            //On incremente le nb d'avions prioritaires
            if(avion->kerosene == Urgent && !avion->incremented){
                ++nbGAvionsPrioritaires[avion->Arrivee.index];
                avion->incremented = true;
            }
        }
        //S'il y a de Grands avions prioritaires et que l'avion n'a pas de pb
        //printf("%d",nbGAvionsPrioritaires);
        if(nbGAvionsPrioritaires[avion->Arrivee.index] > 0 && avion->probTechnique == false && avion->kerosene != Urgent){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met le thread en attente avec une durée
            do {
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                res = pthread_cond_timedwait(&Occupation_Grande_Piste[avion->Arrivee.index], &mutex[avion->Arrivee.index], &ts);
                //On calcule le niveau de kérozène
                computeFuel(avion);
                //Si l'avion s'est écrasé ou qu'une piste est libre on le sort
            } while (res == ETIMEDOUT && avion->status != crashed && !pistes[0][avion->Arrivee.index].disponible);
            //Si l'avion s'est écrasé on le décremente et on libère le mutex
            if(avion->status == crashed){
                --nbGAvionsPrioritaires[avion->Arrivee.index];
                --nbGAvionsAttente[avion->Arrivee.index];
                pthread_mutex_unlock(&mutex[avion->Arrivee.index]);
                return 10;
            }
        }
        //Sinon l'avion peut atterir
        //printf("Atterissage de l'avion n° %d sur la grande piste\n", avion->numero);
        printf("\033[%d;62HAtterissage\033[%d;95HGrande",avion->index+3,avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //La piste est indisponible
        pistes[0][avion->Arrivee.index].disponible = false;
        //displayPistes(pistes);
        numPiste = 1;
    }else{
        //Si l'avion est petit et qu'aucunes des pistes n'est disponible
        if ((pistes[0][avion->Arrivee.index].disponible == false || nbGAvionsPrioritaires[avion->Arrivee.index] > 0 ) && pistes[1][avion->Arrivee.index].disponible == false) {
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente avec une durée
            do {
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                res = pthread_cond_timedwait(&Occupation_Petite_Piste[avion->Arrivee.index], &mutex[avion->Arrivee.index], &ts);
                //On calcule le niveau de kérozène
                computeFuel(avion);
                //Si l'avion est en niveau urgent ou a un pb technique et que la piste est libre on le sort de la boucle
            } while (res == ETIMEDOUT && (avion->kerosene != Urgent || !avion->probTechnique) && (!pistes[0][avion->Arrivee.index].disponible || !pistes[1][avion->Arrivee.index].disponible));
            //On incremente le nb d'avions prioritaires
            if(avion->kerosene == Urgent && !avion->incremented){
                ++nbSAvionsPrioritaires[avion->Arrivee.index];
                avion->incremented = true;
            }
        }
        //S'il y a de petits avions prioritaires et que l'avion n'a pas de pb...
        //printf("%d",nbSAvionsPrioritaires);
        if (nbSAvionsPrioritaires[avion->Arrivee.index] > 0 && avion->probTechnique == false && avion->kerosene != Urgent){
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente
            do {
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 1;
                res = pthread_cond_timedwait(&Occupation_Petite_Piste[avion->Arrivee.index], &mutex[avion->Arrivee.index], &ts);
                //On calcule le niveau de kérozène
                computeFuel(avion);
                //Si l'avion s'est écrasé ou qu'une piste est libre on le sort
            } while (res == ETIMEDOUT && avion->status != crashed && (!pistes[0][avion->Arrivee.index].disponible || !pistes[1][avion->Arrivee.index].disponible));
            //Si l'avion s'est écrasé on le décremente et on libère le mutex
            if(avion->status == crashed){
                --nbSAvionsPrioritaires[avion->Arrivee.index];
                --nbSAvionsAttente[avion->Arrivee.index];
                pthread_mutex_unlock(&mutex[avion->Arrivee.index]);
                return 10;
            }
        }
        //Si la petite piste est libre
        if (pistes[1][avion->Arrivee.index].disponible == true){
            //printf("Atterissage de l'avion n° %d sur la petite piste\n",avion->numero);
            printf("\033[%d;62HAtterissage\033[%d;95HPetite",avion->index+3,avion->index+3);
            printf("\033[%d;5H",nbMaxAvion+5);
            fflush(stdout);
            //On place la petite piste en indisponible
            pistes[1][avion->Arrivee.index].disponible = false;
            //displayPistes(pistes);
            numPiste = 2;
        }else{
            //Sinon on place la grande piste en indisponible
            //printf("Atterissage de l'avion n° %d sur la grande piste\n",avion->numero);
            printf("\033[%d;62HAtterissage\033[%d;95HGrande",avion->index+3,avion->index+3);
            printf("\033[%d;5H",nbMaxAvion+5);
            fflush(stdout);
            pistes[0][avion->Arrivee.index].disponible = false;
            //displayPistes(pistes);
            numPiste = 1;
        }
    }

    //On unlock le mutex
    pthread_mutex_unlock(&mutex[avion->Arrivee.index]);
    return numPiste;
}

void Decollage(Avion *avion,int numPiste){
    //On lock le mutex
    pthread_mutex_lock(&mutex[avion->Depart.index]);
    if (numPiste == 1){
        //Si la piste est la grande piste
        //printf("Décollage de l'avion n° %d sur grande piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HEn vol     \033[%d;82HArrivée\033[%d;95H          ",avion->index+3,avion->index+3,avion->index+3);
        printf("\033[%d;120Hended               ",avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //La piste est mise disponible et on envoie un signal pour réveiller le thread
        pistes[0][avion->Depart.index].disponible = true;
        if(avion->gabarits == Grand){
            --nbGAvionsDecollage[avion->Depart.index];
        }
        //displayPistes(pistes);
        if (nbGAvionsDecollage[avion->Depart.index] > 0 || nbSAvionsPrioritaires[avion->Arrivee.index] > 0){
             pthread_cond_signal(&Occupation_Grande_Piste[avion->Depart.index]);
        }else{
            pthread_cond_signal(&Occupation_Petite_Piste[avion->Depart.index]);
        }
    }else{
        //Sinon la petite piste est mise disponible et on envoie un signal pour réveiller le thread
        //printf("Décollage de l'avion n° %d sur petite piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HEn vol     \033[%d;82HArrivée\033[%d;95H        ",avion->index+3,avion->index+3,avion->index+3);
        printf("\033[%d;120Hended               ",avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        pistes[1][avion->Depart.index].disponible = true;
        //displayPistes(pistes);
        pthread_cond_signal(&Occupation_Petite_Piste[avion->Depart.index]);
    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex[avion->Depart.index]);
}

void libererPiste(Avion *avion,int numPiste){
    pthread_mutex_lock(&mutex[avion->Arrivee.index]);
    if (numPiste == 1){
        //Si la piste est la grande piste
        //printf("Décollage de l'avion n° %d sur grande piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HA quai      \033[%d;82HDépart  \033[%d;95H                    \033[%d;120H                    ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //La piste est mise disponible et on envoie un signal pour réveiller le thread
        pistes[0][avion->Arrivee.index].disponible = true;
        //displayPistes(pistes);
        if (avion->gabarits == Grand){
            //Si l'avion était en urgence on décremente
            if(avion->kerosene == Urgent){
                --nbGAvionsPrioritaires[avion->Arrivee.index];
            }
            --nbGAvionsAttente[avion->Arrivee.index];
            //Si le nombre d'avion en attente est >0 ou qu'il y'a des avions prioritaires on reveille la grande piste
            if(nbGAvionsAttente[avion->Arrivee.index] > 0 || nbGAvionsPrioritaires[avion->Arrivee.index] > 0 || nbSAvionsPrioritaires[avion->Arrivee.index] > 0){
                pthread_cond_signal(&Occupation_Grande_Piste[avion->Arrivee.index]);
            }else{
                pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);
            }
        }else{
            if(avion->kerosene == Urgent){
                --nbSAvionsPrioritaires[avion->Arrivee.index];
            }
            --nbSAvionsAttente[avion->Arrivee.index];
            pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);
        }

    }else{
        //Sinon la petite piste est mise disponible et on envoie un signal pour réveiller le thread
        //printf("Décollage de l'avion n° %d sur petite piste effectué, piste libre\n",avion->numero);
        printf("\033[%d;62HA quai      \033[%d;82HDépart  \033[%d;95H                    \033[%d;120H                    ",avion->index+3,avion->index+3,avion->index+3,avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        pistes[1][avion->Arrivee.index].disponible = true;
        //displayPistes(pistes);
        --nbSAvionsAttente[avion->Arrivee.index];
        pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);
    }
    //On unlock le mutex
    //printf("%d;%d;%d;%d",nbGAvionsPrioritaires[avion->Arrivee.index],nbSAvionsPrioritaires[avion->Arrivee.index],nbGAvionsAttente[avion->Arrivee.index],nbSAvionsAttente[avion->Arrivee.index]);
    pthread_mutex_unlock(&mutex[avion->Arrivee.index]);
}

void RamenerHangar(Avion *avion,int numPiste){
    //On lock le mutex
    pthread_mutex_lock(&mutex[avion->Arrivee.index]);
    // Si la piste est la grande piste
    if (numPiste == 1){
        if(avion->probTechnique || avion->kerosene == Urgent){
            if(avion->gabarits == Grand){
                //Si l'avion a un pb technique on diminue le nombre de grands avions avec un pb
                --nbGAvionsPrioritaires[avion->Arrivee.index];
            }else{
                --nbSAvionsPrioritaires[avion->Arrivee.index];
            }
        }
        //printf("L'avion n° %d a été ramené au hangar, la grande piste est libre\n",avion->numero);
        printf("\033[%d;62HMise hangar\033[%d;82H           \033[%d;95H                    ",avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        printf("\033[%d;120Hended               ",avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //On met a piste en disponible et on réveil le thread
        pistes[0][avion->Arrivee.index].disponible = true;
        //displayPistes(pistes);
        if (avion->gabarits == Grand){
            --nbGAvionsAttente[avion->Arrivee.index];
            if(nbGAvionsAttente[avion->Arrivee.index] > 0 || nbGAvionsPrioritaires[avion->Arrivee.index] > 0 || nbSAvionsPrioritaires[avion->Arrivee.index] > 0){
                pthread_cond_signal(&Occupation_Grande_Piste[avion->Arrivee.index]);
            }else{
                pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);
            }
        }else{
            --nbSAvionsAttente[avion->Arrivee.index];
            pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);
        }
    }else{
        //Sinon si l'avion a un pb technique on diminue le nombre d'avion avec un pb de type standard
        if(avion->probTechnique || avion->kerosene == Urgent){
            --nbSAvionsPrioritaires[avion->Arrivee.index];
        }
        //printf("L'avion n° %d a été ramené au hangar, la grande piste est libre\n",avion->numero);
        printf("\033[%d;62HMise hangar\033[%d;82H            \033[%d;95H                   ",avion->index+3,avion->index+3,avion->index+3);
        fflush(stdout);
        printf("\033[%d;120Hended               ",avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //On met a piste en disponible et on réveil le thread
        pistes[1][avion->Arrivee.index].disponible = true;
        //displayPistes(pistes);
        --nbSAvionsAttente[avion->Arrivee.index];
        pthread_cond_signal(&Occupation_Petite_Piste[avion->Arrivee.index]);

    }
    //On unlock le mutex
    //printf("%d;%d;%d;%d",nbGAvionsPrioritaires[avion->Arrivee.index],nbSAvionsPrioritaires[avion->Arrivee.index],nbGAvionsAttente[avion->Arrivee.index],nbSAvionsAttente[avion->Arrivee.index]);
    pthread_mutex_unlock(&mutex[avion->Arrivee.index]);
}

int accordDecollage(Avion *avion){
    //On lock le mutex
    pthread_mutex_lock(&mutex[avion->Depart.index]);
    //printf("Demande de décollage de l'Avion n° %d\n",avion->numero);
    //fflush(stdout);
    int numPiste;

    //SI l'avion est grand
    if (avion->gabarits == Grand){

        //Si la piste n'est pas disponible ou qu'il y de grands avions priritaires
        if (!pistes[0][avion->Depart.index].disponible){
            //printf("Grande piste indisponible\n");
            //fflush(stdout);
            //On met ke thread en attente
            pthread_cond_wait(&Occupation_Grande_Piste[avion->Depart.index],&mutex[avion->Depart.index]);
        }
        if(nbGAvionsPrioritaires[avion->Depart.index] > 0 || nbGAvionsAttente[avion->Depart.index] > 0){
            pthread_cond_wait(&Occupation_Grande_Piste[avion->Depart.index],&mutex[avion->Depart.index]);
        }
        //printf("Décollage de l'avion n° %d sur la grande piste\n", avion->numero);
        printf("\033[%d;62HDécollage\033[%d;82HDépart \033[%d;95HGrande",avion->index+3,avion->index+3,avion->index+3);
        printf("\033[%d;5H",nbMaxAvion+5);
        fflush(stdout);
        //Sinon on met la piste en indisponible
        pistes[0][avion->Depart.index].disponible = false;
        //displayPistes(pistes);
        numPiste = 1;
    }else{

        //Sinon si aucunes pistes sont disponibles ou que des avions standards ont un pb
        if ((!pistes[0][avion->Depart.index].disponible || nbGAvionsDecollage[avion->Depart.index] > 0) && !pistes[1][avion->Depart.index].disponible) {
            //printf("Aucunes pistes disponibles pour l'avion n° %d\n", avion->numero);
            //fflush(stdout);
            //On met le thread en attente
            pthread_cond_wait(&Occupation_Petite_Piste[avion->Depart.index], &mutex[avion->Depart.index]);
        }
        if(nbSAvionsPrioritaires[avion->Depart.index] > 0 ){
            pthread_cond_wait(&Occupation_Petite_Piste[avion->Depart.index], &mutex[avion->Depart.index]);
        }
        if (pistes[1][avion->Depart.index].disponible){
            //Si la petite piste est disponible
            //printf("Décollage de l'avion n° %d sur la petite piste\n",avion->numero);
            printf("\033[%d;62HDécollage\033[%d;82HDépart \033[%d;95HPetite",avion->index+3,avion->index+3,avion->index+3);
            printf("\033[%d;5H",nbMaxAvion+5);
            fflush(stdout);
            //On la rend indisponible
            pistes[1][avion->Depart.index].disponible = false;
            //displayPistes(pistes);
            numPiste = 2;
        }else{
            //Sinon on rend la grande piste indisponible
            //printf("Décollage de l'avion n° %d sur la grande piste\n",avion->numero);
            printf("\033[%d;62HDécollage\033[%d;82HDépart \033[%d;95HGrande",avion->index+3,avion->index+3,avion->index+3);
            printf("\033[%d;5H",nbMaxAvion+5);
            fflush(stdout);
            pistes[0][avion->Depart.index].disponible = false;
            //displayPistes(pistes);
            numPiste = 1;
        }
    }
    //On unlock le mutex
    pthread_mutex_unlock(&mutex[avion->Depart.index]);
    return numPiste;
}

void *threadAvion(void *args){
    //On crée un nouvel avion dans le thread
    Avion *avion;
    //On l'affecte au tableau d'avion initialisés dans le main
    avion = avions+avionIndex;
    //On l'affiche
    ++avionIndex;
    //Si l'avion demande a atterir
    if (avion->typeDemande == demandeAtterissage){
        avion->tempDist = avion->distance;
        //On calcule la distance qui reste à parcourir
        while(avion->tempDist > 10 && avion->status != crashed){
            sleep(1);
            avion->tempDist -=10;

            printf("\033[%d;62HEn Vol      ",avion->index+3);
            printf("\033[%d;135H%d   ",avion->index+3,avion->tempDist);
            printf("\033[%d;5H",nbMaxAvion+5);
            fflush(stdout);
            //On calcule le niveau de kérozène
            computeFuel(avion);
        }
        if(avion->status == crashed){
            return NULL;
        }
        //Si l'avion est en urgence on incrémente les avions prioritaires
        if(avion->kerosene == Urgent && avion->gabarits == Grand){
            ++nbGAvionsPrioritaires[avion->Arrivee.index];
            avion->incremented = true;
        }else if(avion->kerosene == Urgent && (avion->gabarits == Moyen || avion->gabarits == Petit)){
            ++nbSAvionsPrioritaires[avion->Arrivee.index];
            avion->incremented = true;
        }
        if(avion->gabarits == Grand){
            ++nbGAvionsAttente[avion->Arrivee.index];
        }else{
            ++nbSAvionsAttente[avion->Arrivee.index];
        }

        //On lui accorde l'atterissage
        int numPiste = accordAtterissage(avion);
        sleep(5);
        //Si l'avion a finit sont trajet il retourne au hangar
        if(numPiste != 10) {
            //Suivant e type d'arrivée on le met au hangar ou il redécolle
            if (avion->typeArrivee == hangar) {
                RamenerHangar(avion, numPiste);
            } else {
                if (avion->gabarits == Grand) {
                    ++nbGAvionsDecollage[avion->Depart.index];
                }
                libererPiste(avion, numPiste);
                //On affecte la nouvelle destination
                avion->Depart = coords[avion->Arrivee.index];
                int villeArrivee = rand()%(nbAeroports-1);
                if(villeArrivee == avion->Arrivee.index){
                    ++villeArrivee;
                }
                avion->Arrivee = coords[villeArrivee];
                printf("\033[%d;35H%s          \033[%d;50H%s         \033[%d;62HA quai      ",avion->index+3,avion->Depart.nom,avion->index+3,avion->Arrivee.nom,avion->index+3);
                printf("\033[%d;5H",nbMaxAvion+5);
                sleep(10);
                numPiste = accordDecollage(avion);
                sleep(10);
                Decollage(avion, numPiste);
            }
        }
    }else{
        //Sinon on lui accorde le décollage
        if (avion->gabarits == Grand){
            ++nbGAvionsDecollage[avion->Depart.index];
        }
        sleep(5);
        int numPiste = accordDecollage(avion);
        sleep(10);
        //Et il décolle
        Decollage(avion,numPiste);
    }
    avion->status = ended;
    sleep(2);

    for (int i = 0; i < nbAeroports; ++i) {
        printf("\033[%d;5H%d;%d;%d;%d",27+i,nbGAvionsPrioritaires[i],nbSAvionsPrioritaires[i],nbGAvionsAttente[i],nbSAvionsAttente[i]);
    }

    return NULL;
}

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
    }while(avions[rndIndex].typeDemande == demandeDecollage || (avions[rndIndex].status == ended || avions[rndIndex].status == crashed) || avions[rndIndex].probTechnique);
    //On affiche et modifie les valeurs
    avions[rndIndex].probTechnique = true;
    //Si l'avion a un pb technique, l'avion change de cap si possible
    getCloser(&avions[rndIndex],coords);
    printf("\033[%d;35H%s          \033[%d;50H%s      ",avions[rndIndex].index+3,avions[rndIndex].Depart.nom,avions[rndIndex].index+3,avions[rndIndex].Arrivee.nom);
    avions[rndIndex].typeArrivee = hangar;
    printf("\033[%d;120HPb Technique",avions[rndIndex].index+3);
    fflush(stdout);
    //Si l'avion est grand, on augmente le nombre de grands avions avec un pb
    if(avions[rndIndex].gabarits == Grand && !avions[rndIndex].incremented){
        ++nbGAvionsPrioritaires[avions[rndIndex].Arrivee.index];
        //printf("%d",nbGAvionsPrioritaires);
    }else if(!avions[rndIndex].incremented){
        //Sinon on augmente celui des petits avions
        ++nbSAvionsPrioritaires[avions[rndIndex].Arrivee.index];
        //printf("%d",nbSAvionsPrioritaires);
    }
    avions[rndIndex].incremented = true;
    printf("\033[%d;5H",nbMaxAvion+5);
}

int main(){

    srand(time(NULL));

    //printf("\nBienvenue dans le système de gestion de fret de l'aéroport\n\n");
    //On init les pistes et les avions
    initPistes(pistes);
    initAvion(avions,coords);
    setTerminal();
    sleep(2);
    //displayPistes(pistes);
    pthread_t avion[nbMaxAvion];

    //On init les conditions
    for (int k = 0; k < nbAeroports; ++k) {
        pthread_cond_init(&Occupation_Petite_Piste[k],0);
        pthread_cond_init(&Occupation_Grande_Piste[k],0);
    }

    displayAvion(avions);
    printf("\033[%d;5H",nbMaxAvion+5);
    fflush(stdout);

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
