#include <gui/console.h>
#include <gui/vga.h>
#include <gui/layer.h>
#include <libc/string.h>
#include <libc/mem.h>
#include <cpu/timer.h>
#include <mm/memory.h>
#include <drivers/keyboard.h>
#include <fs/fs.h>
#include <kernel/pci.h>
#include <gui/gedit.h>

extern LayerManager* layman;
extern char key_buff[256];
Layer* console_layer;
bool readable = TRUE;
static int cursorX = DFT_CSL_BOR + 2*CHAR_W;
static int cursorY = DFT_CSL_TIT_H;
static MemMan* memMan = (MemMan*)MEM_MAN_ADDR;
extern dir_entry cur_dir; // 当前所在的目录的目录项
extern struct path_node path_linked_list; // 头指针
extern struct path_node* path_tail; // 尾指针
extern char content[DFT_EDIT_CON_H][DFT_EDIT_CON_W]; // 用于保存每一行的数据

void init_console_buf(uint8_t* buf, int weight, int height) {
    init_window_buf(buf, weight, height, "console");
    fill_rect(buf, weight, DFT_CSL_BOR, DFT_CSL_TIT_H, weight - 2 * DFT_CSL_BOR, height - (DFT_CSL_TIT_H+DFT_CSL_BOR), BLACK);    
    put_char(buf, DFT_CSL_W, DFT_CSL_BOR, DFT_CSL_TIT_H, '/', BRIGHT_RED);
    put_char(buf, DFT_CSL_W, DFT_CSL_BOR + CHAR_W, DFT_CSL_TIT_H, '#', BRIGHT_GREEN);
    repaint_single_layer(console_layer, DFT_CSL_BOR, DFT_CSL_TIT_H, 4*CHAR_W, CHAR_H);

}

/**
 * new console line
 * return the cursor's y coordinate
 */
int console_newline(Layer* layer, int cursorY) {
    if (cursorY < DFT_CSL_H - DFT_CSL_BOR - CHAR_H) {
        cursorY += CHAR_H;
        return cursorY;
    } else { // should scroll window
        int i, j, k;

        // copy the contents of the next line to the previous line
        for (i = 0; i < DFT_CSL_CON_H - 1; i++) {
            for (j = 0; j < CHAR_H; j++) {
                for (k = DFT_CSL_BOR; k < DFT_CSL_W - DFT_CSL_BOR; k++) {
                    layer->buf[(DFT_CSL_TIT_H+CHAR_H*i+j)*DFT_CSL_W + k] = \
                    layer->buf[(DFT_CSL_TIT_H+CHAR_H*(i+1)+j)*DFT_CSL_W + k];
                }
            }
        }

        /* cover the last line */
        fill_rect(layer->buf, layer->weight, DFT_CSL_BOR, DFT_CSL_H-DFT_CSL_BOR-CHAR_H, CSL_CON_H_SIZE, CHAR_H, BLACK);
        // repaint content area (i.e. black color area)
        repaint_single_layer(layer, DFT_CSL_BOR, DFT_CSL_TIT_H, CSL_CON_H_SIZE, CSL_CON_V_SIZE);
        return DFT_CSL_H - DFT_CSL_BOR - CHAR_H;
    }

}

void console_clear(Layer* console_layer) {
    put_rect_refresh(console_layer, DFT_CSL_BOR, DFT_CSL_TIT_H, CSL_CON_H_SIZE, CSL_CON_V_SIZE, BLACK);
}

