#ifndef _SW_LCD_H
#define _SW_LCD_H

#include "state.h"

void setupLCD(void);
void welcomeLCD();
void updateLCD(t_State *state);

#endif  // _SW_LCD_H