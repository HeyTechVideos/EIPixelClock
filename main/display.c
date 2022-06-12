#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "esp_log.h"

#include "font.h"
#include "display.h"
#include "utils.h"
#include "ws2812b.h"
#include "shared.h"

const uint16_t font_to_buff_decode[8][32] = { // todo map nazwa, dekoduje uklad czcionki czy grafiki (wierszami) na uklad WS2812B matrix (gdzie idzie dziwnie wezykiem)
    {0, 45, 48, 93, 96, 141, 144, 189, 192, 237, 240, 285, 288, 333, 336, 381, 384, 429, 432, 477, 480, 525, 528, 573, 576, 621, 624, 669, 672, 717, 720, 765},
    {3, 42, 51, 90, 99, 138, 147, 186, 195, 234, 243, 282, 291, 330, 339, 378, 387, 426, 435, 474, 483, 522, 531, 570, 579, 618, 627, 666, 675, 714, 723, 762},
    {6, 39, 54, 87, 102, 135, 150, 183, 198, 231, 246, 279, 294, 327, 342, 375, 390, 423, 438, 471, 486, 519, 534, 567, 582, 615, 630, 663, 678, 711, 726, 759},
    {9, 36, 57, 84, 105, 132, 153, 180, 201, 228, 249, 276, 297, 324, 345, 372, 393, 420, 441, 468, 489, 516, 537, 564, 585, 612, 633, 660, 681, 708, 729, 756},
    {12, 33, 60, 81, 108, 129, 156, 177, 204, 225, 252, 273, 300, 321, 348, 369, 396, 417, 444, 465, 492, 513, 540, 561, 588, 609, 636, 657, 684, 705, 732, 753},
    {15, 30, 63, 78, 111, 126, 159, 174, 207, 222, 255, 270, 303, 318, 351, 366, 399, 414, 447, 462, 495, 510, 543, 558, 591, 606, 639, 654, 687, 702, 735, 750},
    {18, 27, 66, 75, 114, 123, 162, 171, 210, 219, 258, 267, 306, 315, 354, 363, 402, 411, 450, 459, 498, 507, 546, 555, 594, 603, 642, 651, 690, 699, 738, 747},
    {21, 24, 69, 72, 117, 120, 165, 168, 213, 216, 261, 264, 309, 312, 357, 360, 405, 408, 453, 456, 501, 504, 549, 552, 597, 600, 645, 648, 693, 696, 741, 744}
};

uint8_t display_buffer[BUFFER_LENGTH] = {};     // Bufor ramki, https://pl.wikipedia.org/wiki/Bufor_ramki, TODO define 768
uint8_t display_cursor = 0;
uint8_t display_brightess = DISPLAY_BTIGHTNESS;                 // Brightness (1-255), Default: 10 (please consider using better power source if u want to increase this nubmer greatly)


void display_init() {
    display_cursor = 0;
    display_clearBuffer();
}

void display_setCursor(uint8_t c) {
    display_cursor = c;
}

void display_getCursor() {
    return display_cursor;
}

void display_setBrightness(uint8_t b) {
    display_brightess = b;
}

uint8_t display_getBrightness() {
    return display_brightess;
}

void display_clearBuffer() {
    memset(display_buffer, 0, BUFFER_LENGTH);  // copy 0 to display_buffer
}

void display_sendBuffer() {
    WS2812B_RESET();
    for (int i=0; i<BUFFER_LENGTH; i+=3) {
        ws2812b_sendRGB(display_buffer[i], display_buffer[i+1], display_buffer[i+2]);
    }
    WS2812B_RESET();
}


/**
 * @brief Displays single character on ws2812b matrix
 * 
 * @param c   character to display
 * @param rgb intensity (0-1)
 * 
 * @return sign_width: width of the character c (actual width of the sign encoded in font_8x8_default)
 */
uint8_t display_print_char (char c) { return display_printf_char(c, 1, 1, 1); }
uint8_t display_printf_char(char c, float r, float g, float b) {

    uint8_t bit_array[8], sign_width = 0;
    uint16_t buff_index = 0;

    for (uint8_t w = 0; w < 8; w++) {
        decToBin8(font_8x8_default[c - 32][w], &bit_array);     // Z danego wiersza (ze tablicy znakow) rozkodowuje kolumne hex na bin
        
        for (uint8_t k = 0; k < 8; k++) {

            // Decode font -> ws2812b matrix
            buff_index = (uint16_t)font_to_buff_decode[w][k+display_cursor];

            // Load to buffer
            // jezeli przekracza zakres 0-32 to widocznie jest poza, nie wyswietlamy
            if (k+display_cursor < 32 && k+display_cursor >= 0) {
                display_buffer[buff_index  ] = bit_array[k]*display_brightess *r;
                display_buffer[buff_index+1] = bit_array[k]*display_brightess *g;
                display_buffer[buff_index+2] = bit_array[k]*display_brightess *b;
            }

            // Set cursor for next sign (find the max width of the current sign)
            if (k > sign_width && bit_array[k] > 0) {
                sign_width ++;
            }
        }
    }

    // Setting cursor for future sign
    // sign_width is now set at the end of the sign
    // but we want to +2 (+1 we are at the position after sign, +2 we are at the position after sign + one column space)
    sign_width += 2; display_cursor += sign_width;

    return sign_width;
}


