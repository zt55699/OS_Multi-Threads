//
//  dataPar.c
//  AS2P2
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
#include <string.h>
#include <sys/malloc.h>
#include <math.h>

#define SIZE 3652


typedef struct{
    int x;
    int y;
}point;
point points[SIZE];






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

void* fitting(){
    while(1){
        
    }
}

void read_csv(char* file){
    FILE *fp = NULL;
    char *line,*record;
    char buffer[40];//20450这个数组大小也要根据自己文件的列数进行相应修改。

    if((fp = fopen(file, "r")) != NULL)
    {
        fseek(fp, 0, SEEK_SET);  //定位到第二行，每个英文字符大小为1，16425L这个参数根据自己文件的列数进行相应修改。

        while ((line = fgets(buffer, sizeof(buffer), fp))!=NULL)//当没有读取到文件末尾时循环继续
        {
            record = strtok(line, ",");
            while (record != NULL)//读取每一行的数据
            {
                printf("%s ", record);//将读取到的每一个数据打印出来
                record = strtok(NULL, ",");
            }
        }
        fclose(fp);
        fp = NULL;
    }
}
 
int main (void) {
    char* file = "stremflow_time_series.csv";
    read_csv(file);
    
    
    /*
    srand(time(NULL));
    pthread_t p_thread[SIZE];

    pthread_mutex_init(&mutex, NULL);
    sem_init(&mut, 0, 1);
    
    for (int i=0; i<num_thread; i++) {
        sem_init(&chopsticks[i], 0, 1); // initiallization the semophore
    }
    
    //for (int i=0; i<num_thread; i++) {
    pthread_create(&p_thread[0], NULL, thread1, NULL);
    pthread_create(&p_thread[1], NULL, thread2, NULL);
    pthread_create(&p_thread[2], NULL, thread3, NULL);
    //}
    
    for (int i=0; i<num_thread; i++) {
        pthread_join(p_thread[i], NULL);
    }
    
    for (int i=0; i<SIZE; i++) {
        sem_destroy(&chopsticks[i]);
    }
 
    pthread_mutex_destroy(&mutex);
    sem_destroy(&mut);
     
     */
    return 0;
}



