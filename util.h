#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdbool.h>
#include <sys/types.h>

int open_path(char *proc_dir, char *path);
void draw_percbar(char *buf, double frac);
void uid_to_uname(char *name_buf, uid_t uid);
ssize_t lineread(int, char*, size_t);
ssize_t one_lineread(int, char*, ssize_t);
char *next_token(char**, const char*);
bool is_only_numeric(char*);
double kb_to_mb(double);
#endif
