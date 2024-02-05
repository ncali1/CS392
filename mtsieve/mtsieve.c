/*******************************************************************************
* Name        : mtsieve.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/28/2021
* Description : sieve to find numbers that 2 or more digits contain '3'
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>
#include <wait.h>
#include <getopt.h>
#include <float.h>
#include <sys/sysinfo.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

int total_count = 0; 
pthread_mutex_t lock;

typedef struct arg_struct {
    int start;     
    int end; 
    } thread_args; 
bool is_int(char *in) {
    if(strlen(in)>0){
        if(in[0]=='-'){
            if(strlen(in)>1){
                for(int x=1;x<strlen(in);x++){
                    if(!isdigit(in[x])){
                        return false;
                    }
                }
                return true;
            }
            else{
                return false;
            }
        }
        else{
            for(int x=0;x<strlen(in);x++){
                    if(!isdigit(in[x])){
                        return false;
                    }
                }
            return true;
        }
    }
    return false;
}

bool is_overflow(char *in){
    long x;
    if (sscanf(in, "%ld", &x) != 1) {
        return true;
    }
    int y= (int)x;
    if(x!=(long)y){
        return true;
    }
    return false;
}
bool second(int x){
    int temp=0;
    while(x>0){
        temp=x%10;
        x=x/10;
        if(temp==3){
            return true;
        }
    }
    return false;
}
bool threes(int x){
    int temp=0;
    while(x>0){
        temp=x%10;
        x=x/10;
        if(temp==3&&second(x)){
            return true;
        }
    }
    return false;
}

void *sieve(void *ptr){
    thread_args* targs = (thread_args *)ptr;
    int counter = 0;
    int s = targs->start;
    int e = targs->end;
    int square=(int)sqrtf(e);
    bool *lows= (bool *) malloc(sizeof(bool)*(square+1));
    for(int x=0;x<=square;x++){
        lows[x]=true;
    }
    for(int x=2;x*x<square;x++){
        if(lows[x]){
            for(int y=x*x;y<=square;y+=x){
                lows[y]=false;
            }
        }
    }
    int length=e-s+1;
    bool *highs= (bool *) malloc(sizeof(bool)*length+1);
    for(int x=0;x<=length;x++){
        highs[x]=true;
    }
    for(int x=2;x<square;x++){
        if(lows[x]){
            int y= (int)ceil((double)s/x)*x-s;
            if(s<=x){
                y+=x;
            }
            while(y<length){
                highs[y]=false;
                y+=x;
            }
        }
    }
    for(int x=2;x<=length;x++){
        if(highs[x]){
            if(x+s>100&&threes(x+s)){
                counter++;
            }
        }
    }
    int retval;
    if ((retval = pthread_mutex_lock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
    }
    total_count += counter;
    if ((retval = pthread_mutex_unlock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));
    }
    free(lows);
    free(highs);
    pthread_exit(NULL);
}

void usage(){
    printf("Usage: ./mtsieve -s <starting value> -e <ending value> -t <num threads> \n");
}

bool input_checker(bool sF,bool eF,bool tF, int sI, int eI, int tI, char *args){
    if (args != NULL){
        printf("Error: Non-option argument '%s' supplied.\n", args);
        return false;
    }
    if (!sF){
        printf("Error: Required argument <starting value> is missing.\n");
        return false;
    }
    if (sI < 2){
        printf("Error: Starting value must be >= 2.\n");
        return false;
    }
    if (!eF){
        printf("Error: Required argument <ending value> is missing.\n");
        return false;
    }
    if (eI < 2){
        printf("Error: Ending value must be >= 2.\n");
        return false;
    }
    if (eI < sI){
        printf("Error: Ending value must be >= starting value.\n");
        return false;
    }
    if (!tF){
        printf("Error: Required argument <num threads> is missing.\n");
        return false;
    }
    if (tI < 1){
        printf("Error: Number of threads cannot be less than 1.\n");
        return false;
    }
    if (tI > (2*get_nprocs())){
        printf("Error: Number of threads cannot exceed twice the number of processors(%d).\n", get_nprocs());
        return false;
    }
    return true;
}

bool int_checker(char *type, char* number){
    if(number==NULL){
        printf("Error: Option -%s requires an argument.\n", type);
        return false;
    }
    if(!is_int(number)){
        printf("Error: Invalid input '%s' received for parameter '-%s'.\n", number, type);
        return false;
    }
    if(is_overflow(number)){
        printf("Error: Integer overflow for parameter '-%s'.\n", type);
        return false;
    }
    return true;
}
int main(int argc, char *argv[]) {
    if(argc==1){
        usage();
        return EXIT_FAILURE;
    }
    bool sF, eF, tF = false;
    int sI, eI, tI, numOfArgs = 0;
    int opt;
    while ((opt = getopt(argc, argv, ":s:e:t:")) != -1) {
        switch(opt){
            case 's':
                if(!int_checker("s",optarg)){
                    return EXIT_FAILURE;
                }
                sF=true;
                sI=atoi(optarg);
                numOfArgs=optind;
                break;
            case 'e':
                if(!int_checker("e",optarg)){
                    return EXIT_FAILURE;
                }
                eF=true;
                eI=atoi(optarg);
                numOfArgs=optind;
                break;
            case 't':
                if(!int_checker("t",optarg)){
                    return EXIT_FAILURE;
                }
                tF=true;
                tI=atoi(optarg);
                numOfArgs=optind;
                break;
            case '?':
                if (optopt == 'e' || optopt == 's' || optopt == 't') {
                    fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return EXIT_FAILURE;
            case ':':
                if (optopt == 'e' || optopt == 's' || optopt == 't') {
                    fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return EXIT_FAILURE;
        }
    }
    if(!input_checker(sF, eF, tF, sI, eI, tI, argv[numOfArgs])){
        return false;
    }
    int threads =tI;
    int dif=eI-sI+1;
    int extra=0;
    if(threads>dif){
        threads=dif;
    }else{
        extra=dif%threads;
        dif=dif/threads;
    }
    printf("Finding all prime numbers between %d and %d.\n", sI, eI);
    int starter=sI;
    int ender=0;
    int retval;
    if((retval=pthread_mutex_init(&lock, NULL)) != 0){
        fprintf(stderr,"Error: Cannot create mutex. %s. \n",strerror(retval));
        return EXIT_FAILURE;
    }
    pthread_t *runners=(pthread_t *)malloc(threads*sizeof(pthread_t));
    thread_args *passable=(thread_args *)malloc(threads*sizeof(thread_args));
    for (int i=0;i<threads;i++) {
        passable[i].start=starter;
        ender=starter+dif-1;
        if (extra!=0){
            extra--;
            ender++;
        }
        printf("   [%d, %d]\n",starter,ender);
        passable[i].end=ender;
        starter=ender+1;
        if ((retval=pthread_create(&runners[i], NULL, sieve, (void *)(&passable[i]))) != 0) {
            fprintf(stderr,"Error: Cannot create thread %d. %s.\n",i + 1,strerror(retval));
            break;
        }
    }
    for (int i=0;i<threads;i++) {
        if (pthread_join(runners[i],NULL) != 0) {
            fprintf(stderr,"Warning: Thread %d did not join properly.\n",i + 1);
        }
    }
    if ((retval=pthread_mutex_destroy(&lock))!= 0) {
        fprintf(stderr,"Error: Cannot destroy mutex. %s.\n",strerror(retval));
        return EXIT_FAILURE;
    }
    printf("Total primes between %d and %d with two or more '3' digits: %d\n",sI,eI,total_count);
    free(runners);
    free(passable);
    return EXIT_SUCCESS;
}
