#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

// Defina aca las variables globales y funciones auxiliares que necesite
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int parking[10] = {1,1,1,1,1,1,1,1,1,1};
int ticket = 0, turn = 0;

int k_available(int k){
    int actual = 0;
    for(int i = 0; i<10; i++){
        if(actual >= k)
            return i - actual;
        if(parking[i])
            actual++;
        else{
            actual = 0;
        }
    }
    if(actual >= k)
        return 10 - actual;
    return -1;
}



void initReservar() {

}

void cleanReservar() {

}

int reservar(int k) {
    pthread_mutex_lock(&m);

    int my_ticket = ticket++;

    while(my_ticket!=turn || k_available(k)==-1)
        pthread_cond_wait(&c, &m);

    int place = k_available(k);

    for(int i = 0; i < k; i++)
        parking[i + place] = 0;

    turn++;

    pthread_mutex_unlock(&m);
    
    return place;
}

void liberar(int e, int k) {
    int *p = parking;
    p+=e;
    for(int i = 0; i < k; i++){
        *p = 1;
        p++;
    }
    pthread_cond_broadcast(&c);
} 