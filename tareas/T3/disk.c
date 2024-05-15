#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

/*****************************************************
 * Agregue aca los tipos, variables globales u otras
 * funciones que necesite
 *****************************************************/
typedef struct {
    int ready;
    pthread_cond_t w;
    int k;
} Request;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

int busy = 0;
int pista = 0;
PriQueue *q;
PriQueue *p;
PriQueue *a;

void iniDisk(void) {
    q = makePriQueue();
    p = makePriQueue();
}

void cleanDisk(void) {
    destroyPriQueue(q);
    destroyPriQueue(p);
    destroyPriQueue(a);
    pthread_mutex_destroy(&m);
}

void requestDisk(int track) {
    pthread_mutex_lock(&m);
    // int i = priLength(q);
    // int dist = abs(pista - track);
    if (!busy) {
        busy = 1;
        pista = track;
    } else {
        Request req = {0, PTHREAD_COND_INITIALIZER, track};
        if ((track >= pista)) {    
            priPut(q, &req, track);
        } else {
            priPut(p, &req, track);
        }
        //priPut(q, &req, track);
        while (!req.ready) {
            pthread_cond_wait(&req.w, &m);
        }
    }
    pthread_mutex_unlock(&m);
}

void releaseDisk() {
    pthread_mutex_lock(&m);
    if (emptyPriQueue(q)) {
        a = p;
        p = q;
        q = a;
        if (emptyPriQueue(q)) {
            busy = 0;
        } else {
            Request *preq = priGet(q);
            preq -> ready = 1;
            pista = preq -> k;
            pthread_cond_signal(&preq -> w);
        }
    } else {
        Request *preq = priGet(q);
        preq -> ready = 1;
        pista = preq -> k;
        pthread_cond_signal(&preq -> w);
    }       
    pthread_mutex_unlock(&m);
}
