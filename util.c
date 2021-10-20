#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "util.h"
#include "logger.h"
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

/**
 * Read a line in a file
 *
 * @param fd file descriptor
 * @param buf buffer to write into
 * @param sz size of buffer
 *
 * @return count of characters read
 */ 
ssize_t lineread(int fd, char *buf, size_t sz)
{
  size_t count = 0;
  while (count < sz)
  {
    char c;
    ssize_t read_sz = read(fd, &c, 1);
    if (read_sz <= 0)
    {
      return read_sz;
    }
    else
    {
      buf[count] = c;
      count += read_sz;
      if (c == '\n')
      {
        return count;
      }
    }
  }

  return count;
}

/** 
 * Read a single line
 * 
 * @param fd file descriptor to read from
 * @param buf buffer to write into
 * @param sz buffer size to read from
 *
 * @return read / written size
 */ 
ssize_t one_lineread(int fd, char *buf, ssize_t sz){
	ssize_t read_sz = lineread(fd, buf, sz);
	if(read_sz <= 0){
		return read_sz;
	}

	buf[read_sz - 1] = '\0';	
	return read_sz;
}

/**
 * Opens a particular path under the proc file system and returns a file descriptor to the opened file.
 *
 * @param proc_dir location of the proc file system
 * @param path subdirectory under procfs to open
 *
 * @return file descriptor of the opened path or -1 on failure
 */
int open_path(char *proc_dir, char *path){
  if(proc_dir == NULL || path == NULL){
	  errno = EINVAL;
	  return -1;
  }
  size_t str_size = strlen(proc_dir) + strlen(path) + 2;
  char *full_path = malloc(str_size);
  if(full_path == NULL){
	  return -1;
  }
  snprintf(full_path, str_size, "%s/%s", proc_dir, path);

 // LOG("Opening path: %s\n", full_path);
  int fd = open(full_path, O_RDONLY);
  free(full_path);
  return fd;
}

/**
 * Draw the percentage bar
 * 
 * @param buf buffer to draw into
 * @param frac fraction to use to draw
 */ 
void draw_percbar(char *buf, double frac) {
	if(isnan(frac) || frac == -INFINITY || frac == 0.0 || frac < 0.00000){
		strcpy(buf, "[--------------------] 0.0%");
		return;
	}
	else if(frac >= 1 || frac == INFINITY){
		strcpy(buf, "[####################] 100.0%");
		return;
	}
	else {
		double percText = frac * 100;
		double perc = round(frac * 100);
		strcpy(buf, "");
		strcat(buf, "[");
		for(int i=0; i<(int)(perc / 5); i++){
			strcat(buf, "#");
		}
		for(int j=(int)(perc / 5); j<20; j++){
			strcat(buf, "-");
		}
		strcat(buf, "] ");
		sprintf(buf + strlen(buf), "%.1f", percText); 
		strcat(buf, "%");
	}
}

/**
 * Convert a UID to a username
 * 
 * @param name_buf name buffer to write into
 * @param uid uid to read
 */
void uid_to_uname(char *name_buf, uid_t uid)
{
//	LOGP("Get the username from a given userid in passwd with status in proc with certain files?");
    int fd = open_path("/etc", "passwd");
    if(fd <= 0){
	    return;
    }
    char line[300];
    one_lineread(fd, line, 200);
    while(1){
	    
	   // LOG("id: %s\n", line);
	    char *next_tok = line;
	    next_token(&next_tok, ":");
	    next_token(&next_tok, ":");
	    if(atoi(next_tok) == uid){
		    break;
	    }
	    if(one_lineread(fd, line, 200) == 0){
		    sprintf(name_buf, "%d", uid);
		    return;
	    }
    }
    LOG("User: %s\n", line);
    char name[16];
    strcpy(name, line);
    name[15] = '\0';
    strcpy(name_buf, name);
}


/**
 * Retrieves the next token from a string.
 *
 
 * @param str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * @param delim the set of characters to use as delimiters
 *
 * @return char pointer to the next token in the string.
 *
 * https://groups.google.com/forum/message/raw?msg=comp.lang.c/ff0xFqRPH_Y/Cen0mgciXn8J
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  == 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

/**
 * Check if a string is numeric
 *
 * @param str string to check
 *
 * @return true if numeric, false if not
 */ 
bool is_only_numeric(char* str){
	return atoi(str) > 0;
}


/**
 * Convert kb to mb
 *
 * @param kb kb amount
 * 
 * @return mb equivalent
 */ 
double kb_to_mb(double kb){
	return kb / 1024 / 1024;
}
