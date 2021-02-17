#include <libc/string.h>
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