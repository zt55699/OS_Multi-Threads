//
//  diningOut.c
//  AS2P3
//
//  Created by Tong Zhang on 2020-02-29.
//  Copyright © 2020 zt. All rights reserved.
//
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define SIZE 5

int philosophers[5] = {0, 1, 2, 3, 4};//5 philosophers
sem_t chopsticks[5];
int chops_belong[5] = {-1,-1,-1,-1,-1};
int waiting[5] ={0};
int first[5] ={0};
int fullwar = 0;
//sem_t mut;
pthread_mutex_t mutex;

typedef struct {
    int arr[SIZE];
    int head;
    int tail;
} Queue;
Queue waitlist;

void queue_init(Queue *p_queue) {
    p_queue->head = 0;
    p_queue->tail = 0;
}

int queue_empty(const Queue *p_queue) {
    return !(p_queue->tail - p_queue->head);
}

int queue_full(const Queue *p_queue) {
    return p_queue->tail >= SIZE;
}

int queue_push(Queue *p_queue, int val) {
    if (queue_full(p_queue)) {
        return 0;
    }
    else {
        p_queue->arr[p_queue->tail] = val;
        p_queue->tail++;
        return 1;
    }
}


int queue_pop(Queue *p_queue) {
    if (queue_empty(p_queue)) {
        return -1;
    }
    else {
        int pop = p_queue->arr[p_queue->head];
        p_queue->head++;
        return pop;
    }
}

//get the front num (do not remove it from queue)
int queue_front(const Queue *p_queue) {
    if (queue_empty(p_queue)) {
        return -1;
    }
    else {
        return p_queue->arr[p_queue->head];
    }
}

void queue_remove(Queue *p_queue, int num){
    int i;
    for(i=0; i<(SIZE-1);i++){
        if(p_queue->arr[i]==num){
            if(i==0){
                p_queue->head++;
            }
            else if(i==4){
                p_queue->tail--;
            }
            else{
                int j = i;
                for(j=i; j<(p_queue->tail-2); j++){
                    p_queue->arr[j]= p_queue->arr[j+1];
                }
                p_queue->tail--;
            }
            break;
        }
    }
}

int queue_finding(Queue *p_queue, int num){
    int i;
    for(i=0; i<p_queue->tail;i++){
        if(p_queue->arr[i]==num){
            return 1;
        }
    }
    return 0;
}

size_t time_ms() {
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == -1) {
    perror("gettimeofday");
    exit(2);
  }
  
  // Convert timeval values to milliseconds
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}


void dining(int philo){
    queue_remove(&waitlist, philo);
    waiting[philo]=0;
    first[philo]=0;
    int left = philo;
    int right = (philo + 4) % 5;//id of right chopstick=(id_philosopher+4)%5.
    sem_wait(&chopsticks[left]);
    sem_wait(&chopsticks[right]);
    
    //printf("       %d  unlock\n", philo);
    pthread_mutex_unlock(&mutex);
    
    sleep(rand()%4+3);
    printf("philosopher[%d] done dining and put down the chopstics\n\n", philo);
    chops_belong[left] = -1;
    chops_belong[right] = -1;
    sem_post(&chopsticks[left]);
    sem_post(&chopsticks[right]);
}

