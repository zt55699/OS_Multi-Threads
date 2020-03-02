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
#include <math.h>

#define SIZE 3653 //actual size = SIZE-1;
#define COMBINATION 6666726
int R_SIZE = 365;
sem_t mutex;

int cal_progress=0;

typedef struct{
    int x;
    int y;
}point;
point points[SIZE];

typedef struct{
    int p1;
    int p2;
    double SAR;
    double x1;
    double x2;
}pair;
pair sums[COMBINATION];


void print_points(){
    int i;
    for(i=0; i<SIZE; i++){
        printf("(%d,%d)\n",points[i].x, points[i].y);
    }
}

typedef unsigned long marker;
marker one = 1;
int count_com = 0;
void build_pairs(int pool, int need, marker chosen, int at)
{
    if (pool < need + at) return; /* not enough bits left */
 
    if (!need) {
        /* got all we needed; print the thing.  if other actions are
         * desired, we could have passed in a callback function. */
        int dig=0;
        for (at = 0; at < pool; at++){
            if (chosen & (one << at)){
                if(dig==0){
                    sums[count_com].p1= at;
                    //printf("p1 = %d ", at);
                    dig =1;
                }
                else if(dig==1){
                    sums[count_com].p2= at;
                    //printf("p2 = %d ", at);
                    dig =0;
                }
                //printf("%d ", at);
            }
            
        }
        //printf("\n");
        count_com++;
        if(count_com==100000) printf("count reach 100000\n");
        if(count_com==500000) printf("count reach 500000\n");
        if(count_com==1000000) printf("count reach 1000000\n");
        if(count_com==2000000) printf("count reach 2000000\n");
        if(count_com==3000000) printf("count reach 3000000\n");
        if(count_com==4000000) printf("count reach 4000000\n");
        if(count_com==5000000) printf("count reach 5000000\n");
        if(count_com==6000000) printf("count reach 6000000\n");
        return;
    }
    /* if we choose the current item, "or" (|) the bit to mark it so. */
    build_pairs(pool, need - 1, chosen | (one << at), at + 1);
    build_pairs(pool, need, chosen, at + 1);  /* or don't choose it, go to next */
}

//calculate the sum of absolute residuals of the line of a pair of points
//SAR(a1,a2) = Σwi│di – (a1 + a2ti)│, for all i = 1,2,…,m,
void cal_sum(pair *pair){
    pair->x2 = (1.0*(points[pair->p2].y)-1.0*(points[pair->p1].y))/(1.0*points[pair->p2].x-1.0*points[pair->p1].x);
    pair->x1 = (1.0*points[pair->p1].y) - pair->x2*(1.0*(points[pair->p1].x));
    double sum = 0.0;
    int i;
    int count_resi = 0;
//    printf("Line %d-%d\n", pair->p1, pair->p2);
    for(i=1;i<R_SIZE+1;i++){
        //if(i!=(pair->p1) && i!=(pair->p2)){
            double absResidual =fabs(1.0*points[i].y-(pair->x1+pair->x2*(1.0*i)));
//            printf(" p%d 's residual is %f\n", i, absResidual);
            count_resi++;
            sum+= absResidual;
        //}
    }
//    printf("numof resi= %d\n", count_resi);
    pair->SAR = sum;
//    printf("pair%d-%d sum:%f\n", pair->p1, pair->p2, pair->SAR);
}

void cal_slopint(pair pair){
    printf("with slop: %f   y_intercept: %f\n", pair.x2, pair.x1);
}

void find_min(){
    double minSAR = INFINITY;
    int min_pair = -1;
    int i;
    for(i=0; i<count_com; i++){
        if(sums[i].SAR<minSAR){
            minSAR = sums[i].SAR;
            min_pair = i;
        }
    }
    printf("the best L1 line: (%d,%d)and(%d,%d)  SAR: %f\n", points[sums[min_pair].p1].x,points[sums[min_pair].p1].y, points[sums[min_pair].p2].x,points[sums[min_pair].p2].y,minSAR);
    
    cal_slopint(sums[min_pair]);
    
}

void print_comb(){
    int i;
    int count_print=0;
    for(i=0; i<count_com; i++){
        printf("(%d,%d)and(%d,%d)\n",sums[i].p1,points[sums[i].p1].y, points[sums[i].p2].x, points[sums[i].p2].y);
        count_print++;
    }
    printf("pairs: %d\n", count_print);
}



void read_csv(char* file){
    
    FILE *fp = NULL;
    char *line,*record;
    char buffer[40];
    
    if((fp = fopen(file, "r")) != NULL){
        fseek(fp, 0, SEEK_SET);
        int xcordi=1;
        line = fgets(buffer, sizeof(buffer), fp);
        while ((line = fgets(buffer, sizeof(buffer), fp))!=NULL){
            int colum =1;
            record = strtok(line, ",");
            while (record != NULL){
                if(colum==2)
                    points[xcordi].y = atoi(record);
                else
                    points[xcordi].x = xcordi;
 
                record = strtok(NULL, ",");
                colum++;
            }
            xcordi++;
        }
        fclose(fp);
        fp = NULL;
    }
}

void print_progress(){
    if(cal_progress== 1)
        printf("progress:1%% \n");
    if(cal_progress== count_com/20)
        printf("progress:5%% \n");
    if(cal_progress== count_com/10)
        printf("progress:10%% \n");
    if(cal_progress== count_com/5)
        printf("progress:20%% \n");
    if(cal_progress== count_com/2)
        printf("progress:50%% \n");
    if(cal_progress== (count_com-2))
        printf("progress:99.99%% \n");
}

void* cal_all_sum(){
    while(1){
        if(cal_progress>count_com-1){
            printf("thread Exit!\n\n");
            return NULL;
        }
        sem_wait(&mutex);
        print_progress();
        int i = cal_progress;
        cal_progress++;
        sem_post(&mutex);
        cal_sum(&sums[i]);
    }
    return NULL;
}


 
int main (void) {
    printf("\n***[This is Multi-threads]***\n\n");
    //char* file = "stremflow_time_series.csv";
    char* file = "test1_2002.csv";
    read_csv(file);
    //print_points();
    build_pairs(R_SIZE+1,2,0,1);
    //build_pairs(3653,2,0,1);
    //    print_comb();
    printf("total com: %d\n", count_com);
    
    pthread_t cal_thread[3];
    int val[3];
    
    sem_init(&mutex, 0, 1);
    /*
    for (int i=0; i<5; i++) {
        sem_init(&threads[i], 0, 1); // initiallization the semophore
    }
       */
    for (int i=0; i<3; i++) {
        pthread_create(&cal_thread[i], NULL, cal_all_sum, &val[i]);
    }
       
    for (int i=0; i<3; i++) {
        pthread_join(cal_thread[i], NULL);
    }
       

    sem_destroy(&mutex);
    
    //cal_all_sum();
    find_min();

    return 0;
}

