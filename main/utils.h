#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*----------------------
 | Utilities
 -----------------------*/

/**
 * @brief Converts decimal number (byte) to bit array
 * 
 * @param dec_num 8-bit (one byte) number
 * @param bit_array result array
 */
void decToBin8(uint8_t dec_num, uint8_t *bit_array);

#endif