void console_run() {

    int kb_idx = 0; // keyboard buffuer index
    timer_t* cursorTimer = set_timer(50);
    int cursorColor = WHITE;
    MemMan* memMan = (MemMan*)MEM_MAN_ADDR;

    char cmd[256] = {'\0'}; // command
    char full_path[64]; // 路径字符串
    uint8_t* console_buf = (uint8_t*)mem_alloc_4k(memMan, DFT_CSL_H*DFT_CSL_W);
    init_console_buf(console_buf, DFT_CSL_W, DFT_CSL_H);
    console_layer = alloc_layer(layman, console_buf, DFT_CSL_H, DFT_CSL_W, 6);
    move_layer(console_layer, 200, 200);
    while (1) {
        if (!readable) {
            continue;
        }
        /* keyboard buff is not null */
        if (key_buff[kb_idx] != '\0') {
            if (key_buff[kb_idx] == KEY_BACKSPACE) {
                if (cursorX > DFT_CSL_BOR + 2 * CHAR_W + CHAR_W * strlen(full_path)) { // can backspace
                    cursorX -= CHAR_W;
                    backspace(cmd);
                    put_rect_refresh(console_layer, cursorX, cursorY, 2*CHAR_W, CHAR_H, BLACK);
                }
            } else if (key_buff[kb_idx] == KEY_ENTER) {
                put_rect_refresh(console_layer, cursorX, cursorY, CHAR_W, CHAR_H, BLACK);
                cursorX = DFT_CSL_BOR;
                cursorY = console_newline(console_layer, cursorY);

                parse_cmd(cmd); // 解析命令 

                gen_path(full_path); // 生成当前的绝对路径

                put_str_refresh(console_layer, cursorX, cursorY, full_path, BRIGHT_RED);
                cursorX += CHAR_W * strlen(full_path);
                put_char_refresh(console_layer, cursorX, cursorY, '#', BRIGHT_GREEN);                    
                cursorX += 2*CHAR_W;


                strclear(cmd);
                strclear(key_buff);
                kb_idx = -1; // why not 0? kb_idx++ will add 1, so don't worry

            } else {
                append(cmd, key_buff[kb_idx]);        
                fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, CHAR_W, CHAR_H, BLACK);
                put_char(console_buf, DFT_CSL_W, cursorX, cursorY, key_buff[kb_idx], WHITE);
                repaint_single_layer(console_layer, cursorX, cursorY, CHAR_W, CHAR_H);
                cursorX += CHAR_W;
            }
            kb_idx++;
        }
        /* cursor blinks */
        if (cursorTimer->timeoutFlags == TIME_OUT) {
            if (cursorColor == WHITE) {
                cursorColor = BLACK;
            } else {
                cursorColor = WHITE;
            }
            restart_timer(cursorTimer);
        }
        fill_rect(console_buf, DFT_CSL_W, cursorX, cursorY, 8, 16, cursorColor);
        repaint_single_layer(console_layer, cursorX, cursorY, CHAR_W, CHAR_H);
    }
}

void parse_cmd(char* cmd_str) {
    int i;
    char* argv[5] = {NULL};
    clear_argv(argv);
    str_split(cmd_str, (char**)argv, ' ');

    /* memory command */
    if (strcmp(argv[0], "mem") == 0) {
        console_printfn("total:    %dMB", 128);
        console_printfn("free:     %dMB", mem_total(memMan) / (1024*1024));
    /* command not found */
    } else if (strcmp(argv[0], "clear") == 0) {
        console_clear(console_layer);
        cursorX = DFT_CSL_BOR;
        cursorY = DFT_CSL_TIT_H;
    } else if (strcmp(argv[0],"ls") == 0) {
        dir_entry file_array[141]; // 一个路径下最多13+128个文件，文件名最长15个字符
        int file_num = read_all_files(&cur_dir, file_array);

        printf("file_num: %d", file_num);
        for (i = 0; i < file_num; i++) {
            if (file_array[i].f_type == FT_DIRECOTRY) {
                console_print(BRIGHT_BLUE, "%s ", file_array[i].filename);
            } else if (file_array[i].f_type == FT_FILE) {
                console_print(DARK_GREEN, "%s ", file_array[i].filename);
            }
        }
        cursorX = DFT_CSL_BOR;
        cursorY = console_newline(console_layer, cursorY);

    } else if (strcmp(argv[0], "lspci") == 0) {
        init_pcilist();
    } else if (strcmp(argv[0], "cat") == 0){ // 需要进行修改
        uint8_t sector[512] = {0};
        int status = read_file(argv[1], sector, 512);
        if (status == FILE_BUT_DIR) {
            console_printfn("%s is a directory!", argv[1]);
        } else if (status == FAIL) {
            console_printfn("%s doesn't exist!", argv[1]);
        } else {
            console_printfn("%s", (char*)sector);
        }
    } else if (strcmp(argv[0], "mkdir") == 0) {
        int status = create_dir(argv[1], cur_dir.i_no);
        if (status == FAIL) {
            console_printfn("%s has exist", argv[1]);
        }
    } else if (strcmp(argv[0], "cd") == 0) {
        if (strcmp(argv[1], "../") == 0) { // 返回上一级目录
            switch_backward();
        } else {
            dir_entry* dir = parse_full_path(argv[1]);
            if (dir == NULL) {
                console_printfn("directory %s doesn't exist", argv[1]);
            } else if (dir->f_type != FT_DIRECOTRY) {
                console_printfn("%s is not a directory!", argv[1]);
            } else {
                switch_full_path(argv[1]);
            }
            //mem_free_4k(memMan, (uint32_t)dir, sizeof(dir_entry));
        }

    } else if (strcmp(argv[0], "echo") == 0) {
        create_file(cur_dir.i_no ,argv[3], (uint8_t*)(argv[1]), strlen(argv[1]));
    } else if (strcmp(argv[0], "rm") == 0) {
        rm_dir_by_name(argv[1], cur_dir.i_no, FT_FILE);
    } else if (strcmp(argv[0], "rmdir") == 0) {
        rm_dir_by_name(argv[1], cur_dir.i_no, FT_DIRECOTRY);
    } else if (strcmp(argv[0], "gedit") == 0) {
        cmd_gedit(argv);
    } else if (strcmp(argv[0], "format") == 0) {
        init_fs(TRUE);
    } else if (strcmp(argv[0], "ln") == 0) {
        dir_entry* dir = parse_full_path(argv[1]);
        if (dir == NULL) {
            console_printfn("%s doesn't exists", argv[1]);
        }
        printf("%s", dir->filename);
    } else {
        put_str_refresh(console_layer, cursorX, cursorY, "command not found", WHITE);
        cursorY = console_newline(console_layer, cursorY);
    }

}

