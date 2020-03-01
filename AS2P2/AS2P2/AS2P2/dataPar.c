//
//  dataPar.c
//  AS2P3
//
//  Created by Tong Zhang on 2020-02-28.
//  Copyright © 2020 zt. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 3

int philosophers[5] = {0, 1, 2, 3, 4};//5 philosophers
sem_t chopsticks[5];


sem_t mut;
pthread_mutex_t mutex;

void* thread1(){
    while(1){
        sem_wait(&mut);
        printf("I'm 1\n");
        sleep(1);
        sem_post(&mut);
        sleep(2);
    }
}

void* thread2(){
    while(1){
        sem_wait(&mut);
        printf("I'm 2\n");
        sleep(1);
        sem_post(&mut);
        sleep(2);
    }
}

void* thread3(){
    while(1){
        sem_wait(&mut);
        printf("I'm 3\n");
        sleep(1);
        sem_post(&mut);
        sleep(2);
    }
}

 
int main (void) {
    srand(time(NULL));
    pthread_t p_thread[SIZE];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&mut, 0, 1);
    
    for (int i=0; i<SIZE; i++) {
        sem_init(&chopsticks[i], 0, 1); // initiallization the semophore
    }
    
    //for (int i=0; i<SIZE; i++) {
        pthread_create(&p_thread[0], NULL, thread1, NULL);
    pthread_create(&p_thread[1], NULL, thread2, NULL);
    pthread_create(&p_thread[2], NULL, thread3, NULL);
    //}
    
    for (int i=0; i<SIZE; i++) {
        pthread_join(p_thread[i], NULL);
    }
    
    for (int i=0; i<SIZE; i++) {
        sem_destroy(&chopsticks[i]);
    }
 
    pthread_mutex_destroy(&mutex);
    sem_destroy(&mut);
    return 0;
}



