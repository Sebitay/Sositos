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
        if(parking[i]){
            actual++;
            if(actual >= k)
                return i - actual + 1;
        }else{
            actual = 0;
        }
    }
    return -1;
}



void initReservar() {
    for(int i = 0; i < 10; i++)
        parking[i] = 1;
    ticket = 0, turn = 0;
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

    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
    return place;
}

void liberar(int e, int k) {
    pthread_mutex_lock(&m);

    for(int i = 0; i < k; i++)
        parking[i + e] = 1;

    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
} 