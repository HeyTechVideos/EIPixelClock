#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "screens.h"
#include "shared.h"
#include "display.h"
#include "graphics.h"
#include "graphics_anim.h"

#include "freertos/FreeRTOS.h"

#include "esp_log.h"


/*==============================================================================================================
 || Variables for all screens
 ===============================================================================================================*/

uint8_t      screens_id = 0;                        // ID of the current screen
void       (*screens_tab[255]) ();                  // array which contains all available Screens
uint16_t     screens_ticks      = 0;                     // 
uint16_t     screens_ticks_help = 0;                     // Counter for ticks, rozne wykorzystania :)

int8_t       screens_scroll_cursor = 0;             // cursor var needed for scrolling

uint8_t      *screens_btn1;                         // button1
uint8_t      screens_MAX_BRIGHTNESS;

/* Graphics - frame by frame animation */
int8_t       screens_anim_frame = 0;                // animation frame


/*==============================================================================================================
 || Screens - Main Functions
 ===============================================================================================================*/
void screens_init() {
    /* Init display
    */
    display_init();

    /* Init screens
    */
    screens_tab[0] = screens_Loading;
    // screens_tab[1] = screens_Message;
    // screens_tab[1] = screens_Time;
    screens_tab[1] = screens_ScrollRob;
}

void screens_setBrightness(uint8_t b) {
    screens_MAX_BRIGHTNESS = b;
    display_setBrightness(b);
}

void screens_change(uint8_t s_id) {
    screens_id = s_id;                              // set screen id for next tick
    screens_scroll_cursor = 0;                      // reset scroll to 0
}

void screens_tick(uint8_t *btn_pressed) {
    if (screens_ticks > 65000) screens_ticks = 0;   // tick overflow
    screens_btn1 = btn_pressed;                     // button1

    (*screens_tab[screens_id]) ();                  // call the screen func, screens_tab[screens_id] ();

    *screens_btn1 = false;                          // reset button1 flag
    
    screens_ticks++;                                // increase tick
}


/*==============================================================================================================
 || Transitions
 ===============================================================================================================*/

/* Transition: Brightness */
uint8_t      screens_tr_bright = 0;             // transition brightness
uint8_t      screens_tr_bright_run = false;     // transition brightness should be running?

void transition_BrightInit() {
    screens_tr_bright = 0;
    screens_tr_bright_run = true;
}

void transition_BrightTick() {
    if (screens_tr_bright >= screens_MAX_BRIGHTNESS) {
        screens_tr_bright_run = false;
        screens_tr_bright = screens_MAX_BRIGHTNESS;
    }
    display_setBrightness(screens_tr_bright);

    if (screens_tr_bright < 5) screens_tr_bright ++; // jakies farmazony (dostosowuje zeby efekt byl git)
    screens_tr_bright *= 2;
}


/*==============================================================================================================
 || Custom Screens
 ===============================================================================================================*/


/*--------------------------------------------------------------------------------------------------------------
 | Screen: LOADING
 ---------------------------------------------------------------------------------------------------------------*/
/* LOADING: main */
void screens_Loading() {
    display_clearBuffer();

    if (screens_ticks >= 12) screens_change(1);     // after x time change to screen MAIN
    
    display_setCursor(12);

    // Graphics animation frame
    if (screens_ticks%2 == 0) screens_anim_frame ++;
    if (screens_anim_frame >= anim_pxci_loading_fc) screens_anim_frame = 0;

    // Display graphics
    display_pxci(anim_pxci_loading[screens_anim_frame].graphics, anim_pxci_loading[screens_anim_frame].colormap, 0);   // "." used for non pointers, "->" for pointers
    
    // Send bufer
    portDISABLE_INTERRUPTS(); display_sendBuffer(); portENABLE_INTERRUPTS();
}


/*--------------------------------------------------------------------------------------------------------------
 | Screen: TIME
 ---------------------------------------------------------------------------------------------------------------*/
char    screenTIME_time[64];    // Time string
uint8_t screenTIME_gp_i = 0;    // Current graphic

