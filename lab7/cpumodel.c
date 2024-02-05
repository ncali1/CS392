/*******************************************************************************
* Name        : cpumodel.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/14/2021
* Description : prints out cpu of users computer
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool starts_with(const char *str, const char *prefix) {
    /* TODO:
       Return true if the string starts with prefix, false otherwise.
       Note that prefix might be longer than the string itself.
     */
    if(strlen(prefix) > strlen(str)){
      return false;
    }else{
      return (strncmp(prefix, str, strlen(prefix)) == 0);
    } 

}

int main() {
    /* TODO:
       Open "cat /proc/cpuinfo" for reading, redirecting errors to /dev/null.
       If it fails, print the string "Error: popen() failed. %s.\n", where
       %s is strerror(errno) and return EXIT_FAILURE.
     */
      FILE *pipe;
      FILE *f;
      int exit_status;
      char buf[256];


      if ((pipe = popen("cat /proc/cpuinfo 2>/dev/null", "r")) == NULL){
        fprintf(stderr, "Error: popen() failed. %s,\n", strerror(errno));
        return EXIT_FAILURE;
      }

      if((f = fopen("/proc/cpuinfo", "r")) == NULL){
        fprintf(stderr, "Error: failed to open file. %s,\n", strerror(errno));
        return EXIT_FAILURE;
      }

    /* TODO:
       Allocate an array of 256 characters on the stack.
       Use fgets to read line by line.
       If the line begins with "model name", print everything that comes after
       ": ".
       For example, with the line:
       model name      : AMD Ryzen 9 3900X 12-Core Processor
       print
       AMD Ryzen 9 3900X 12-Core Processor
       including the new line character.
       After you've printed it once, break the loop.
     */

      while ((fgets(buf, 255, f)) != NULL){
        if (starts_with(buf, "model name")){
          printf("%s", strchr(buf, ':')+2);
          break;
        }

      }

      

    /* TODO:
       Close the file descriptor and check the status.
       If closing the descriptor fails, print the string
       "Error: pclose() failed. %s.\n", where %s is strerror(errno) and return
       EXIT_FAILURE.
     */
    if(ferror(f)) {
      fprintf(stderr, "Error: failed to properly read from file.\null");
    }

    if(fclose(f) < 0) {
      fprintf(stderr, "Error: failed to close file. %s.\n", strerror(errno));
      return EXIT_FAILURE;
    }

    if((exit_status = pclose(pipe)) < 0) {
      fprintf(stderr, "Error: failed to close pipe. %s.\n", strerror(errno));
      return EXIT_FAILURE;
    }


    return !(WIFEXITED(exit_status) && WEXITSTATUS(exit_status) == EXIT_SUCCESS);
}
