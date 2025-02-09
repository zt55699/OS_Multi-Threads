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
//#include <sys/malloc.h>
#include <math.h>

#define SIZE 3653
#define COMBINATION 6666726
size_t timer_start;
int R_SIZE = 3652;

typedef struct{
    int x;
    int y;
}point;
point points[SIZE];

typedef struct{
    int p1;
    int p2;
    double sum;
    double x1;
    double x2;
}combination;
combination sums[COMBINATION];

sem_t mut;
pthread_mutex_t mutex;

size_t time_ms() {
  struct timeval tv;
  if(gettimeofday(&tv, NULL) == -1) {
    perror("gettimeofday");
    exit(2);
  }
  
  // Convert timeval values to milliseconds
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}

void print_points(){
    int i;
    for(i=0; i<SIZE; i++){
        printf("(%d,%d)\n",points[i].x, points[i].y);
    }
}

int count_com = 0;
void building_process(){
    if(count_com==100000) printf("Building... 2%%\n");
    if(count_com==500000) printf("Building... 10%%\n");
    if(count_com==1000000) printf("Building... 20%%\n");
    if(count_com==2000000) printf("Building... 40%%\n");
    if(count_com==4000000) printf("Building... 70%%\n");
    if(count_com==6000000) printf("Building... 95%%\n");
}

typedef unsigned long marker;
marker one = 1;

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
		building_process();
		
        return;
    }
    /* if we choose the current item, "or" (|) the bit to mark it so. */
    build_pairs(pool, need - 1, chosen | (one << at), at + 1);
    build_pairs(pool, need, chosen, at + 1);  /* or don't choose it, go to next */
}

/*
void cal_sum(point p1, point p2){
    float x1, x2;
    x2 = 1.0*(p2.y-p1.y)/(p2.x-p1.x);
    x1 = 1.0*p1.y - 1.0*x2*p1.x;
    
}*/


//calculate the sum of absolute residuals of the line of a pair of points
//SAR(a1,a2) = Σwi│di – (a1 + a2ti)│, for all i = 1,2,…,m,
void cal_sum(combination *pair){
    pair->x2 = (1.0*(points[pair->p2].y)-1.0*(points[pair->p1].y))/1.0*(points[pair->p2].x-1.0*points[pair->p1].x);
    pair->x1 = 1.0*(points[pair->p1].y) - 1.0*pair->x2*(points[pair->p1].x);
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
    pair->sum = sum;
//    printf("pair%d-%d sum:%f\n", pair->p1, pair->p2, pair->sum);
}

void cal_slopint(combination pair){
    printf("with slop: %f   y_intercept: %f\n", pair.x2, pair.x1);
}

void find_min(){
    double minSAR = INFINITY;
    int min_pair = -1;
    int i;
    for(i=0; i<count_com; i++){
        if(sums[i].sum<minSAR){
            minSAR = sums[i].sum;
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
    printf("%d data read from %s\n",R_SIZE,file);
    if((fp = fopen(file, "r")) != NULL)
    {
        fseek(fp, 0, SEEK_SET);  //定位到第二行，每个英文字符大小为1，16425L这个参数根据自己文件的列数进行相应修改。
        int xcordi=1;
        line = fgets(buffer, sizeof(buffer), fp);
        while ((line = fgets(buffer, sizeof(buffer), fp))!=NULL)
        {
            int colum =1;
            record = strtok(line, ",");
            while (record != NULL)//读取每一行的数据
            {
                if(colum==2)
                    points[xcordi].y = atoi(record)*1.0;
                else
                    points[xcordi].x = xcordi*1.0;
                //printf("%s ", record);//将读取到的每一个数据打印出来
                record = strtok(NULL, ",");
                colum++;
            }
            xcordi++;
        }
        fclose(fp);
        fp = NULL;
    }
}

void print_progress(int);

void cal_all_sum(){
    int i;
    for(i=0;i<count_com;i++){
        cal_sum(&sums[i]);
        print_progress(i);
    }
}

void print_progress(int cal_progress){
    if(cal_progress== 1)
        printf("progress:1%% \n");
    else if(cal_progress== count_com/20)
        printf("progress:5%% \n");
    else if(cal_progress== count_com/10)
        printf("progress:10%% \n");
    else if(cal_progress== count_com/5)
        printf("progress:20%% \n");
    else if(cal_progress== count_com/2)
        printf("progress:50%% \n");
    else if(cal_progress== (count_com-2))
        printf("progress:99.99%% \n");
}
 
int main (void) {
	printf("\n***[This is Single-threads]***\n\n");
	timer_start = time_ms();
    char* file = "stremflow_time_series.csv";
    //char* file = "test1_2002.csv";
    read_csv(file);
    //print_points();
    build_pairs(R_SIZE+1,2,0,1);
    //build_pairs(3653,2,0,1);
    
//    print_comb();
    printf("Building finish! total lines: %d\n\n", count_com);
    printf("SAR Calculation begin... \n");
    cal_all_sum();
    find_min();

	size_t runtime = time_ms() - timer_start;
    int second = runtime/1000;
    printf("Runing time: %ds %lums\n",second, runtime%1000);
    return 0;
}