uint8_t screenTIME_tran_status = 0;    // Transition status: 0 - none, 1 - stage1, 2 - stage 2
int8_t  screenTIME_tran_count  = 0;    // Transition counter

/* Graphics for screen TIME    
    IMPORTANT! "const char *arr" array of pointers to "const char", you can modify the array elements, but can't modify what they point to
    https://stackoverflow.com/questions/22763362/what-is-an-array-of-constant-pointers-in-c */
const GraphicsFrame_t *screenTIME_gp[] = {
    &anim_pxci_clock[0],                                                    // &anim_pxci_clock[0] == anim_pxci_clock, pointer to the first element
    anim_pxci_pacmanR,
    anim_pxci_pacmanL,
    anim_pxci_tetrisfull,
    i_pxci_sandglass, i_pxci_timer, i_pxci_tick,     // images
};

uint8_t screenTIME_gp_fc[] = {                                              // frame count of each graphic (image then fc=1)
    anim_pxci_clock_fc,
    anim_pxci_pacmanR_fc,
    anim_pxci_pacmanL_fc,
    anim_pxci_tetrisfull_fc,
    1, 1, 1,                                                             // images, fc=1
};

/* TIME: OnTransitionAction */
void screens_Time_OnTransitionAction() {
  if (screenTIME_tran_status > 0) {

        screenTIME_tran_count ++;

        // First stage completed (prev graphic slided down)
        if (screenTIME_tran_status == 1 && screenTIME_tran_count >= 8) {

            // start second stage (new graphic slides from top)
            screenTIME_tran_status = 2;
            screenTIME_tran_count = -8;
            
            screenTIME_gp_i ++;     // new graphic
            screens_anim_frame = 0; // reset anim

        // Second stage completed, Done.
        } else if (screenTIME_tran_status == 2 && screenTIME_tran_count >= 0) {
            screenTIME_tran_status = 0; // Done.
            screenTIME_tran_count = 0;
        }

    }
}

/* TIME: main
    https://linuxhint.com/gettimeofday_c_language/
*/
void screens_Time() {
   
    /* Button1 Click
    */
    // Change graphics and start transition
    // if (*screens_btn1) {
    //     screenTIME_gp_i ++;

    //     transition_BrightInit();
    // }

    // // Tick transition
    // if (screens_tr_bright_run) {
    //     transition_BrightTick();
    // }


    // On button1 click
    if (*screens_btn1) {

        // 1. Graphic Transition Animation
        screenTIME_tran_status = 1; // transition start
        screenTIME_tran_count = 0;  // reset counter
    }

    // 1. Graphic Transition Animation
    screens_Time_OnTransitionAction();
  

    /* Display graphics and time
    */
    char time_buff[64];
    display_clearBuffer();
    display_setCursor(0);

    // Get graphics
    if (screenTIME_gp_i >= sizeof(screenTIME_gp)/sizeof(screenTIME_gp[0])) screenTIME_gp_i = 0;  // graphics index overflow
    const GraphicsFrame_t *frames      = screenTIME_gp[screenTIME_gp_i];
    uint8_t                frame_count = screenTIME_gp_fc[screenTIME_gp_i];

    // Graphics animation frame
    if (screens_ticks%2 == 0) screens_anim_frame ++;
    if (screens_anim_frame >= frame_count) screens_anim_frame = 0;

    // Display graphics
    display_pxci(frames[screens_anim_frame].graphics, frames[screens_anim_frame].colormap, screenTIME_tran_count);   // "." used for non pointers, "->" for pointers

    // Display time
    display_setCursor(11);
    strcpy(time_buff, screenTIME_time);
    if (screens_ticks%20 < 10) time_buff[2] = ' ';    // tick every 100ms, so blink every second
    display_print(time_buff);
    
    // Send bufer
    portDISABLE_INTERRUPTS(); display_sendBuffer(); portENABLE_INTERRUPTS();
}


/*--------------------------------------------------------------------------------------------------------------
 | Screen: MESSAGE
 ---------------------------------------------------------------------------------------------------------------*/
char screenMESSAGE_sender[64] = ""; // Author
char screenMESSAGE_msg[64] = "";    // Message