void waiter (int philo){
    if(chops_belong[0]!=chops_belong[1]&&chops_belong[0]!=chops_belong[2]&& chops_belong[0]!=chops_belong[3]&&chops_belong[0]!=chops_belong[4]&&chops_belong[1]!=chops_belong[2]&&chops_belong[1]!=chops_belong[3]&&chops_belong[1]!=chops_belong[4]&&chops_belong[2]!=chops_belong[3]&&chops_belong[2]!=chops_belong[4]&&chops_belong[3]!=chops_belong[4]){
        if(fullwar==0){
            fullwar = 1;
            printf("4 philosophers are holding 1 chopstics, 5th cannot, back wait\n");
        }
        return;
    }
    fullwar = 0;
    int left = philo;
    int right = (philo + 4) % 5;//id of right chopstick=(id_philosopher+4)%5.
    int sval_l, sval_r;
    if(philo != queue_front(&waitlist)){
        sleep(1);
    }
    pthread_mutex_lock(&mutex);//lock
    //printf("       %d lock\n", philo);
    sem_getvalue(&chopsticks[left], &sval_l);
    sem_getvalue(&chopsticks[right], &sval_r);
    if(sval_l <1 ||sval_r <1 ){
        if(sval_l <1 &&sval_r <1 ){
            //printf("philosopher %d doesn't has chopstics to pick up\n", philo);
      
        }
        else if(sval_l ==1){
            chops_belong[left] = philo;
            printf("The waiter gives philosopher[%d] left chopstic\n", philo);
            sem_wait(&chopsticks[left]);
 
        }
        else if(sval_r ==1){
            chops_belong[right] = philo;
            printf("The waiter gives philosopher[%d] right chopstic\n", philo);
            sem_wait(&chopsticks[right]);
        }
        //printf("NO%d  LEFT belong to NO%d； RIGHT belong to NO%d\n", philo, chops_belong[left], chops_belong[right]);
        if(chops_belong[left] == philo && chops_belong[right] == philo){
            printf("Philosopher[%d] now has 2 chopstics, begin dining\n", philo);
            sem_post(&chopsticks[left]);
            sem_post(&chopsticks[right]);
            dining(philo);
            return;
        }
        //printf("Philosopher %d does't have enough chopstics to eat, waitlist\n", philo);
        queue_push(&waitlist,philo);
        waiting[philo]=1;
        pthread_mutex_unlock(&mutex);
        return;
    }
    else{
        chops_belong[left] = philo;
        chops_belong[right] = philo;
        printf("The waiter gives philosopher[%d] two chopstics, begin dining\n", philo);
        dining(philo);
    }
    
}
 
void *philosopher (void* param) {
    
    int i = *(int *)param;  //turn param to a int pointer, then get the integer it points
    int printcount =0;
    size_t loop_start = time_ms();
    while (1) {
        if((time_ms()-loop_start)>8000){
            printf("           DEAD LOCK!! \n");
            exit(0);
        }
        int temp, sval;
        int all_picked = 0;
        for (temp =0; temp<4; temp++){
            sem_getvalue(&chopsticks[temp], &sval);
            if(sval>0){
                all_picked = 1;
                break;
            }
        }
        if(all_picked==0){
            if(printcount==0){
                printf("The waiter told philo[%d] that all chopsticks are being hold, waiting someone done\n", i);
                queue_push(&waitlist,i);
                printcount =1;
            }
            continue;
        }
        if(waiting[i]==1 && first[i]==0){
            printf("philosopher[%d] is waiting \n", i);
            first[i]=1;
            //sem_wait(&mut);
        }
        else if(waiting[i]!=1){
            printf("philosopher[%d] is thinking \n", i);
            sleep(rand()%6+2);
            printf("philosopher[%d] is hungry, send request to the waiter \n", i);
        }
        //send request to waiter
        waiter(i);
        printcount =0;
        loop_start = time_ms();
    }
}
 
int main (void) {
    srand(time(NULL));
    pthread_t philos_thread[5];
    
    queue_init(&waitlist);
    pthread_mutex_init(&mutex, NULL);
    //sem_init(&mutex, 0, 1);
    
    for (int i=0; i<5; i++) {
        sem_init(&chopsticks[i], 0, 1); // initiallization the semophore
    }
    
    for (int i=0; i<5; i++) {
        pthread_create(&philos_thread[i], NULL, philosopher, &philosophers[i]);
    }
    
    for (int i=0; i<5; i++) {
        pthread_join(philos_thread[i], NULL);
    }
    
    for (int i=0; i<5; i++) {
        sem_destroy(&chopsticks[i]);
    }
 
    pthread_mutex_destroy(&mutex);
 
    return 0;
}



