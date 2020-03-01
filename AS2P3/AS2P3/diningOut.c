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
        return 1;//表示将数字加进去了
    }
}


int queue_pop(Queue *p_queue, int *p_num) {
    if (queue_empty(p_queue)) {
        return 0;
    }
    else {
        *p_num = p_queue->arr[p_queue->head];//因为要删除，所以先给
        p_queue->head++;//将取过的数跳过去
        return 1;
    }
}

//get the front num (do not remove it from queue)
int queue_front(const Queue *p_queue, int *p_num) {
    if (queue_empty(p_queue)) {
        return 0;
    }
    else {
        *p_num = p_queue->arr[p_queue->head];//多次调用是同一个数
        return 1;
    }
}

void dining(int philo){
    int left = philo;//左筷子的编号和哲学家的编号相同
    int right = (philo + 4) % 5;//右筷子的编号为哲学家编号+4%5
    sem_wait(&chopsticks[left]);
    sem_wait(&chopsticks[right]);
    
    printf("       %d号 拿起2支筷子开始进食 unlock\n", philo);
    pthread_mutex_unlock(&mutex);
    
    sleep(rand()%4+3);
    printf("       %d号 进餐结束放下2支筷子\n", philo);
    chops_belong[left] = -1;
    chops_belong[right] = -1;
    sem_post(&chopsticks[left]);
    sem_post(&chopsticks[right]);
}

void waiter (int philo){
    int left = philo;//左筷子的编号和哲学家的编号相同
    int right = (philo + 4) % 5;//右筷子的编号为哲学家编号+4%5
    int sval_l, sval_r;
    pthread_mutex_lock(&mutex);//加锁
    printf("       %d号 lock\n", philo);
    sem_getvalue(&chopsticks[left], &sval_l);
    sem_getvalue(&chopsticks[right], &sval_r);
    if(sval_l <1 ||sval_r <1 ){
        if(sval_l <1 &&sval_r <1 ){
            printf("  %d号 does't has chopstics to pick up\n", philo);
        }
        else if(sval_l ==1){
            chops_belong[left] = philo;
            printf("  %d号 pick up left\n", philo);
            sem_wait(&chopsticks[left]);
            //queue_push(&waitlist,philo);
        }
        else{
            chops_belong[right] = philo;
            printf("  %d号 pick up right\n", philo);
            sem_wait(&chopsticks[right]);
            //queue_push(&waitlist,philo);
        }
        printf("  左 belong to %d号； 右 belong to %d号\n", chops_belong[left], chops_belong[right]);
        if(chops_belong[left] == philo && chops_belong[right] == philo){
            dining(philo);
        }
        printf("       %d号 不能进食进入等待列表 unlock\n", philo);
        pthread_mutex_unlock(&mutex);
        return;
    }
    else{
        dining(philo);
    }
    
}
 
void *philosopher (void* param) {
    
    int i = *(int *)param;  //turn param to a int pointer, then get the integer it points
   /* int left = i;//左筷子的编号和哲学家的编号相同
    int right = (i + 4) % 5;//右筷子的编号为哲学家编号+4%5
    */
    int printcount =0;
    while (1) {
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
                printf("所有筷子都在使用 等待中\n");
                printcount =1;
            }
            continue;
        }
        printf("哲学家%d正在思考问题\n", i);
        sleep(rand()%3+1);
                
        printf("哲学家%d饿了\n", i);
        //send request to waiter
        waiter(i);
        /*
        pthread_mutex_lock(&mutex);//加锁
            printf("       %d号 lock\n", i);
            sem_getvalue(&chopsticks[left], &sval_l);
            sem_getvalue(&chopsticks[right], &sval_r);
            printf("       %d号 check sem_wait left=%d right=%d\n", i,sval_l, sval_r);
        sem_wait(&chopsticks[left]);//此时这哲学家左筷的信号量-1之后>=0时，表示能继续。
        printf("哲学家%d拿起了%d号筷子,现在只有一支筷子,不能进餐\n", i, left);
        sem_wait(&chopsticks[right]);
        printf("哲学家%d拿起了%d号筷子\n", i, right);
            printf("       %d号 unlock\n", i);
        pthread_mutex_unlock(&mutex);//解锁
         
        printf("哲学家%d现在有两支筷子,开始进餐\n", i);
        sleep(rand()%3+1);
        sem_post(&chopsticks[left]);
        printf("哲学家%d放下了%d号筷子\n", i, left);
        sem_post(&chopsticks[right]);
        printf("哲学家%d放下了%d号筷子\n", i, right);
         
         */
    }
}
 
int main (void) {
    srand(time(NULL));
    pthread_t philos_thread[5];
    
    queue_init(&waitlist);
    pthread_mutex_init(&mutex, NULL);
    
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



