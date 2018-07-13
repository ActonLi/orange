include(GNUInstallDirs)

set_source_group(SRCS1 "generic sources" begin.c cntl.c end.c errmsg.c errno.c fill.c flag.c getarhdr.c
	getarsym.c getbase.c getdata.c getident.c getscn.c hash.c kind.c
	ndxscn.c newdata.c newscn.c next.c nextscn.c rand.c rawdata.c
	rawfile.c strptr.c update.c version.c checksum.c getaroff.c)
	
set_source_group(SRCS2 "32-bit sources" 32.fsize.c 32.getehdr.c 32.getphdr.c 32.getshdr.c 32.newehdr.c
	32.newphdr.c 32.xlatetof.c)

set_source_group(SRCS3 "support" cook.c data.c input.c assert.c)

set_source_group(SRCS4 "nlist" nlist.c)

set_source_group(SRCS5 "opt" opt.delscn.c x.remscn.c x.movscn.c x.elfext.c)

set_source_group(SRCS64 "64-bit sources" 64.xlatetof.c gelfehdr.c gelfphdr.c gelfshdr.c gelftrans.c swap64.c)

set_source_group(SRCS_V "Versioning sources" verdef_32_tof.c verdef_32_tom.c verdef_64_tof.c verdef_64_tom.c)
set_source_group(HDRS_V verdef.h verneed.h)

set_source_group(HDRS "public header files" libelf.h nlist.h gelf.h)

set_source_group(AUXHDRS "public header files" ${CMAKE_CURRENT_BINARY_DIR}/sys_elf.h)

set_source_group(PRIVHDRS "private header files" byteswap.h errors.h ext_types.h private.h elf_repl.h)

configure_file(sys_elf.h.cmake.in sys_elf.h)

add_library(elf STATIC ${SRCS1} ${SRCS2} ${SRCS3} ${SRCS4} ${SRCS5} ${SRCS64} ${SRCS_V} ${HDRS_V} ${HDRS} ${AUXHDRS} ${PRIVHDRS})

set_target_properties(elf PROPERTIES
	PREFIX lib
	VERSION ${VERSION})

target_include_directories(elf PRIVATE ${CMAKE_BINARY_DIR}/libelf)

target_compile_definitions(elf PRIVATE HAVE_CONFIG_H)

install(TARGETS elf 
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}		
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(DO_SHLIB)
    add_library(elf_shared SHARED ${SRCS1} ${SRCS2} ${SRCS3} ${SRCS4} ${SRCS5} ${SRCS64} ${SRCS_V} ${HDRS_V} ${HDRS} ${AUXHDRS} ${PRIVHDRS}
		$<$<BOOL:${MSVC}>:libelf.def>)
    
    if (UNIX AND mr_enable_gnu_names)
		set_target_properties(elf_shared PROPERTIES 
			OUTPUT_NAME elf-${VERSION}
			NO_SONAME TRUE)
		
		add_custom_command(TARGET elf_shared
			POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:elf_shared> libelf.so.${MAJOR}
            COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:elf_shared> libelf.so)

	else()
		set_target_properties(elf_shared PROPERTIES 
			OUTPUT_NAME elf
			VERSION ${VERSION}
			SOVERSION ${MAJOR})	
	endif()

    target_include_directories(elf_shared PRIVATE ${CMAKE_BINARY_DIR}/libelf)
    
    target_compile_definitions(elf_shared PRIVATE HAVE_CONFIG_H)

	target_link_libraries(elf_shared PUBLIC
		"${elf_extra_libraries}")
    
    install(TARGETS elf_shared 
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}		
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

endif()

install(FILES ${HDRS} ${AUXHDRS} elf_repl.h
    DESTINATION include/libelf)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/install_compat.cmake
	CONTENT 
"foreach(file ${HDRS})
    file(WRITE \${file}
        \"#include <libelf/\${file}>\")
	file(INSTALL \${file} 
		DESTINATION \"${CMAKE_INSTALL_PREFIX}/include\")
endforeach()"
)
    
if(DO_COMPAT)
	install(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/install_compat.cmake)
endif()

add_custom_target(install-compat
	COMMAND ${CMAKE_COMMAND} -P install_compat.cmake
	SOURCES install_compat.cmake)
	