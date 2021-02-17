#ifndef STRING_H
#define STRING_H
int strlen(char[]);
void strclear(char* str);
void strcat(char*, char*);
int strcmp(char* s1, char* s2);
void reverse(char[]);
void append(char[], char);
void backspace(char*);

int uint_to_dec(unsigned int n, char s[]);
int uint_to_hex(unsigned int n, char s[]);
#endif