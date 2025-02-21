#include "LCDMenu.h"
#include <string.h>

LCDMenu::LCDMenu(LiquidCrystal* lcd) {
    _lcd = lcd;
    _items = NULL;
    _item_count = 0;
    _current_index = 0;
}

void LCDMenu::setup_menu(const char** items, int count) {
    _items = items;
    _item_count = count;
    _current_index = 0;
    display_menu();
}

void LCDMenu::navigate(int direction) {
    if (direction > 0) {
        _current_index = (_current_index + 1) % _item_count;
    } else if (direction < 0) {
        _current_index = (_current_index - 1 + _item_count) % _item_count;
    }
    display_menu();
}

int LCDMenu::get_current_index() {
    return _current_index;
}

const char* LCDMenu::get_current_item() {
    if (_items && _current_index < _item_count) {
        return _items[_current_index];
    }
    return "";
}

void LCDMenu::display_menu() {
    _lcd->clear();
    if (_items && _current_index < _item_count) {
        _lcd->setCursor(0, 0);
        _lcd->print("> ");
        _lcd->print(_items[_current_index]);
        
        if (_current_index + 1 < _item_count) {
            _lcd->setCursor(0, 1);
            _lcd->print("  ");
            _lcd->print(_items[_current_index + 1]);
        }
    }
}
