#ifndef __ORANGE_ELF_H__
#define __ORANGE_ELF_H__

#include "orange.h"

typedef struct orange_elf_file {
	int			fd;
	int			size;
	Elf32_Ehdr* e;
} orange_elf_file_t;

typedef int (*orange_elf_symbol_hook)(char* name, uint64_t value, void* data);

extern int orange_elf_symbol(struct orange_elf_file* file, orange_elf_symbol_hook hook, void* data);
extern int orange_elf_dump(struct orange_elf_file* file, int (*print)(const char* fmt, ...));
extern struct orange_elf_file* orange_elf_open(char* filename);

extern void orange_elf_close(struct orange_elf_file* file);

#endif /* __ORANGE_ELF_H__ */
