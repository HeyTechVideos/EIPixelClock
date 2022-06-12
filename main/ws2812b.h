#ifndef _WS2812B_H_
#define _WS2812B_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ----------------------
 | Externs (ASM)
 ------------------------ */
extern uint32_t mwait(uint32_t, uint32_t);

extern void     WS2812B_RESET();
extern uint32_t WS2812B_SEND_BIT(uint32_t);


/* ----------------------
 | WS2812B functions
 ------------------------ */


/**
 * @brief Send RGB bytes to ws2812b
 * 
 * @param r red
 * @param g green
 * @param b blue
 */
void ws2812b_sendRGB(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Send RGBW bytes to ws2812b
 * 
 * @param r red
 * @param g green
 * @param b blue
 * @param w white
 */
void ws2812b_sendRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w);

#endif
