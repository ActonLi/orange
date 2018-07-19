#include "orange_elf.h"
#include "orange.h"
#include "orange_endian.h"

#if 0
#define DEBUGP printf
#else
#define DEBUGP(format, args...)
#endif

#define orange_elf_get_addr orange_elf_get_quad
#define orange_elf_get_off orange_elf_get_quad
#define orange_elf_get_size orange_elf_get_quad

/* Note header in a PT_NOTE section */
typedef struct elf_note {
	Elf32_Word n_namesz; /* Name size */
	Elf32_Word n_descsz; /* Content size */
	Elf32_Word n_type;   /* Content type */
} Elf_Note;

enum elf_member {
	D_TAG = 1,
	D_PTR,
	D_VAL,

	E_CLASS,
	E_DATA,
	E_OSABI,
	E_TYPE,
	E_MACHINE,
	E_VERSION,
	E_ENTRY,
	E_PHOFF,
	E_SHOFF,
	E_FLAGS,
	E_EHSIZE,
	E_PHENTSIZE,
	E_PHNUM,
	E_SHENTSIZE,
	E_SHNUM,
	E_SHSTRNDX,

	N_NAMESZ,
	N_DESCSZ,
	N_TYPE,

	P_TYPE,
	P_OFFSET,
	P_VADDR,
	P_PADDR,
	P_FILESZ,
	P_MEMSZ,
	P_FLAGS,
	P_ALIGN,

	SH_NAME,
	SH_TYPE,
	SH_FLAGS,
	SH_ADDR,
	SH_OFFSET,
	SH_SIZE,
	SH_LINK,
	SH_INFO,
	SH_ADDRALIGN,
	SH_ENTSIZE,

	ST_NAME,
	ST_VALUE,
	ST_SIZE,
	ST_INFO,
	ST_SHNDX,

	R_OFFSET,
	R_INFO,

	RA_OFFSET,
	RA_INFO,
	RA_ADDEND
};

typedef enum elf_member elf_member_t;

int elf32_offsets[] = {0,

					   offsetof(Elf32_Dyn, d_tag),
					   offsetof(Elf32_Dyn, d_un.d_ptr),
					   offsetof(Elf32_Dyn, d_un.d_val),

					   offsetof(Elf32_Ehdr, e_ident[EI_CLASS]),
					   offsetof(Elf32_Ehdr, e_ident[EI_DATA]),
					   offsetof(Elf32_Ehdr, e_ident[EI_OSABI]),
					   offsetof(Elf32_Ehdr, e_type),
					   offsetof(Elf32_Ehdr, e_machine),
					   offsetof(Elf32_Ehdr, e_version),
					   offsetof(Elf32_Ehdr, e_entry),
					   offsetof(Elf32_Ehdr, e_phoff),
					   offsetof(Elf32_Ehdr, e_shoff),
					   offsetof(Elf32_Ehdr, e_flags),
					   offsetof(Elf32_Ehdr, e_ehsize),
					   offsetof(Elf32_Ehdr, e_phentsize),
					   offsetof(Elf32_Ehdr, e_phnum),
					   offsetof(Elf32_Ehdr, e_shentsize),
					   offsetof(Elf32_Ehdr, e_shnum),
					   offsetof(Elf32_Ehdr, e_shstrndx),

					   offsetof(Elf_Note, n_namesz),
					   offsetof(Elf_Note, n_descsz),
					   offsetof(Elf_Note, n_type),

					   offsetof(Elf32_Phdr, p_type),
					   offsetof(Elf32_Phdr, p_offset),
					   offsetof(Elf32_Phdr, p_vaddr),
					   offsetof(Elf32_Phdr, p_paddr),
					   offsetof(Elf32_Phdr, p_filesz),
					   offsetof(Elf32_Phdr, p_memsz),
					   offsetof(Elf32_Phdr, p_flags),
					   offsetof(Elf32_Phdr, p_align),

					   offsetof(Elf32_Shdr, sh_name),
					   offsetof(Elf32_Shdr, sh_type),
					   offsetof(Elf32_Shdr, sh_flags),
					   offsetof(Elf32_Shdr, sh_addr),
					   offsetof(Elf32_Shdr, sh_offset),
					   offsetof(Elf32_Shdr, sh_size),
					   offsetof(Elf32_Shdr, sh_link),
					   offsetof(Elf32_Shdr, sh_info),
					   offsetof(Elf32_Shdr, sh_addralign),
					   offsetof(Elf32_Shdr, sh_entsize),

					   offsetof(Elf32_Sym, st_name),
					   offsetof(Elf32_Sym, st_value),
					   offsetof(Elf32_Sym, st_size),
					   offsetof(Elf32_Sym, st_info),
					   offsetof(Elf32_Sym, st_shndx),

					   offsetof(Elf32_Rel, r_offset),
					   offsetof(Elf32_Rel, r_info),

					   offsetof(Elf32_Rela, r_offset),
					   offsetof(Elf32_Rela, r_info),
					   offsetof(Elf32_Rela, r_addend)};