/**
 * @brief Displays multiple characters on ws2812b matrix
 * 
 * @param c   character to display
 * @param rgb intensity (0-1)
 * 
 * @return sign_width: width of the character c (actual width of the sign encoded in font_8x8_default)
 */
uint8_t display_print (char *text) { return display_printf(text, 1, 1, 1); }
uint8_t display_printf(char *text, float r, float g, float b) {
   uint8_t text_width = 0;

    while (*text) {                                     // Until value pointed by var is not NULL
        text_width += display_printf_char(*(text++), r, g, b);   // print single char, then increment pointer to next char value in array
    }

    return text_width;
}


/**
 * @brief BITMAP 8x8 // TODO any size
 * 
 * Bitmap pixel structure, START row (top left px first), next row, ...:
 *      last -> 0xB2, 0x22, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xA4, 0xF4,
 *               ...
 *      next -> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *      START-> 0x11, 0xA1, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 *           -> 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x23, 0xF3
 *   
 *   Starting with left to right [24 bytes, 8-byte * 3 -> (row data)*(rgb)]
 *
 *   So first row of our image (starting with left top pixel of our image):
 *      0x11, 0xA1, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x23, 0xF3
 *      (1st px r,g,b)....(2nd px r,g,b)....
 *
 *  If we load it from the last index (191) to first (0) we will get mirrored image.
 *  To avoid that, just reverse painting every row with formual: 8 - column_number [8px = image width]
 *
 * @param graphics 
 * @param mirror_y true/false
 */
void display_bmp(uint8_t graphics[192], uint8_t mirror_y) {

    uint16_t buff_index = 0;

    for (uint8_t w=0; w<8; w++) {
        for (uint8_t k=0; k<24; k+=3) {

            if (mirror_y) buff_index = font_to_buff_decode[w][k/3 + display_cursor];
            else          buff_index = font_to_buff_decode[w][8-k/3 + display_cursor];

            display_buffer[buff_index  ] = graphics[191-(w*24+k   )] * (float)(display_brightess/255.0f);
            display_buffer[buff_index+1] = graphics[191-(w*24+k +1)] * (float)(display_brightess/255.0f);
            display_buffer[buff_index+2] = graphics[191-(w*24+k +2)] * (float)(display_brightess/255.0f);
        }
    }

}

/**
 * @brief PXCI 8x8 TODO any size
 * For generating please use the script 'pxc_compress_bitmap.py'!
 *
 *  Graphics coded in:
 *      4bit4bit, 4bit4bit,...4bit4bit
 *      (leftTop(start)    ...rightBot(end))
 *
 *  4bit - coded color index (up to 16 colors)
 *  Colormap - contains indexes of colors
 *
 *  Compression:
 *      32*8 + 16*3*8
 *      graphics (32 bytes) + colormap (16 * 3[r,g,b] bytes)
 *
 *  Minimal compression (16 colors):
 *      1536 bytes -> 640 bytes (2.4 times smaller)
 *  Maximal compression (1 color):
 *      1536 bytes -> 304 bytes (5 times smaller)
 * 
 * @param graphics 
 * @param colormap 
 * @param shiftY 
 * @return uint8_t 
 */
uint8_t display_pxci(uint8_t graphics[], uint8_t colormap[][3], int8_t shiftY) {
    uint8_t pxH=0;
    uint8_t pxL=0;
    
    uint8_t k=0;
    uint16_t buff_index  = 0;

    for (uint8_t i=0; i<32; i++) {
        pxH = (graphics[i] >> 4);       // shift to get HIGH bits
        pxL = (graphics[i] & 0x0F);     // and mask to get LOW bits

        if (i%4 == 0) k = 0;                // each hex contains 8bits (2x4bits) so 8(byte) pixels (row) coded in 4 hexs

        if ((uint8_t)(i/4)+shiftY >= 8 || (uint8_t)(i/4)+shiftY < 0) continue;   // prevent too much and too small value

        buff_index = font_to_buff_decode[(uint8_t)(i/4)+shiftY][display_cursor + k++];  // decode first 4bits
        display_buffer[buff_index]   = colormap[pxH][0] * (float)(display_brightess/255.0f);    // decode colors using colomap
        display_buffer[buff_index+1] = colormap[pxH][1] * (float)(display_brightess/255.0f);
        display_buffer[buff_index+2] = colormap[pxH][2] * (float)(display_brightess/255.0f);

        buff_index = font_to_buff_decode[(uint8_t)(i/4)+shiftY][display_cursor + k++];  // decode next 4bits
        display_buffer[buff_index]   = colormap[pxL][0] * (float)(display_brightess/255.0f);    // decode colors using colomap
        display_buffer[buff_index+1] = colormap[pxL][1] * (float)(display_brightess/255.0f);
        display_buffer[buff_index+2] = colormap[pxL][2] * (float)(display_brightess/255.0f);
    }

    return 8;   // TODO width (for now const)
}



