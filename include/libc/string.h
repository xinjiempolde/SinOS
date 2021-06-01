#ifndef STRING_H
#define STRING_H
int strlen(char[]);
int str_list_len(char** argv, int maxLen);
void strclear(char* str);
void strcat(char*, char*);
int strcmp(char* s1, char* s2);
void strcp(char* src, char* dst, unsigned int nbytes);
void reverse(char[]);
void append(char[], char);
void backspace(char*);

int str_split(char* string, char** argv, char toekn);

int uint_to_dec(unsigned int n, char s[]);
int uint_to_hex(unsigned int n, char s[]);
#endif