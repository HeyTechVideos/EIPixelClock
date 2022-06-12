#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdlib.h>


/*----------------------
 | Display
 -----------------------*/

/**
 * @brief Display settings/variables
 * 
 */
extern uint8_t display_buffer[];                // Bufor ramki, https://pl.wikipedia.org/wiki/Bufor_ramki
// extern uint8_t display_cursor;                  // Cursor
// extern uint8_t display_brightess;               // Brightness (1-255)


/**
 * @brief Display init (clear buffer, reset cursor)
 * 
 */
void display_init();


/**
 * @brief Set display cursor
 * 
 * @param c new cursor position
 */
void display_setCursor(uint8_t c);

/**
 * @brief Get display cursor
 */
void display_getCursor();

/**
 * @brief Set display brightness
 * 
 * @param b new brightness
 */
void display_setBrightness(uint8_t b);

/**
 * @brief Get display brightness
 * 
 */
uint8_t display_getBrightness();

/**
 * @brief Clear buffer
 * 
 */
void display_clearBuffer();


/**
 * @brief Send Buffer
 * 
 */
void display_sendBuffer();


/**
 * @brief Populate buffer with char (font data) 
 * Decodes char using font array
 * Manages cursor and prepare it for next font character
 * 
 * @param c character
 * @param rgb intensity (0-1)
 * @return uint8_t width of the character (according to font array)
 */
uint8_t display_printf_char(char c, float r, float g, float b);
uint8_t display_print_char (char c);

/**
 * @brief Populate buffer with mutiple characters (font data)
 * Calls display_printf_char() multiple times
 * Manages cursor
 * 
 * @param text text
 * @param rgb intensity (0-1)
 * @return uint8_t width of the text (according to font array)
 */
uint8_t display_printf(char *text, float r, float g, float b);
uint8_t display_print (char *text);


/**
 * @brief Populate buffer with bitmap
 * BMP 8x8 TODO any size
 * 
 * @param postac 
 * @param frame 
 */
void display_bmp(uint8_t graphics[192], uint8_t mirror_y);

/**
 * @brief Populate buffer with custom image format pxci (pixel clock image)
 * 
 * PXCI 8x8 // TODO any size
 * For generating please use the script 'pxc_compress_bitmap.py'!
 * 
 * Graphics coded in:
 *  4bit4bit, 4bit4bit,...4bit4bit
 *  (leftTop           ...rightBot)
 *
 * 4bit - coded color index (up to 16 colors)
 *
 * Colormap - contains indexes of colors
 * 
 * Compression:
 *  32*8 + 16*3*8
 *      graphics (32 bytes) + colormap (16 * 3[r,g,b] bytes)
 * 
 * Minimal compression (16 colors):
 *  1536 bytes -> 640 bytes (2.4 times smaller)
 * 
 * Maximal compression (1 color):
 *  1536 bytes -> 304 bytes (5 times smaller)
 * 
 * @param graphics 8x8 graphics (max 16 colors)
 * @param colormap up to 16 colors colormap
 * @param shiftY shift image vertically
 * @return uint8_t width of an image (todo)
 */
uint8_t display_pxci(uint8_t graphics[32], uint8_t colormap[][3], int8_t shiftY);

#endif
