/*******************************************************************************
* Name        : sum.c
* Author      : Nicholas Cali & Kyle Henderson
* Date        : 6/24/2021
* Description : add array using a shared library.
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/
#include "sum.h"

/**
 * TODO:
 * Takes in an array of integers and its length.
 * Returns the sum of integers in the array.
 */
int sum_array(int *array, const int length) {
    int sum = 0;
    for (int x = 0; x < length;x++){
        sum += array[x];
    }
    return sum;
}
