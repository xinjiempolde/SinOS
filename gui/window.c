#include <gui/window.h>
#include <gui/vga.h>
void init_window_buf(uint8_t* buf, int weight, int height, char* title) {
	/* why static? we may use this fuction in many times */
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
    };
    int i, j;
	uint8_t c; // color
	fill_rect(buf, weight, 0, 0, weight, 1, BRIGHT_GRAY);
	fill_rect(buf, weight, 1, 1, weight - 2, 1, WHITE);
	fill_rect(buf, weight, 0, 0, 1, height, BRIGHT_GRAY);
	fill_rect(buf, weight, 1, 1, 1, height - 2, WHITE);
	fill_rect(buf, weight, weight - 2, 1, 1, height - 2, DARK_GRAY);
	fill_rect(buf, weight, weight - 1, 0, 1, height, BLACK);
	fill_rect(buf, weight, 2, 2, weight - 4, height - 4, BRIGHT_GRAY);
	fill_rect(buf, weight, 3, 3, weight - 6, 18, DARK_BLUE);
	fill_rect(buf, weight, 1, height - 2, weight - 2, 1, DARK_GRAY);
	fill_rect(buf, weight, 0, height - 1, weight, 1, BLACK);

	put_string(buf,weight, 10, 3, title, WHITE);
	/* draw close btn */
    for (i = 0; i < 14; i++) {
        for (j = 0; j < 16; j++) {
            if (closebtn[i][j] == 'O') {
                c = WHITE;
            } else if (closebtn[i][j] == 'Q') {
				c = BRIGHT_GRAY;
			} else if (closebtn[i][j] == '@') {
				c = BLACK;
			} else if (closebtn[i][j] == '$') {
				c = DARK_GRAY;
			}
			buf[(5+i)*weight + (weight-21+j)] = c;
        }
    }
}