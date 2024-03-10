#ifndef _SW_LCD_H
#define _SW_LCD_H

#include <Adafruit_SSD1306.h>

#include "state.h"

namespace UI {
    enum t_UIScreen : uint8_t {
        WELCOME,
        RoasterState,
        TOTAL_UI_SCREENS
    };


    class _UIScreen {
        protected:
            _UIScreen(t_UIScreen id): id(id) {};
        public:
            const t_UIScreen id;
            virtual t_UIScreen getNextScreen(t_State *state) {
                return this->id;
            }
            virtual void draw(t_State *state) {};
    };


    class UI {
        private:
            t_UIScreen currentScreen = WELCOME;
            Adafruit_SSD1306 display;
            //_UIScreen screens[TOTAL_UI_SCREENS];
        public:
            UI();
            void begin();
            void update(t_State *state);
    };
}

#endif  // _SW_LCD_H