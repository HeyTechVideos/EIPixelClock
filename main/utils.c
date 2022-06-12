#include "utils.h"
#include "shared.h"


void decToBin8(uint8_t dec_num, uint8_t *bit_array) {
    /* Reszta z dzielenia i dzielimy */
    for (uint8_t i = 0; i < 8; i++) {
        bit_array[8-i-1] = dec_num % 2;
        dec_num /= 2;
    }
}
