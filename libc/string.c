#include <libc/string.h>
#include <gui/vga.h>
#include <mm/memory.h>
#include <libc/mem.h>
static MemMan* memMan = (MemMan*)MEM_MAN_ADDR;
/**
 * unsigned int converts to decimal number
 * return the number of digits
 */
int uint_to_dec(unsigned int n, char s[]) {
    int i = 0;
    if (n == 0) {
        s[i++] = '0';
    }

    while (n != 0) {
        s[i++] = n % 10 + '0';
        n = n / 10; 
    }
    s[i] = '\0';

    // reverse
    reverse(s);
    return i;
}

/**
 * unsigned int converts to hex number
 * return the number of digits
 */
int uint_to_hex(unsigned int n, char s[]) {
    int i = 0;
    char hexChar[16] = {'0', '1','2','3', '4','5','6', '7','8','9', 'a','b','c', 'd','e','f'};

    if (n == 0) {
        s[i++] = '0';
    }

    while (n != 0) {
        s[i++] = hexChar[n % 16];
        n = n / 16;
    }
    s[i++] = 'x';
    s[i++] = '0';
    s[i] = '\0';

    // reverse
    reverse(s);
    return i;
}

int strlen(char* s) {
    if (s == 0) return 0; // avoid s is null pointer

    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    return i;
}

/**
 * 功能：返回字符串数组的长度
 */
int str_list_len(char** argv, int maxLen) {
    int i;
    for (i = 0; i < maxLen; i++) {
        if (argv[i] == NULL) {
            break;
        }
    }
    return i;
}

void strclear(char str[]) {
    str[0] = '\0';
}

int strcmp(char* s1, char* s2) {
    int s1Len = strlen(s1);
    int s2Len = strlen(s2);
    int i;
    if (s1Len != s2Len) return 1;
    for (i = 0; i < s1Len; i++) {
        if (s1[i] != s2[i]) return 1;
    }
    return 0;
}

void reverse(char* s) {
    int begin = 0, end = strlen(s) - 1;
    char c;
    while (begin < end) {
        c = s[begin];
        s[begin] = s[end];
        s[end] = c;
        
        begin++;
        end--;
    }
    s[strlen(s)] = '\0';
}

void append(char* s, char c) {
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    s[i] = c;
    s[i+1] = '\0';
}

void backspace(char* s) {
    int i = 0;
    while (s[i] != '\0') {
        i++;
    }
    s[i-1] = '\0';
}

void strcat(char* dst, char* src) {
    int i = 0;
    while (dst[i] != '\0') {
        i++;
    }
    int j = 0;
    while (src[j] != '\0') {
        dst[i++] = src[j++];
    }
    dst[i] = '\0';
}

/* 有时间需要改进 */
void strcp(char* src, char* dst, unsigned int nbytes) {
    unsigned int i;
    for (i = 0; i < nbytes; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';

}

/* 按指定字符分隔字符串，结果保存到argv中，返回子串数量 */
int str_split(char* string, char** argv, char token) {
    int argc = 0; // 子串的数量
    char* substring = (char*)mem_alloc_4k(memMan, sizeof(char)*(strlen(string)+1)); // 子串
    memory_copy((uint8_t*)string, (uint8_t*)substring, strlen(string)+1);
    while (*substring) {

        /* 跳过字符串中连续token */
        while (*substring == token) {
            substring++;
        }

        argv[argc] = substring;

        if (*substring == '\"') { // 保护"this is a test"双引号中的空格分隔
            substring++;
            while (*substring != '\"') {
                substring++;
            }
            substring++;
            *substring = '\0';
            substring++;
            argc++;
            continue;
        }

        while (*substring != token && *substring != '\0') {
            substring++;    // 寻找下一个token
        }

        if (*substring) { // 如果没有结束，不是结尾
            *substring = '\0';
            substring++;
        } else {
            *substring = '\0'; // 是末尾了，只需要添\0, 不需要++，否则会越界
        }

        argc++;
    }

    //mem_free_4k(memMan, (uint32_t)substring, sizeof(char)*(strlen(string)+1));
    return argc;
}