#include <drivers/mouse.h>
#include <cpu/isr.h>
#include <libc/function.h>
#include <libc/string.h>
#include <libc/mem.h>
#include <drivers/ports.h>
#include <gui/vga.h>
#include <gui/layer.h>
static int offset = 0;
static char buffer[3];

typedef struct {
    int x;
    int y;
} mouse_info;

static mouse_info m_info;
static BOOTINFO* bootInfo = (BOOTINFO*)BOOTINFO_ADDR;

/* definition in kernel.c */
extern LayerManager* layman;
extern Layer* mouse_layer;
extern Layer* window_layer;
int write_ready() {
    if ((port_byte_in(MOUSE_STATE_PORT) & 0x02) == 0) {
        return 1;
    }
    return 0;
}

int read_ready() {
    if ((port_byte_in(MOUSE_STATE_PORT) & 0x01) == 1) {
        return 1;
    }
    return 0;
}

/**
 * initlate mouse(the second PS/2 device), see details below:
 * https://wiki.osdev.org/%228042%22_PS/2_Controller(the most useful)
 * https://wiki.osdev.org/Mouse_Input#PS2_Mouse_Commands
 */
void enable_mouse() {
    /* write Controller Configuration Byte to Controller */
    while (!write_ready());
    port_byte_out(MOUSE_CMD_PORT, 0x60);
    while (!write_ready());
    port_byte_out(MOUSE_DATA_PORT, 0x43);

    /**
     * two bytes command, Enable Mouse Packet Streaming
     */
    while (!write_ready());
    port_byte_out(MOUSE_CMD_PORT, 0xd4);
    while (!write_ready());
    port_byte_out(MOUSE_DATA_PORT, 0xf4);
    
    /* Read 0xFA ACK */
    while (!read_ready());
    port_byte_in(MOUSE_DATA_PORT);

}

/**
 * Mouse Handler
 * ATTENTION：mouse will don't generate IRQ12 if output buffer is full
 */
static void mouse_callback(registers_t r) {
    while (!read_ready());
    buffer[offset] = port_byte_in(MOUSE_DATA_PORT);
    offset = (offset + 1) % 3;
    /* data reception completed */
    if (offset == 0) {
        //fill_rect(m_info.x, m_info.y, 16, 16, LIGHT_BRIGHT_BLUE);

        /* I don't understand why I can't change the bottom of 8 lines if
           I don't add this code.
         * new here, it's all qemu's faluts!!!
         */
        //fill_rect(0, 0, 1, 1, LIGHT_BRIGHT_BLUE);

        m_info.x += buffer[1];
        m_info.y -= buffer[2];
        /* why -1 instead of -16? hide the mouse cursor in right */
        if (m_info.x < 0) m_info.x = 0;
        if (m_info.x >= bootInfo->screen_w-1) m_info.x = bootInfo->screen_w-1;
        if (m_info.y < 0) m_info.y = 0;
        if (m_info.y >= bootInfo->screen_h-1) m_info.y = bootInfo->screen_h-1;
        move_layer(mouse_layer, m_info.x, m_info.y);
        if (buffer[0] & LEFT_BTN_ON) {
            move_layer(window_layer, m_info.x, m_info.y);
        }
    }
    UNUSED(r);
}

void init_mouse() {
    enable_mouse();
    add_interrupt_handler(IRQ12, mouse_callback);
}

void init_mouse_cursor(uint8_t* mouse_cursor_buff) {
    static char cursor[16][16] = {
        "**************..",
        "*OOOOOOOOOOO*...",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO**OOO*.....",
        "*OOO*..*OOO*....",
        "*OO*....*OOO*...",
        "*O*......*OOO*..",
        "**........*OOO*.",
        "*..........*OOO*",
        "............*OO*",
        ".............***"
    };

    int i, j;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            if (cursor[i][j] == '*') {
                mouse_cursor_buff[i*16 + j] = BLACK;
            } else if (cursor[i][j] == 'O') {
                mouse_cursor_buff[i*16 + j] = WHITE;
            } else if (cursor[i][j] == '.') {
                mouse_cursor_buff[i*16 + j] = TRANSPARENT;
            }
        }
    }
}