int elf64_offsets[] = {0,

					   offsetof(Elf64_Dyn, d_tag),
					   offsetof(Elf64_Dyn, d_un.d_ptr),
					   offsetof(Elf64_Dyn, d_un.d_val),

					   offsetof(Elf32_Ehdr, e_ident[EI_CLASS]),
					   offsetof(Elf32_Ehdr, e_ident[EI_DATA]),
					   offsetof(Elf32_Ehdr, e_ident[EI_OSABI]),
					   offsetof(Elf64_Ehdr, e_type),
					   offsetof(Elf64_Ehdr, e_machine),
					   offsetof(Elf64_Ehdr, e_version),
					   offsetof(Elf64_Ehdr, e_entry),
					   offsetof(Elf64_Ehdr, e_phoff),
					   offsetof(Elf64_Ehdr, e_shoff),
					   offsetof(Elf64_Ehdr, e_flags),
					   offsetof(Elf64_Ehdr, e_ehsize),
					   offsetof(Elf64_Ehdr, e_phentsize),
					   offsetof(Elf64_Ehdr, e_phnum),
					   offsetof(Elf64_Ehdr, e_shentsize),
					   offsetof(Elf64_Ehdr, e_shnum),
					   offsetof(Elf64_Ehdr, e_shstrndx),

					   offsetof(Elf_Note, n_namesz),
					   offsetof(Elf_Note, n_descsz),
					   offsetof(Elf_Note, n_type),

					   offsetof(Elf64_Phdr, p_type),
					   offsetof(Elf64_Phdr, p_offset),
					   offsetof(Elf64_Phdr, p_vaddr),
					   offsetof(Elf64_Phdr, p_paddr),
					   offsetof(Elf64_Phdr, p_filesz),
					   offsetof(Elf64_Phdr, p_memsz),
					   offsetof(Elf64_Phdr, p_flags),
					   offsetof(Elf64_Phdr, p_align),

					   offsetof(Elf64_Shdr, sh_name),
					   offsetof(Elf64_Shdr, sh_type),
					   offsetof(Elf64_Shdr, sh_flags),
					   offsetof(Elf64_Shdr, sh_addr),
					   offsetof(Elf64_Shdr, sh_offset),
					   offsetof(Elf64_Shdr, sh_size),
					   offsetof(Elf64_Shdr, sh_link),
					   offsetof(Elf64_Shdr, sh_info),
					   offsetof(Elf64_Shdr, sh_addralign),
					   offsetof(Elf64_Shdr, sh_entsize),

					   offsetof(Elf64_Sym, st_name),
					   offsetof(Elf64_Sym, st_value),
					   offsetof(Elf64_Sym, st_size),
					   offsetof(Elf64_Sym, st_info),
					   offsetof(Elf64_Sym, st_shndx),

					   offsetof(Elf64_Rel, r_offset),
					   offsetof(Elf64_Rel, r_info),

					   offsetof(Elf64_Rela, r_offset),
					   offsetof(Elf64_Rela, r_info),
					   offsetof(Elf64_Rela, r_addend)};

