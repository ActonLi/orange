#ifndef __ORANGE_FILE_H__
#define __ORANGE_FILE_H__

#include "orange.h"

extern FILE* orange_file_fopen_rdonly(const char* pathname);
extern FILE* orange_file_fopen_wronly(const char* pathname);
extern int orange_file_fclose(FILE* fp);
extern int orange_file_fread_line(char* line, int size, FILE* fp);
extern int orange_file_fwrite_line(char* line, FILE* fp);

extern int orange_file_create(const char* pathname, mode_t mode);
extern int orange_file_open(const char* pathname, int flags);
extern int orange_file_open_rdonly(const char* pathname);
extern int orange_file_open_wronly(const char* pathname);
extern int orange_file_close(int fd);
extern int orange_file_read(int fd, void* buf, unsigned int size);
extern int orange_file_write(int fd, void* buf, unsigned int size);
extern int orange_file_get_size(int fd, unsigned int* size);
/*
 * orange_file_read_line():
 * return value: If successful, the number of bytes
 *      of a line actually read is returned.
 * If a 0 is returned, it indicates a blank line.
 * And if error, a -1 is returned.
 * */
extern int orange_file_read_line(int fd, void* buf, unsigned int size);
/*
 * orange_file_write_line():
 * return value: If successful, a 0 is returned.
 * Otherwise,a -1 is returned and the
 *      global variable errno is set to indicate the error.
 * */
extern int orange_file_write_line(int fd, void* buf, unsigned int size);

extern int orange_file_remove(const char* pathname);

#endif /* __ORANGE_FILE_H__ */
