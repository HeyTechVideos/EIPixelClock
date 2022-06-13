/**
 * @file main.c
 * @brief 
 * @version 0.1
 * 
 * @copyright Copyright (c) 2022
 * 
 * Fonts:
 * - https://github.com/rene-d/fontino
 * - https://github.com/dhepper/font8x8
 * 
 * Tools for creating custom fonts:
 * - https://gurgleapps.com/tools/matrix
 * 
 * Font i used as template:
 * - https://fonts2u.com/picopixel.font
 * 
 * 
 * Inne:
 * - https://i.pinimg.com/originals/d1/97/1b/d1971b5caa755512c92e8dfeb37a07c7.gif
 * - https://twitter.com/johanvinet/status/1253351171059855361
 * - https://www.google.com/search?q=pixel+art+joker+8x8&tbm=isch&ved=2ahUKEwipn5ry3uH2AhVwaPEDHQ9EB9kQ2-cCegQIABAA&oq=pixel+art+joker+8x8&gs_lcp=CgNpbWcQAzoFCAAQgAQ6BggAEAcQHjoECAAQHjoGCAAQCBAeOgQIABATOgYIABAeEBM6CAgAEAUQHhATOggIABAIEB4QE1DwAljPB2COCmgAcAB4AIABUogB0AKSAQE1mAEAoAEBqgELZ3dzLXdpei1pbWfAAQE&sclient=img&ei=svU9Yqn-DPDQxc8Pj4idyA0&bih=913&biw=1280#imgrc=Bg4aaf2t4u0CJM&imgdii=IVeDj4yqXg1xfM
 * - https://hexed.it/
 * - TODO https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/SPRITESHEET.md
 * - TODO https://github.com/espressif/esp-idf/blob/master/examples/storage/spiffsgen/main/spiffsgen_example_main.c
 * - https://developer.lametric.com/applications/createdisplay#create-popup <<< STRONA do tworzenia z LAMETRIC!
 * - https://www.piskelapp.com/p/create/sprite <<< PiskelApp (TWORZENIE animacji)
 * 
 */

/**
 * @brief Tworzenie grafik
 * 
 * 1. Wybieram grafike z https://developer.lametric.com/applications/createdisplay#create-popup / tworze koncept
 * 2. Tworze grafike przy pomocy:
 *  a) PiskelApp (w aplikacji zaznaczenie potem z shiftem mozna przesuwac obiekt :) takie info )
 *  b) Gimp - kazda klatka w osobnej warstwie i export
 * 3. Exportuje tak zeby:
 *  - kazda klatka byla w osobnym pliku (najlepiej .png)
 *  - klatki powinny nazywac sie po kolei to znaczy np. "mario_frame01.bmp, mario_frame02.bmp, ... mario_frame99.bmp"
 * 4. Uruchamiam skrypt "img_to_pxci_v4.py"
 * 5. Wygenerowany kod kopiuje w zaleznosci czy grafika czy animacja do "graphics_anim.h"/"graphics.h"
 * 6. Gotowe!
 */

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

/* Moje biblioteki
*/
#include "screens.h"
#include "display.h"
#include "shared.h"


/*----------------------
 | Extern (ASM)
 -----------------------*/
extern void ledon (void);
extern void ledoff(void);

extern void GPIO_SET(uint8_t, uint8_t);


/*----------------------
 | Interrupts handlers
 -----------------------*/

/* ISR Button1
    The default is that the generated code can live in flash. By flagging (IRAM_ATTR) it up front as
    living in instruction RAM, it will always be there and ready to be immediately executed.
*/
uint8_t btn1_pressed = false;
long    btn1_last_time_click = 0;

void IRAM_ATTR isr_btn1_handler(void *args) {
    time_t ltime;

    WRITE_PERI_REG(GPIO_STATUS_W1TC_REG, (1<<25));  // Clear interupt on GPIO25 

    time(&ltime);   // time in seconds
                    // TODO timer jak bedzie trzeba na nizszym poziomie

    if (ltime - btn1_last_time_click >= 1) {     // anti bounce
        btn1_pressed = true;
        btn1_last_time_click = ltime;
    }
}


/*----------------------
 | Main
 -----------------------
 
    TODO interrupts disable/enable portDISABLE_INTERRUPTS(); portENABLE_INTERRUPTS();
    TODO ze reset i konfiguracja przez strone to sie laczy z wifi ktore JA TWORZE z telefonu!
*/
void app_main(void) {
    ESP_LOGE("LOG", "START");

    /* GPIO init
    */
    gpio_pad_select_gpio(GPIO_NUM_2);   // configure pins as GPIO
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    // WS2812B
    gpio_pad_select_gpio(GPIO_NUM_27);  // configure pins as GPIO
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);

    // gpio_set_level(GPIO_NUM_27, 0);
    //GPIO_SET((1<<GPIO_NUM_27), 0);

    // Interrupt button
    gpio_pad_select_gpio(GPIO_NUM_25);
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);
    gpio_set_level(GPIO_NUM_25, 0);

    gpio_set_pull_mode(GPIO_NUM_25, GPIO_PULLUP_ONLY);      // pullup
    gpio_set_intr_type(GPIO_NUM_25, GPIO_INTR_POSEDGE);     // interrupt on POSITIVE edge
    gpio_intr_enable(GPIO_NUM_25);                          // interrupt status clear mask


    /* Button interrupts
        TODO gpio_set_intr_type -> portENTER_CRITICAL https://esp32.com/viewtopic.php?t=12621
    */
    intr_matrix_set(xPortGetCoreID(), ETS_GPIO_INTR_SOURCE, 13);    // ROM function (attach interrupt num 13 to APP_CPU (CPU1))
    xt_set_interrupt_handler(13, isr_btn1_handler, 0);      // by default (file: xtensa_intr_asm.S) generates an array
                                                            // with jump codes to "xt_unhandled_interrupt" (unhandled interrupt array)
                                                            // with "xt_set_interrupt_handler" we change jump to our destination
    xt_ints_on(1<<13);  // enable interrupt num 13
    

    /* Init
    */
    screens_init();
    screens_setBrightness(DISPLAY_BTIGHTNESS);
    strcpy(screenTIME_time, "11:04");

    strcpy(screenMESSAGE_sender, "");
    strcpy(screenMESSAGE_msg, "20037");
    // ESP_LOGE("BTN0", "klik");


    /* Main loop
    */
    while (true) {

        if (btn1_pressed) {
            ESP_LOGE("BTN0", "klik");
        }
        screens_tick(&btn1_pressed);
        vTaskDelay(100 / portTICK_PERIOD_MS);   // portTICK_PERIOD_MS = 1000/100 ms = 10 ms


    }
}



