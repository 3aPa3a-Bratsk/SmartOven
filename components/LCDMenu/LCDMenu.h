#ifndef LCDMENU_H
#define LCDMENU_H

#include "LiquidCrystal.h"

class LCDMenu {
public:
    LCDMenu(LiquidCrystal* lcd);
    void setup_menu(const char** items, int count);
    void navigate(int direction);
    int get_current_index();
    const char* get_current_item();
    
private:
    LiquidCrystal* _lcd;
    const char** _items;
    int _item_count;
    int _current_index;
    void display_menu();
};

#endif