#if 0
/* http://www.sco.com/developers/gabi/latest/ch5.dynamic.html#tag_encodings */
static const char *
d_tags(u_int64_t tag) {
	switch (tag) {
	case 0: return "DT_NULL";
	case 1: return "DT_NEEDED";
	case 2: return "DT_PLTRELSZ";
	case 3: return "DT_PLTGOT";
	case 4: return "DT_HASH";
	case 5: return "DT_STRTAB";
	case 6: return "DT_SYMTAB";
	case 7: return "DT_RELA";
	case 8: return "DT_RELASZ";
	case 9: return "DT_RELAENT";
	case 10: return "DT_STRSZ";
	case 11: return "DT_SYMENT";
	case 12: return "DT_INIT";
	case 13: return "DT_FINI";
	case 14: return "DT_SONAME";
	case 15: return "DT_RPATH";
	case 16: return "DT_SYMBOLIC";
	case 17: return "DT_REL";
	case 18: return "DT_RELSZ";
	case 19: return "DT_RELENT";
	case 20: return "DT_PLTREL";
	case 21: return "DT_DEBUG";
	case 22: return "DT_TEXTREL";
	case 23: return "DT_JMPREL";
	case 24: return "DT_BIND_NOW";
	case 25: return "DT_INIT_ARRAY";
	case 26: return "DT_FINI_ARRAY";
	case 27: return "DT_INIT_ARRAYSZ";
	case 28: return "DT_FINI_ARRAYSZ";
	case 29: return "DT_RUNPATH";
	case 30: return "DT_FLAGS";
	case 32: return "DT_PREINIT_ARRAY"; /* XXX: DT_ENCODING */
	case 33: return "DT_PREINIT_ARRAYSZ";
	/* 0x6000000D - 0x6ffff000 operating system-specific semantics */
	case 0x6ffffdf5: return "DT_GNU_PRELINKED";
	case 0x6ffffdf6: return "DT_GNU_CONFLICTSZ";
	case 0x6ffffdf7: return "DT_GNU_LIBLISTSZ";
	case 0x6ffffdf8: return "DT_SUNW_CHECKSUM";
	case 0x6ffffdf9: return "DT_PLTPADSZ";
	case 0x6ffffdfa: return "DT_MOVEENT";
	case 0x6ffffdfb: return "DT_MOVESZ";
	case 0x6ffffdfc: return "DT_FEATURE";
	case 0x6ffffdfd: return "DT_POSFLAG_1";
	case 0x6ffffdfe: return "DT_SYMINSZ";
	case 0x6ffffdff: return "DT_SYMINENT (DT_VALRNGHI)";
	case 0x6ffffe00: return "DT_ADDRRNGLO";
	case 0x6ffffef8: return "DT_GNU_CONFLICT";
	case 0x6ffffef9: return "DT_GNU_LIBLIST";
	case 0x6ffffefa: return "DT_SUNW_CONFIG";
	case 0x6ffffefb: return "DT_SUNW_DEPAUDIT";
	case 0x6ffffefc: return "DT_SUNW_AUDIT";
	case 0x6ffffefd: return "DT_SUNW_PLTPAD";
	case 0x6ffffefe: return "DT_SUNW_MOVETAB";
	case 0x6ffffeff: return "DT_SYMINFO (DT_ADDRRNGHI)";
	case 0x6ffffff9: return "DT_RELACOUNT";
	case 0x6ffffffa: return "DT_RELCOUNT";
	case 0x6ffffffb: return "DT_FLAGS_1";
	case 0x6ffffffc: return "DT_VERDEF";
	case 0x6ffffffd: return "DT_VERDEFNUM";
	case 0x6ffffffe: return "DT_VERNEED";
	case 0x6fffffff: return "DT_VERNEEDNUM";
	case 0x6ffffff0: return "DT_GNU_VERSYM";
	/* 0x70000000 - 0x7fffffff processor-specific semantics */
	case 0x70000000: return "DT_IA_64_PLT_RESERVE";
	case 0x7ffffffd: return "DT_SUNW_AUXILIARY";
	case 0x7ffffffe: return "DT_SUNW_USED";
	case 0x7fffffff: return "DT_SUNW_FILTER";
	default: return "ERROR: TAG NOT DEFINED";
	}
}

static const char *
e_machines(u_int mach)
{
	static char machdesc[64];

	switch (mach) {
	case EM_NONE:	return "EM_NONE";
	case EM_M32:	return "EM_M32";
	case EM_SPARC:	return "EM_SPARC";
	case EM_386:	return "EM_386";
	case EM_68K:	return "EM_68K";
	case EM_88K:	return "EM_88K";
	case EM_860:	return "EM_860";
	case EM_MIPS:	return "EM_MIPS";
	case EM_PPC:	return "EM_PPC";
	case EM_ARM:	return "EM_ARM";
	case EM_ALPHA:	return "EM_ALPHA (legacy)";
	case EM_SPARCV9:return "EM_SPARCV9";
	case EM_IA_64:	return "EM_IA_64";
	case EM_X86_64:	return "EM_X86_64";
	}
	snprintf(machdesc, sizeof(machdesc),
	    "(unknown machine) -- type 0x%x", mach);
	return (machdesc);
}
#endif

