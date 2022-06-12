#include "ws2812b.h"
#include "shared.h"

/**
 * @brief Send byte to ws2812b
 * 
 * @param _byte one byte to send
 */
inline void ws2812b_sendByte(uint8_t _byte) {
    // TODO disable interrupts
    // TODO zoptymalizowac
    /* WS2812B first takes MOST significant bit (left one first) */
    for (int i = 7; i >= 0; i--) {
        WS2812B_SEND_BIT((_byte >> i));    // przesuwamy w prawo
    }
}


void ws2812b_sendRGB(uint8_t r, uint8_t g, uint8_t b) {
    /* WS2812B accepts in that order GRB */
    ws2812b_sendByte(g);
    ws2812b_sendByte(r);
    ws2812b_sendByte(b);
}


void ws2812b_sendRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    /* WS2812B accepts in that order GRBW */
    ws2812b_sendByte(g);
    ws2812b_sendByte(r);
    ws2812b_sendByte(b);
    ws2812b_sendByte(w);
}
