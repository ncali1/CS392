/*******************************************************************************
 * Name        : sort.c
 * Author      : Nicholas Cali & Kyle Henderson
 * Date        : 6/3/2021
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;


/**
 * Reads data from filename into an already allocated 2D array of chars.
 * Exits the entire program if the file cannot be opened.
 */
size_t read_data(char *filename, char **data) {
    // Open the file.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", filename,
                strerror(errno));
        free(data);
        exit(EXIT_FAILURE);
    }

    // Read in the data.
    size_t index = 0;
    char str[MAX_STRLEN + 2];
    char *eoln;
    while (fgets(str, MAX_STRLEN + 2, fp) != NULL) {
        eoln = strchr(str, '\n');
        if (eoln == NULL) {
            str[MAX_STRLEN] = '\0';
        } else {
            *eoln = '\0';
        }
        // Ignore blank lines.
        if (strlen(str) != 0) {
            data[index] = (char *)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(data[index++], str);
        }
    }

    // Close the file before returning from the function.
    fclose(fp);

    return index;
}


void printer(){
    printf("Usage: ./sort [-i|-d] filename\n   -i: Specifies the file contains ints.\n    -   d: Specifies the file contains doubles.\n    filename: The file to sort.\n    No flags defaults to sorting strings.");
}
/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv) {
    if(argc==1){
        printer();
        return EXIT_FAILURE;
    }
    else if(argc==2){
        if(argv[1][0]=='-'){
            fprintf(stderr,"Error: No input file specified.");
            return EXIT_FAILURE;
        }
        char *passed[MAX_ELEMENTS];
        char *finial[MAX_STRLEN];
        for (int x=0; x<MAX_ELEMENTS; x++){
            passed[x]= (char*)calloc(MAX_STRLEN+2, sizeof(char));
        }
        int length=read_data(argv[1], passed);
        for(int y=0; y<length;y++){
                finial[y]= passed[y];
        }
        quicksort(finial, length, sizeof(char*), str_cmp);
        for(int y=0; y<length; y++){
            printf("%s\n", passed[y]);
        }
    }
    else{
        if(argc>3||argv[1][0]!='-'){
            fprintf(stderr,"Error: Too many files specified.");
            return EXIT_FAILURE;
        }
        int sit;
        int errorchecker=0;
        char *passed[MAX_ELEMENTS];
        for(int x=0; x<MAX_ELEMENTS; x++){
            passed[x]= (char*)calloc(MAX_STRLEN+2, sizeof(char));
        }
        int length=read_data(argv[2], passed);
        while((sit= getopt(argc, argv, ":i :d"))!=-1){
            switch(sit){
                case 'i':{
                    if(errorchecker>0){
                        fprintf(stderr,"Error: Too many flags specified.");
                        return EXIT_FAILURE;
                    }
                    int runner[MAX_STRLEN];
                    for(int y=0; y<length;y++){
                        runner[y]= atoi(passed[y]);
                    }
                    quicksort(runner, length, sizeof(int), int_cmp);
                    for(int y=0; y<length; y++){
                        printf("%d\n", runner[y]);
                    }
                    errorchecker++;
                    break;}
                case 'd':{
                    if(errorchecker>0){
                        fprintf(stderr,"Error: Too many flags specified.");
                        return EXIT_FAILURE;
                    }
                    double winner[MAX_STRLEN];
                    for(int y=0; y<length;y++){
                        winner[y]= atof(passed[y]);
                    }
                    quicksort(winner, length, sizeof(double), dbl_cmp);
                    for(int y=0; y<length; y++){
                        printf("%f\n", winner[y]);
                    }
                    errorchecker++;
                    break;}
                case '?':
                    fprintf(stderr,"Error: Unknown option '%s' received.\n", argv[1]);
                    printer();
                    return EXIT_FAILURE;
            }
        }
    }
    
    return EXIT_SUCCESS;
}