const char* e_types[] = {"ET_NONE", "ET_REL", "ET_EXEC", "ET_DYN", "ET_CORE"};

const char* ei_versions[] = {"EV_NONE", "EV_CURRENT"};

const char* ei_classes[] = {"ELFCLASSNONE", "ELFCLASS32", "ELFCLASS64"};

const char* ei_data[] = {"ELFDATANONE", "ELFDATA2LSB", "ELFDATA2MSB"};

const char* ei_abis[] = {"ELFOSABI_SYSV",	 "ELFOSABI_HPUX", "ELFOSABI_NETBSD",  "ELFOSABI_LINUX", "ELFOSABI_HURD",	"ELFOSABI_86OPEN", "ELFOSABI_SOLARIS",
						 "ELFOSABI_MONTEREY", "ELFOSABI_IRIX", "ELFOSABI_FREEBSD", "ELFOSABI_TRU64", "ELFOSABI_MODESTO", "ELFOSABI_OPENBSD"};

const char* p_types[] = {"PT_NULL", "PT_LOAD", "PT_DYNAMIC", "PT_INTERP", "PT_NOTE", "PT_SHLIB", "PT_PHDR", "PT_TLS"};

const char* p_flags[] = {"", "PF_X", "PF_W", "PF_X|PF_W", "PF_R", "PF_X|PF_R", "PF_W|PF_R", "PF_X|PF_W|PF_R"};

#if 0
/* http://www.sco.com/developers/gabi/latest/ch4.sheader.html#sh_type */
static const char *
sh_types(u_int64_t sht) {
	switch (sht) {
	case 0:	return "SHT_NULL";
	case 1: return "SHT_PROGBITS";
	case 2: return "SHT_SYMTAB";
	case 3: return "SHT_STRTAB";
	case 4: return "SHT_RELA";
	case 5: return "SHT_HASH";
	case 6: return "SHT_DYNAMIC";
	case 7: return "SHT_NOTE";
	case 8: return "SHT_NOBITS";
	case 9: return "SHT_REL";
	case 10: return "SHT_SHLIB";
	case 11: return "SHT_DYNSYM";
	case 14: return "SHT_INIT_ARRAY";
	case 15: return "SHT_FINI_ARRAY";
	case 16: return "SHT_PREINIT_ARRAY";
	case 17: return "SHT_GROUP";
	case 18: return "SHT_SYMTAB_SHNDX";
	/* 0x60000000 - 0x6fffffff operating system-specific semantics */
	case 0x6ffffff0: return "XXX:VERSYM";
	case 0x6ffffff4: return "SHT_SUNW_dof";
	case 0x6ffffff7: return "SHT_GNU_LIBLIST";
	case 0x6ffffffc: return "XXX:VERDEF";
	case 0x6ffffffd: return "SHT_SUNW(GNU)_verdef";
	case 0x6ffffffe: return "SHT_SUNW(GNU)_verneed";
	case 0x6fffffff: return "SHT_SUNW(GNU)_versym";
	/* 0x70000000 - 0x7fffffff processor-specific semantics */
	case 0x70000000: return "SHT_IA_64_EXT";
	case 0x70000001: return "SHT_IA_64_UNWIND";
	case 0x7ffffffd: return "XXX:AUXILIARY";
	case 0x7fffffff: return "XXX:FILTER";
	/* 0x80000000 - 0xffffffff application programs */
	default: return "ERROR: SHT NOT DEFINED";
	}
}
#endif

const char* sh_flags[] = {"",
						  "SHF_WRITE",
						  "SHF_ALLOC",
						  "SHF_WRITE|SHF_ALLOC",
						  "SHF_EXECINSTR",
						  "SHF_WRITE|SHF_EXECINSTR",
						  "SHF_ALLOC|SHF_EXECINSTR",
						  "SHF_WRITE|SHF_ALLOC|SHF_EXECINSTR"};

const char* st_types[] = {"STT_NOTYPE", "STT_OBJECT", "STT_FUNC", "STT_SECTION", "STT_FILE"};

const char* st_bindings[] = {"STB_LOCAL", "STB_GLOBAL", "STB_WEAK"};