void screens_anim_helper(GraphicsFrame_t anim_pxci[], uint8_t fc, int8_t shiftY, uint8_t speed_modulo) {
    /** @brief IMPORTANT! Need a flush (sendBuffer)
     * 
     * @param speed_module 1 = fastest possible!, for other speeds think it over which value to type
    */

    // Graphics animation frame
    if (screens_ticks%speed_modulo == 0) screens_anim_frame ++; 
    if (screens_anim_frame >= fc) screens_anim_frame = 0;

    // Display graphics
    display_pxci(anim_pxci[screens_anim_frame].graphics, anim_pxci[screens_anim_frame].colormap, shiftY);   // "." used for non pointers, "->" for pointers
}

void screens_scroll_helper(int8_t cursor_start, uint8_t width) {

    // Scroll if needed
    if (width >= 32-cursor_start) {

        // Create some delay for every first loop (to give some time to read the first letters/words)
        if (screens_scroll_cursor == 0) {   
            screens_ticks_help ++;

            if (screens_ticks_help > 10) screens_scroll_cursor --;

        } else screens_scroll_cursor --;

        // Reset if scrolled everything
        if (screens_scroll_cursor < -width) {
            screens_scroll_cursor = 0;
            screens_ticks_help = 0;
        }
    }

}

/* MESSAGE: main */
void screens_Message() {
    display_clearBuffer();

    // Message
    display_setCursor(9+screens_scroll_cursor);
    uint8_t t_width1 = display_printf(screenMESSAGE_sender, 0, 1, 1);
    display_setCursor(9+screens_scroll_cursor+t_width1);
    uint8_t t_width2 = display_print(screenMESSAGE_msg);

    screens_scroll_helper(9, t_width1+t_width2);

    // Graphic
    display_setCursor(0);
    // screens_anim_helper(i_pxci_messenger, 1, 0);
    screens_anim_helper(anim_pxci_message, anim_pxci_message_fc, 0, 1);

    // Send bufer
    portDISABLE_INTERRUPTS(); display_sendBuffer(); portENABLE_INTERRUPTS();
}

/*--------------------------------------------------------------------------------------------------------------
 | Screen: SCROLLROB
 ---------------------------------------------------------------------------------------------------------------*/
void screens_ScrollRob() {
    display_clearBuffer();

    // On button1 click
    if (*screens_btn1) {

        // 1. Graphic Transition Animation
        screenTIME_tran_status = 1; // transition start
        screenTIME_tran_count = 0;  // reset counter
    }

    // 1. Graphic Transition Animation
    screens_Time_OnTransitionAction();

    // Message
    display_setCursor(9+screens_scroll_cursor);
    uint8_t t_width1 = display_printf(screenMESSAGE_sender, 0, 1, 1);
    display_setCursor(9+screens_scroll_cursor+t_width1);
    uint8_t t_width2 = display_print(screenMESSAGE_msg);

    screens_scroll_helper(9, t_width1+t_width2);

    // Graphic
    display_setCursor(0);
    // screens_anim_helper(i_pxci_messenger, 1, 0);
    // screens_anim_helper(anim_pxci_message, anim_pxci_message_fc, 0, 1);

    // Get graphics
    if (screenTIME_gp_i >= sizeof(screenTIME_gp)/sizeof(screenTIME_gp[0])) screenTIME_gp_i = 0;  // graphics index overflow
    const GraphicsFrame_t *frames      = screenTIME_gp[screenTIME_gp_i];
    uint8_t                frame_count = screenTIME_gp_fc[screenTIME_gp_i];

    // Graphics animation frame
    if (screens_ticks%2 == 0) screens_anim_frame ++;
    if (screens_anim_frame >= frame_count) screens_anim_frame = 0;

    // Display graphics
    display_pxci(frames[screens_anim_frame].graphics, frames[screens_anim_frame].colormap, screenTIME_tran_count);   // "." used for non pointers, "->" for pointers



    // Send bufer
    portDISABLE_INTERRUPTS(); display_sendBuffer(); portENABLE_INTERRUPTS();
}






// TODO Messages GET https://gmail.googleapis.com/gmail/v1/users/{userId}/messages




