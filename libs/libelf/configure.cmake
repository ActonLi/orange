include(AutoconfHelper)
include(aclocal.cmake)

mr_PACKAGE(libelf)

# NOTE: there must be at least one .po file!
file(GLOB ALL_LINGUAS 
	RELATIVE "${CMAKE_SOURCE_DIR}/po"
	"po/*.po")
string(REPLACE ".po" "" ALL_LINGUAS "${ALL_LINGUAS}")

# Assuming all arguments have already been processed...
string(REGEX REPLACE "([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" MAJOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.([0-9]+)\\.[0-9]+" "\\1" MINOR ${VERSION})
string(REGEX REPLACE "[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" PATCH ${VERSION})  

# Checks for header files.
ac_header_stdc()
ac_check_headers(unistd.h stdint.h fcntl.h)
ac_check_headers(elf.h sys/elf.h link.h sys/link.h)

if(ac_cv_header_elf_h)
   set(__LIBELF_HEADER_ELF_H <elf.h>)
elseif(ac_cv_header_sys_elf_h)
   set(__LIBELF_HEADER_ELF_H <sys/elf.h>)
endif()

ac_try_compile("
#include ${__LIBELF_HEADER_ELF_H}

int main()
{
	Elf32_Ehdr dummy;
    return 0;
}"
libelf_cv_elf_h_works)

if(NOT libelf_cv_elf_h_works)
	set(ac_cv_header_elf_h FALSE)
	set(ac_cv_header_sys_elf_h FALSE)
	unset(__LIBELF_HEADER_ELF_H)
endif()

ac_check_headers(ar.h libelf.h nlist.h gelf.h)
message(STATUS "Checking whether to install <libelf.h>, <nlist.h> and <gelf.h>...")
if(ac_cv_header_libelf_h AND (ac_cv_header_nlist_h AND ac_cv_header_gelf_h))
  set(compat_value TRUE)
else()
  set(compat_value FALSE)
endif()
option(compat "install <libelf.h>, <nlist.h> and <gelf.h>" ${compat_value})
set(DO_COMPAT ${compat})
message(STATUS "DO_COMPAT: ${DO_COMPAT}")

# Checks for typedefs, structures, and compiler characteristics.
ac_c_const()
ac_type_off_t()
ac_type_size_t()

ac_check_sizeof(short)
ac_check_sizeof(int)
ac_check_sizeof(long)
ac_check_sizeof("long long")
# Windows port
ac_check_sizeof(__int64)

if(ac_cv_header_elf_h OR ac_cv_header_sys_elf_h)
    # Slowaris declares Elf32_Dyn in <link.h>.
    # QNX declares Elf32_Dyn in <sys/link.h>.
    message(STATUS "Checking for struct Elf32_Dyn...")
    ac_try_compile("
   #include ${__LIBELF_HEADER_ELF_H}
   int main()
   {
       Elf32_Dyn x;
       return 0;
   }"
   libelf_cv_struct_elf32_dyn)
   if(NOT libelf_cv_struct_elf32_dyn)
       ac_try_compile([=[
       #include <link.h>
       int main()
       {
           Elf32_Dyn x;
           return 0;
       }]=]
       __LIBELF_NEED_LINK_H)
       if(NOT __LIBELF_NEED_LINK_H)
           ac_try_compile([=[
           #include <sys/link.h>
           int main()
           {
               Elf32_Dyn x;
               return 0;
           }]=]
           __LIBELF_NEED_SYS_LINK_H)
           if(NOT __LIBELF_NEED_SYS_LINK_H)
               message(FATAL_ERROR "no declaration for Elf32_Dyn")
           endif()
       endif()
   endif()

   # Linux declares struct nlist in <elf.h>.
   message(STATUS "Checking for struct nlist in elf.h...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
   int main()
   {
       struct nlist nl;
       return 0;
   }" 
   HAVE_STRUCT_NLIST_DECLARATION)
   
   # Check for 64-bit data types.
   message(STATUS "Checking for struct Elf64_Ehdr...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
   int main()
   {
       Elf64_Ehdr x;
       return 0;
   }" 
   libelf_cv_struct_elf64_ehdr)
   
   # Linux lacks typedefs for scalar ELF64_* types.
   message(STATUS "Checking for Elf64_Addr...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
   int main()
   {
       Elf64_Addr x;
       return 0;
   }" 
   libelf_cv_type_elf64_addr)
   
   #  IRIX' struct Elf64_Rel is slightly different. Ugh.
   message(STATUS "Checking for struct Elf64_Rel...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
   int main()
   {
       Elf64_Rel x; 
       x.r_info = 1;
       return 0;
   }" 
   libelf_cv_struct_elf64_rel)
   if(NOT libelf_cv_struct_elf64_rel)
       ac_try_compile("
        #include ${__LIBELF_HEADER_ELF_H}
       int main()
       {
           Elf64_Rel x; 
           x.r_sym = 1;
           return 0;
       }" 
       libelf_cv_struct_elf64_rel_irix)
   endif()
   
   if(libelf_cv_struct_elf64_ehdr AND (libelf_cv_type_elf64_addr AND libelf_cv_struct_elf64_rel))
       set(libelf_64bit TRUE)
   elseif(libelf_cv_struct_elf64_ehdr AND (libelf_cv_type_elf64_addr AND libelf_cv_struct_elf64_rel_irix))
       set(__LIBELF64_IRIX TRUE)
       set(libelf_64bit TRUE)
   elseif(libelf_cv_struct_elf64_ehdr AND ((NOT libelf_cv_type_elf64_addr) AND libelf_cv_struct_elf64_rel))
       set(__LIBELF64_LINUX TRUE)
       set(libelf_64bit TRUE)
   else()
       set(libelf_64bit FALSE)
   endif()
   
   # Check for symbol versioning definitions
   message(STATUS "Checking for Elf32_Verdef...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
    #if __LIBELF_NEED_LINK_H
    #include <link.h>	/* Solaris wants this */
    #endif
   int main()
   {
       struct {
           Elf32_Verdef vd;
           Elf32_Verdaux vda;
           Elf32_Verneed vn;
           Elf32_Vernaux vna;
       } x;
       return 0;
   }" 
   libelf_cv_verdef32)
   
   message(STATUS "Checking for Elf64_Verdef...")
   ac_try_compile("
    #include ${__LIBELF_HEADER_ELF_H}
    #if __LIBELF_NEED_LINK_H
    #include <link.h>	/* Solaris wants this */
    #endif
	int main()
	{
	   struct {
		   Elf64_Verdef vd;
		   Elf64_Verdaux vda;
		   Elf64_Verneed vn;
		   Elf64_Vernaux vna;
	   } x;
	   return 0;
	}" 
	libelf_cv_verdef64)
   
	message(STATUS "Checking for SHT_SUNW_verdef...")
	ac_try_compile("
	#include ${__LIBELF_HEADER_ELF_H}
	int main()
	{
	   Elf32_Word x = SHT_SUNW_verdef + SHT_SUNW_verneed + SHT_SUNW_versym;
	   return 0;
	}" 
	libelf_cv_sun_verdef)

	message(STATUS "Checking for SHT_GNU_verdef...")
	ac_try_compile("
	#include ${__LIBELF_HEADER_ELF_H}
	int main()
	{
	   Elf32_Word x = SHT_GNU_verdef + SHT_GNU_verneed + SHT_GNU_versym;
	   return 0;
	}" 
	libelf_cv_gnu_verdef)

else()
   # lib/elf_repl.h supports 64-bit
 
   set(libelf_64bit TRUE)

   # lib/elf_repl.h supports symbol versioning
   set(libelf_cv_verdef32 TRUE)
   set(libelf_cv_verdef64 TRUE)
   set(libelf_cv_sun_verdef TRUE)
   set(libelf_cv_gnu_verdef TRUE)
endif()

message(STATUS "Checking for 64-bit integer...")
if(SIZEOF_LONG EQUAL 8)
   set(libelf_cv_int64 long)
elseif(SIZEOF___INT64 EQUAL 8)
   set(libelf_cv_int64 __int64)
elseif(SIZEOF_LONG_LONG 8)
   set(libelf_cv_int64 "long long")
else()
   set(libelf_cv_int64 FALSE)
endif()
if(libelf_cv_int64)
   set(__libelf_i64_t ${libelf_cv_int64})
   set(__libelf_u64_t "unsigned ${libelf_cv_int64}")
else()
   set(libelf_64bit FALSE)
endif()

message(STATUS "Checking for 32-bit integer...")
if(SIZEOF_INT EQUAL 4)
   set(libelf_cv_int32 int)
elseif(SIZEOF_LONG EQUAL 4)
   set(libelf_cv_int32 long)
else()
   set(libelf_cv_int32 FALSE)
endif()
if(libelf_cv_int32)
   set(__libelf_i32_t ${libelf_cv_int32})
   set(__libelf_u32_t "unsigned ${libelf_cv_int32}")
else()
   message(FATAL_ERROR "neither int nor long is 32-bit")
endif()

message(STATUS "Checking for 16-bit integer...")
if(SIZEOF_SHORT EQUAL 2)
   set(libelf_cv_int16 short)
elseif(SIZEOF_INT EQUAL 2)
   set(libelf_cv_int16 int)
else()
   set(libelf_cv_int16 FALSE)
endif()

if(libelf_cv_int16)
   set(__libelf_i16_t ${libelf_cv_int16})
   set(__libelf_u16_t "unsigned ${libelf_cv_int16}")
else()
   message(FATAL_ERROR "neither short nor int is 16-bit")
endif()

ac_check_headers("unistd.h")

# Checks for library functions.
ac_check_funcs(getpagesize mmap ftruncate memcmp memcpy memmove memset)

message(STATUS "Checking whether overlapping arrays are copied correctly...")
if(HAVE_MEMMOVE)
set(CMAKE_REQUIRED_DEFINITIONS "-DHAVE_MEMMOVE=1")
endif()
check_c_source_runs([=[
#if HAVE_MEMMOVE
extern void *memmove();
#else
extern void bcopy();
#define memmove(d,s,n) bcopy((s),(d),(n))
#endif
extern int strcmp();
main() {
 char buf[] = "0123456789";
 memmove(buf + 1, buf, 9);
 if (strcmp(buf, "0012345678")) exit(1);
 exit(0);
}]=]
libelf_cv_working_memmove)
set(CMAKE_REQUIRED_DEFINITIONS)
if(NOT libelf_cv_working_memmove)
   set(HAVE_BROKEN_MEMMOVE TRUE)
endif()

# Check for 64-bit support.
message(STATUS "Checking whether 64-bit ELF support is sufficient... ${libelf_64bit}")
if(libelf_64bit)
   option(elf64 "compile in 64-bit support" TRUE)
endif()
set(libelf_enable_64bit ${elf64})
message(STATUS "Checking whether to include 64-bit support... ${libelf_enable_64bit}")
if(libelf_enable_64bit)
   set(__LIBELF64 TRUE)
endif()

set(libelf_versioning FALSE)
if(((NOT libelf_enable_64bit) AND libelf_cv_verdef32) OR
   (libelf_enable_64bit AND (libelf_cv_verdef32 AND libelf_cv_verdef64)))
   if(libelf_cv_sun_verdef)
       set(__LIBELF_SUN_SYMBOL_VERSIONS TRUE)
       set(libelf_versioning TRUE)
   elseif(libelf_cv_gnu_verdef)
       set(__LIBELF_GNU_SYMBOL_VERSIONS TRUE)
       set(libelf_versioning TRUE)
   endif()
endif()
message(STATUS "Checking whether versioning support is sufficient... ${libelf_versioning}")

if(libelf_versioning)
   option(versioning "compile in versioning support" TRUE)
else()
   set(versioning FALSE)
endif()
set(libelf_enable_versioning ${versioning})
message(STATUS "Checking whether to include versioning support ${libelf_enable_versioning}")
if(libelf_enable_versioning)
   set(__LIBELF_SYMBOL_VERSIONS TRUE)
endif()

# Check for NLS support.
mr_ENABLE_NLS()
# this is for gmo2msg...
ac_check_lib(intl gettext)
if(HAVE_INTL)
	set(LIBINTL "intl")
endif() 

# Check for shared library support.
mr_ENABLE_SHARED()

# Check for extended ELF format support
option(extended-format "support extended file formats" FALSE)
set(mr_enable_extended_format ${extended-format})
if(mr_enable_extended_format)
    set(ENABLE_EXTENDED_FORMAT TRUE)
endif()

# Check if ELF sanity checks should be enabled
option(sanity-checks "enable sanity checks by default" TRUE)
set(mr_enable_sanity_checks ${sanity-checks})
if(mr_enable_sanity_checks)
    set(ENABLE_SANITY_CHECKS TRUE)
endif()

# Check for debug support.
mr_ENABLE_DEBUG()

configure_file(config.h.cmake.in config.h)
configure_file(libelf.pc.cmake.in libelf.pc @ONLY)