static uint64_t orange_elf_get_byte(Elf32_Ehdr* e, void* base, elf_member_t member)
{
	u_int64_t val;

	val = 0;
	switch (e->e_ident[EI_CLASS]) {
		case ELFCLASS32:
			val = ((char*) base)[elf32_offsets[member]];
			break;
		case ELFCLASS64:
			val = ((char*) base)[elf64_offsets[member]];
			break;
		case ELFCLASSNONE:
			DEBUGP("invalid class\n");
	}

	return val;
}

static uint64_t orange_elf_get_word(Elf32_Ehdr* e, void* base, elf_member_t member)
{
	u_int64_t val;

	val = 0;
	switch (e->e_ident[EI_CLASS]) {
		case ELFCLASS32:
			base = (char*) base + elf32_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be32dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le32dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASS64:
			base = (char*) base + elf64_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be32dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le32dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASSNONE:
			DEBUGP("invalid class\n");
	}

	return val;
}

static uint64_t orange_elf_get_quad(Elf32_Ehdr* e, void* base, elf_member_t member)
{
	u_int64_t val;

	val = 0;
	switch (e->e_ident[EI_CLASS]) {
		case ELFCLASS32:
			base = (char*) base + elf32_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be32dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le32dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASS64:
			base = (char*) base + elf64_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be64dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le64dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASSNONE:
			DEBUGP("invalid class\n");
	}

	return val;
}

static uint64_t orange_elf_get_quarter(Elf32_Ehdr* e, void* base, elf_member_t member)
{
	u_int64_t val;

	val = 0;
	switch (e->e_ident[EI_CLASS]) {
		case ELFCLASS32:
			base = (char*) base + elf32_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be16dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le16dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASS64:
			base = (char*) base + elf64_offsets[member];
			switch (e->e_ident[EI_DATA]) {
				case ELFDATA2MSB:
					val = orange_be16dec(base);
					break;
				case ELFDATA2LSB:
					val = orange_le16dec(base);
					break;
				case ELFDATANONE:
					DEBUGP("invalid data format\n");
			}
			break;
		case ELFCLASSNONE:
			DEBUGP("invalid class\n");
	}

	return val;
}

static void __orange_elf_print_symtab(Elf32_Ehdr* e, void* sh, char* shstrtab, char* str, int (*print)(const char* fmt, ...))
{
	uint64_t offset;
	uint64_t entsize;
	uint64_t size;
	uint64_t name;
	uint64_t value;
	void*	st;
	int		 len;
	int		 i;

	offset  = orange_elf_get_off(e, sh, SH_OFFSET);
	entsize = orange_elf_get_size(e, sh, SH_ENTSIZE);
	size	= orange_elf_get_size(e, sh, SH_SIZE);
	name	= orange_elf_get_word(e, sh, SH_NAME);
	len		= size / entsize;

	print("\nsymbol table (%s):\n", shstrtab + name);

	for (i = 0; i < len; i++) {
		uint64_t info;
		uint64_t shndx;

		st	= (char*) e + offset + i * entsize;
		name  = orange_elf_get_word(e, st, ST_NAME);
		value = orange_elf_get_addr(e, st, ST_VALUE);
		size  = orange_elf_get_size(e, st, ST_SIZE);
		info  = orange_elf_get_byte(e, st, ST_INFO);
		shndx = orange_elf_get_quarter(e, st, ST_SHNDX);

		print("\n");
		print("entry: %d\n", i);
		print("\tst_name: %s\n", str + name);
		print("\tst_value: %#jx\n", value);
		print("\tst_size: %d\n", (int) size);

		print("\tst_info: %s %s\n", st_types[ELF32_ST_TYPE(info)], st_bindings[ELF32_ST_BIND(info)]);

		print("\tst_shndx: %d\n", (int) shndx);
	}
}

