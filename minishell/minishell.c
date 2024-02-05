/*******************************************************************************
* Name        : minishell.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/20/2021
* Description : builds mini shell prompt
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <wait.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <limits.h>


#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"
#define BUFSIZE 4096

volatile sig_atomic_t interrupted = 0;

bool starts_with(const char *str, const char *prefix) {
    if(strlen(prefix) > strlen(str)){
      return false;
    }else{
      return (strncmp(prefix, str, strlen(prefix)) == 0);
    } 

}

void catch_signal(int sig){
    interrupted = 1;
}

void changeddirectory(char* route){
  if (!strcmp(route, "~")||!strcmp(route, " ")){
    struct passwd *password;
    
    if ((password = getpwuid(getuid())) == NULL){
      fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
    }

    if (chdir(password->pw_dir) == -1){
      fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", password->pw_dir, strerror(errno));
    }
  } else if (chdir(route) == -1){
      fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", route ,strerror(errno));
    }
  }
  


int main(int argc, char *argv[]){
  struct sigaction action;
  memset(&action, 0, sizeof(struct sigaction));
  action.sa_handler = catch_signal;
  

  if (sigaction(SIGINT, &action, NULL) == -1){
    fprintf(stderr, "Error: Could not registar signal handler.  %s \n", strerror(errno));
    return EXIT_FAILURE;
  }


  char buffer[BUFSIZE];
  while(true){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr,"Error: Cannot get current working directory. %s.\n",strerror(errno));
        return EXIT_FAILURE;
    }
    printf("[%s%s%s]$ ", BRIGHTBLUE, cwd, DEFAULT);
    fflush(stdout);

    int amt_read = read(STDIN_FILENO, buffer, BUFSIZE-1);
    if(amt_read > 0){
      buffer[amt_read - 1]='\0';    
    }else if (amt_read < 0){
      if (errno == EINTR){
        printf("\n");
        continue;
      }
      fprintf(stderr,"Error: Failed to read from stdin. %s.\n",strerror(errno));
      return EXIT_FAILURE;      
    } 
    char *found = strtok(buffer, " ");
    int ac=0;
    char *av[BUFSIZE];
    while(found != NULL){
      av[ac]= found;
      ac++;
      found = strtok(NULL, " ");
    }

    //for (int i = 0; i < ac; i++){
    //  printf("%s\n", av[i]);
    //}

    //printf("%d\n", ac);
    if (ac == 0){
      continue;
    } 
      if(strcmp(av[0],"exit")==0){
          while(ac>0){
            ac--;
          }
          return EXIT_SUCCESS;
      }
      if(strcmp(av[0],"cd")==0){
          if(ac==1){
            changeddirectory("~");
          }
          else if(ac==2){
            changeddirectory(av[1]);
          }else{
            fprintf(stderr,"Error: Too many arguments to cd.\n" );
          }
      }else if(ac>= 1){
          pid_t child =fork();
          av[ac]=NULL;
          if(child<0){
            fprintf(stderr,"Error: fork() failed. %s.\n",strerror(errno));
          }
          else if (child==0){
              if(execvp(av[0],av)==-1){
                fprintf(stderr,"Error: exec() failed. %s.\n",strerror(errno));
              }
              while(ac>0){
                ac--;
              }
              exit(EXIT_FAILURE);
          }else{
              int waiter;
              interrupted=1;
              pid_t paitent = wait(&waiter);
              if(paitent<0){
                  fprintf(stderr,"Error: wait() failed. %s.\n",strerror(errno));
                  while(ac>0){
                    ac--;
                  }
                  return EXIT_FAILURE;
              }
              interrupted=0;
          }
        }
  }
}