/*******************************************************************************
* Name        : spfind.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/14/2021
* Description : returns permission output sorted
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <wait.h>


bool starts_with(const char *str, const char *prefix) {
    if(strlen(prefix) > strlen(str)){
      return false;
    }else{
      return (strncmp(prefix, str, strlen(prefix)) == 0);
    } 

}


int main(int argc, char *argv[]){
    if (argc == 1){
        printf("Usage: ./spfind -d <directory> -p <permissions string> [-h]\n");
        return EXIT_SUCCESS;
    }
    int pf_to_s[2], s_to_p[2];
    pid_t ids[2];
    int stat;
    pipe(pf_to_s);
    pipe(s_to_p);

    if ((ids[0] = fork()) == 0){
        close(pf_to_s[0]);
        dup2(pf_to_s[1], STDOUT_FILENO);
        close(s_to_p[1]);
        close(s_to_p[0]);
        if(execv("pfind", argv) == -1){
            fprintf(stderr, "Error: pfind failed.\n");
            exit(EXIT_FAILURE);
        }
    }else if(ids[0] < 0){
       	fprintf(stderr, "Error: fork failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if ((ids[1] = fork()) == 0){
        close(pf_to_s[1]);
        dup2(pf_to_s[0], STDIN_FILENO);        
        close(s_to_p[0]);
        dup2(s_to_p[1], STDOUT_FILENO);
        if(execlp("sort", "sort", NULL) == -1){
            fprintf(stderr, "Error: sort failed.\n");
            exit(EXIT_FAILURE);
        }    
    }else if(ids[1] < 0){
       	fprintf(stderr, "Error: fork failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }


    close(s_to_p[1]);
    dup2(s_to_p[0], STDIN_FILENO);
    close(pf_to_s[1]);
    close(pf_to_s[0]);

    
    for (int i = 0; i<2; i++){
        do {
            pid_t p = waitpid(ids[i], &stat, WUNTRACED | WCONTINUED);
            if (p == -1) {
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));
        if (WEXITSTATUS(stat) == EXIT_FAILURE && stat != 0) {
            return EXIT_FAILURE;
        }
    }


    int match_count = 0;
    char buf[4096];
    while (1) {        
        ssize_t count = read(STDIN_FILENO, buf, sizeof(buf));        
        if (count == -1) {            
            if (errno == EINTR) {                
                continue;            
            } else {                
                perror("read()");                
                exit(EXIT_FAILURE);            
            }        
        } else if (count == 0) {            
            if (match_count == 0){
                printf("Total matches: %d\n", match_count);
            }else{
                match_count = match_count - 2;
                printf("Total matches: %d\n", match_count);
            }
            break;        
        } else {
            for(int i = 0; i < sizeof(buf); i++){
                if(buf[i] == '\n'){
                    match_count++;
                }
            }
            if(starts_with(buf, "Usage")){
                write(STDOUT_FILENO, buf, count);  
                break;          
            }            
            write(STDOUT_FILENO, buf, count);        
        }    
    }
    return EXIT_SUCCESS;
}