void console_printfn(const char* fmt, ...) {
    char s[1024] = {0};
    char* argv[20] = {NULL};
    int i, len;

    va_list args = (va_list)((char*)(&fmt) + 4);     // get the next parameter from stack, see c call conventions
    vsprintf(s, fmt, args);

    str_split(s, argv, ASCII_NL);
    len = len_argv(argv);
    for (i = 0; i < len; i++) {
        put_str_refresh(console_layer, cursorX, cursorY, argv[i], WHITE);
        cursorY = console_newline(console_layer, cursorY);
    }

}

void console_print(int color, const char* fmt, ...) {
    char s[64];

    va_list args = (va_list)((char*)(&fmt) + 4);     // get the next parameter from stack, see c call conventions
    vsprintf(s, fmt, args);

    if (cursorX + strlen(s) * CHAR_W >= CSL_CON_H_SIZE) { // 将要输出的内容将超过console
        cursorX = DFT_CSL_BOR;
        cursorY = console_newline(console_layer, cursorY);
    }
    put_str_refresh(console_layer, cursorX, cursorY, s, color);
    cursorX += strlen(s) * CHAR_W;
}

/* 根据路径链表生成当前的绝对路径 */
void gen_path(char* path) {
    strclear(path); // 清空路径
    append(path, '/'); // 根目录后面不需要再添加/
    struct path_node* p = path_linked_list.next;
    while (p) {
        strcat(path, p->dir_name);
        append(path, '/');
        p = p->next;
    }
}

int len_argv(char** argv) {
    int i;
    for (i = 0; i < 20; i++) {
        if (argv[i] == 0) {
            break;
        }
    }
    return i;
}

void clear_argv(char** argv) {
    int i = 5;
    while (i--) {
        argv[i] = 0;
    }
}

void cmd_gedit(char** argv) {
    char buf[1024] = {0};

    if (len_argv(argv) != 2) {
        console_printfn("gedit must have 2 arguements!");
        return;
    }

    if (search_dir_by_id(argv[1], cur_dir.i_no) != NULL) {
        console_printfn("%s has exists!", argv[1]);
        return;
    }

    readable = FALSE;
    int line_num = gedit_run();
    transfer((char*)content, DFT_EDIT_CON_W, line_num, buf);

    create_file(cur_dir.i_no, argv[1], (uint8_t*)buf, strlen(buf));
}