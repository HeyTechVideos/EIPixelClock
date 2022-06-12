#ifndef _SCREENS_H_
#define _SCREENS_H_

#include <stdlib.h>

void screens_init();
void screens_setBrightness(uint8_t b);
void screens_change(uint8_t s_id);
void screens_tick();


/*==============================================================================================================
 || Transitions - animated transitions
 ===============================================================================================================*/
 /**
  * @brief Transition: Brightness init and tick
  * 
  */
void transition_BrightInit();
void transition_BrightTick();


/*==============================================================================================================
 || Screens
 ===============================================================================================================*/

void screens_ScrollRob();

/**
 * @brief Screen Loading
 * 
 */
void screens_Loading();


/**
 * @brief Screen Time
 * 
 */
extern char screenTIME_time[];
void screens_Time();

/**
 * @brief Screen Message
 * 
 */
extern char screenMESSAGE_sender[];
extern char screenMESSAGE_msg[];
void screens_Message();

#endif