static int __orange_elf_get_symtab(Elf32_Ehdr* e, void* sh, char* shstrtab, char* str, orange_elf_symbol_hook hook, void* data)
{
	uint64_t offset;
	uint64_t entsize;
	uint64_t size;
	uint64_t name;
	uint64_t value;
	void*	st;
	int		 len;
	int		 i;
	int		 ret = 0;

	offset  = orange_elf_get_off(e, sh, SH_OFFSET);
	entsize = orange_elf_get_size(e, sh, SH_ENTSIZE);
	size	= orange_elf_get_size(e, sh, SH_SIZE);
	name	= orange_elf_get_word(e, sh, SH_NAME);
	len		= size / entsize;

	for (i = 0; i < len; i++) {
		st	= (char*) e + offset + i * entsize;
		name  = orange_elf_get_word(e, st, ST_NAME);
		value = orange_elf_get_addr(e, st, ST_VALUE);
		size  = orange_elf_get_size(e, st, ST_SIZE);

		if (*(str + name) != '\0' && value != 0) {
			ret = hook(str + name, value, data);
			if (ret != 0) {
				break;
			}
		}
	}

	return ret;
}

static int __orange_elf_symbol(struct orange_elf_file* file, int dump, int (*print)(const char* fmt, ...), orange_elf_symbol_hook hook, void* data)
{
	int		 ret = EINVAL;
	uint64_t i;
	uint64_t shnum;
	uint64_t shoff;
	uint64_t shentsize;
	uint64_t shstrndx;
	uint64_t type;
	uint64_t name;
	uint64_t offset;

	char* shstrtab;
	char* strtab = NULL;
	void* sh;
	void* v;

	shoff	 = orange_elf_get_off(file->e, file->e, E_SHOFF);
	shentsize = orange_elf_get_quarter(file->e, file->e, E_SHENTSIZE);
	shnum	 = orange_elf_get_quarter(file->e, file->e, E_SHNUM);
	shstrndx  = orange_elf_get_quarter(file->e, file->e, E_SHSTRNDX);

	sh		 = (char*) file->e + shoff;
	offset   = orange_elf_get_off(file->e, (char*) sh + shstrndx * shentsize, SH_OFFSET);
	shstrtab = (char*) file->e + offset;

	for (i = 0; (uint64_t) i < shnum; i++) {
		name   = orange_elf_get_word(file->e, (char*) sh + i * shentsize, SH_NAME);
		offset = orange_elf_get_off(file->e, (char*) sh + i * shentsize, SH_OFFSET);
		if (strcmp(shstrtab + name, ".strtab") == 0) {
			strtab = (char*) file->e + offset;
		}
#if 0
		if (strcmp(shstrtab + name, ".dynstr") == 0) {
			dynstr = (char *)file->e + offset;
        }
#endif
	}

	for (i = 0; (uint64_t) i < shnum; i++) {
		v	= (char*) sh + i * shentsize;
		type = orange_elf_get_word(file->e, v, SH_TYPE);
		switch (type) {
			case SHT_SYMTAB:
				if (dump) {
					__orange_elf_print_symtab(file->e, v, shstrtab, strtab, print);
				} else {
					ret = __orange_elf_get_symtab(file->e, v, shstrtab, strtab, hook, data);
					if (ret != 0) {
						goto exit;
					}
				}
				break;
		}
	}

exit:
	return ret;
}

int orange_elf_symbol(struct orange_elf_file* file, orange_elf_symbol_hook hook, void* data)
{
	return __orange_elf_symbol(file, 0, NULL, hook, data);
}

int orange_elf_dump(struct orange_elf_file* file, int (*print)(const char* fmt, ...))
{
	return __orange_elf_symbol(file, 1, print, NULL, NULL);
}

struct orange_elf_file* orange_elf_open(char* filename)
{
	int						fd;
	struct stat				sb;
	Elf32_Ehdr*				hdr;
	struct orange_elf_file* file = NULL;

	fd = open(filename, O_RDONLY);
	if (fd <= 0) {
		DEBUGP("%s: open file faild: %s\n", __func__, filename);
		goto exit;
	}

	if (fstat(fd, &sb) < 0) {
		close(fd);
		goto exit;
	}

	hdr = (Elf32_Ehdr*) mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (hdr == MAP_FAILED) {
		hdr = NULL;
		close(fd);
		goto exit;
	}

	file = orange_malloc(sizeof(struct orange_elf_file));
	if (file != NULL) {
		file->fd   = fd;
		file->e	= hdr;
		file->size = sb.st_size;
	} else {
		munmap(hdr, sb.st_size);
		close(fd);
	}
exit:
	return file;
}

void orange_elf_close(struct orange_elf_file* file)
{
	if (file != NULL) {
		munmap(file->e, file->size);
		close(file->fd);

		file->fd   = -1;
		file->size = 0;
		file->e	= NULL;
		orange_free(file);
	}
}
