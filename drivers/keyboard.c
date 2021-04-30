#include <drivers/keyboard.h>
#include <cpu/isr.h>
#include <libc/function.h>
#include <libc/string.h>
#include <drivers/ports.h>
#include <gui/vga.h>
#include <cpu/type.h>
/* 103 keyboard map table */
static char* shirt_table[] = {  "ESC", "!", "@", "#", "$", 
                                "%", "^", "&", "*", "(", 
                                ")", "_", "+", "BackSpace", "Tab",
                                "Q", "W", "E", "R", "T",
                                "Y", "U", "I", "O", "P",
                                "{", "}", "ENTER", "Ctrl", "A",
                                "S", "D", "F", "G", "H",
                                "J", "K", "L", ":", "\"",
                                "`", "lShift", "\\", "Z",
                                "X", "C", "V", "B", "N", "M",
                                "<", ">", "?", "rShift", "None",
                                "LAlt", " ", "CapsLock", "F1", "F2", 
                                "F3", "F4", "F5", "F6", "F7",
                                "F8", "F9", "F10", "F11", "F12"};

static char* scan_table[] = {  "ESC", "1", "2", "3", "4", 
                                "5", "6", "7", "8", "9", 
                                "0", "-", "=", "BackSpace", "Tab",
                                "q", "w", "e", "r", "t",
                                "y", "u", "i", "o", "p",
                                "[", "]", "ENTER", "Ctrl", "a",
                                "s", "d", "f", "g", "h",
                                "j", "k", "l", ";", "'",
                                "`", "lShift", "\\", "z",
                                "x", "c", "v", "b", "n", "m",
                                ",", ".", "/", "rShift", "None",
                                "LAlt", " ", "CapsLock", "F1", "F2", 
                                "F3", "F4", "F5", "F6", "F7",
                                "F8", "F9", "F10", "F11", "F12"};
char key_buff[256] = {'\0'};
static bool isShfit = FALSE;
static bool isCapsLock = FALSE;

static void keyboard_callback(registers_t r) {
    // read scan code
    char* str;
    uint8_t scan_code = port_byte_in(KEYBOARD_PORT);
    if (scan_code == KEY_ENTER) {
        append(key_buff, KEY_ENTER);
    } else if (scan_code == KEY_BACKSPACE) {
        append(key_buff, KEY_BACKSPACE);
    /* shift down */
    } else if (scan_code == 0x2a || scan_code == 0x36) {
        isShfit = TRUE;
    /* shift up */
    } else if (scan_code == 0xaa || scan_code == 0xb6) {
        isShfit = FALSE;
    /* CapsLock down */
    } else if (scan_code == KEY_CAPSLOCK) {
        isCapsLock ^= 0x1;
    } else if (scan_code == KEY_ESC) { 
        append(key_buff, KEY_ESC);
    } else if (scan_code < KEY_MAX){
        if (isShfit ^ isCapsLock) {
            str = shirt_table[scan_code - 1];
        } else {
            str = scan_table[scan_code - 1];
        }
        strcat(key_buff, str);
    }
    UNUSED(r);

}


void init_keyboard() {
    add_interrupt_handler(IRQ1, keyboard_callback);
}