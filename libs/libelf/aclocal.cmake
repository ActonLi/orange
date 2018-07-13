# aclocal.m4 - Local additions to Autoconf macros.
# Copyright (C) 1995 - 2006 Michael Riepe
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA

# @(#) $Id: aclocal.m4,v 1.28 2008/05/23 08:17:56 michael Exp $

include(CMakeDependentOption)
include(CMakeDependentVariable)

# mr_PACKAGE(package-name)
macro(mr_PACKAGE PACKAGE)
    file(READ VERSION VERSION)
    string(STRIP ${VERSION} VERSION)
    set(PACKAGE ${PACKAGE})
    option(maintainer-mode "enable maintainer-specific make rules" ${I_AM_THE_MAINTAINER_OF})
    if(maintainer-mode)
        set(MAINT)
    else()
        set(MAINT maintainer-only-)
    endif()
endmacro()

macro(mr_ENABLE_NLS)    
    # Needed for `make dist' even if NLS is disabled.
    set(GMOFILES)
    set(MSGFILES)
    set(POFILES)
    foreach(mr_lang ${ALL_LINGUAS})
        list(APPEND GMOFILES ${mr_lang}.gmo)
        list(APPEND MSGFILES ${mr_lang}.msg)
        list(APPEND POFILES ${mr_lang}.po)
    endforeach()
    
    option(nls "use Native Language Support" TRUE)
    set(mr_enable_nls ${nls})
    message(STATUS "Checking whether NLS is requested - ${mr_enable_nls}")
    
    set(CATOBJEXT)
    set(INSTOBJEXT)
    set(localedir)
    if(mr_enable_nls)
        if(WIN32)
            set(pathSep "\\;")
        else()
            set(pathSep ":")
        endif() 
        string(REGEX REPLACE "^[^${pathSep}]*openwin[^${pathSep}]*" "" mr_PATH $ENV{PATH})
        set(ENV{mr_PATH} ${mr_PATH})
        check_c_source_compiles([=[
        #include <libintl.h>
        int main()
        {
            char *s = dgettext("", "");
            return 0;
        }]=]
        mr_cv_func_dgettext)
        message(STATUS "Checking for dgettext... ${mr_cv_func_dgettext}")
        if(mr_cv_func_dgettext)
            ac_path_prog(MSGFMT msgfmt FALSE mr_PATH)
            if(MSGFMT)
                ac_path_prog(GMSGFMT gmsgfmt ${MSGFMT} mr_PATH)
                ac_path_prog(XGETTEXT xgettext xgettext mr_PATH)
                ac_path_prog(MSGMERGE msgmerge msgmerge mr_PATH)
                check_c_source_compiles([=[
                int main()
                {
                    extern int _nl_msg_cat_cntr; 
                    return _nl_msg_cat_cntr;
                }]=]
                mr_cv_gnu_gettext)
                message(STATUS "Checking for GNU gettext... ${mr_cv_gnu_gettext}")
                if(mr_cv_gnu_gettext)
                    check_c_source_compiles([=[
                    int main()
                    {
                        extern int _msg_tbl_length; 
                        return _msg_tbl_length;
                    }]=]
                    mr_cv_catgets_based_gettext)
                    message(STATUS "Checking for losing catgets-based GNU gettext... ${mr_cv_catgets_based_gettext}")
                    if(mr_cv_catgets_based_gettext)
                	    # This loses completely. Turn it off and use catgets.
                	    string(REGEX REPLACE "-lintl" "" LIBS ${LIBS})
                	    set(mr_cv_func_dgettext FALSE)
                	else()
	                    # Is there a better test for this case?
                        check_c_source_compiles([=[
                        int main()
                        {
                            extern int gettext(); 
                            return gettext();
                        }]=]
                        mr_cv_pure_gnu_gettext)
	                    message(STATUS "Checking for pure GNU gettext... ${mr_cv_pure_gnu_gettext}")
                        if(mr_cv_pure_gnu_gettext)
                            set(CATOBJEXT ".gmo")
                            set(localedir "${CMAKE_INSTALL_PREFIX}/share/locale")
                        else()
                            set(CATOBJEXT ".mo")
                            set(localedir "${CMAKE_INSTALL_PREFIX}/lib/locale")
                        endif()                            
                        set(INSTOBJEXT ".mo")
                    endif()
                else()
                    set(CATOBJEXT ".mo")         
                    set(INSTOBJEXT ".mo")
                    set(localedir "${CMAKE_INSTALL_PREFIX}/lib/locale")                    
                endif()
            else()
                # Gettext but no msgfmt. Try catgets.
                set(mr_cv_func_dgettext FALSE)
            endif()            
        endif()
        if(mr_cv_func_dgettext)
            set(HAVE_DGETTEXT TRUE CACHE BOOL "have dgettext")
        else()
            check_c_source_compiles([=[
            #include <nl_types.h>
            int main()
            {
                catgets(catopen("",0),0,0,"");
                return 0;
            }]=]
            mr_cv_func_catgets)
            message(STATUS "Checking for catgets... ${mr_cv_func_catgets}")
            if(mr_cv_func_catgets)
                ac_path_prog(GENCAT gencat FALSE mr_PATH)
                if(GENCAT)
                    set(HAVE_CATGETS TRUE CACHE BOOL "have catgets")
                    ac_path_prog(GMSGFMT "gmsgfmt msgfmt" msgfmt mr_PATH)
                    ac_path_prog(XGETTEXT xgettext xgettext mr_PATH)
					ac_path_prog(MSGMERGE msgmerge msgmerge mr_PATH)
                    set(CATOBJEXT ".cat")
                    set(INSTOBJEXT ".cat")
                    set(localedir "${CMAKE_INSTALL_PREFIX}/lib/locale")
                endif()
            else()
                message(WARNING "no NLS support, disabled")
                set(mr_enable_nls FALSE)
            endif()
        endif()
    endif()

    set(POSUB)
    set(CATALOGS)
    if(mr_enable_nls)
        set(mr_linguas)
        foreach(mr_lang ${ALL_LINGUAS})
            list(APPEND mr_linguas ${mr_lang})
            list(APPEND CATALOGS ${mr_lang}${CATOBJEXT})
        endforeach()
        message(STATUS "Checking for catalogs to be installed... ${mr_linguas}")
        set(POSUB po)
    endif()
endmacro()

macro(mr_TARGET_ELF)
    message(STATUS "Checking for native ELF system...")
    check_c_source_runs([=[
    #include <stdio.h>
    int
    main(int argc, char **argv) {
      char buf[BUFSIZ];
      FILE *fp;
      int n;
    
      if ((fp = fopen(*argv, "r")) == NULL) {
        exit(1);
      }
      n = fread(buf, 1, sizeof(buf), fp);
      if (n >= 52
       && buf[0] == '\\177'
       && buf[1] == 'E'
       && buf[2] == 'L'
       && buf[3] == 'F') {
        exit(0);
      }
      exit(1);
    }]=]
    mr_cv_target_elf)
    message(STATUS "Checking for native ELF system... ${mr_cv_target_elf}")
endmacro()

macro(mr_ENABLE_SHARED)
    option(shared "build shared library" TRUE)
    set(mr_enable_shared ${shared})
    message(STATUS "Checking whether to build a shared library... ${mr_enable_shared}")
    set(DO_SHLIB ${mr_enable_shared})
	cmake_dependent_option(gnu-names "use GNU library naming conventions" FALSE "DO_SHLIB" FALSE)
	if((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME STREQUAL "QNX"))
		set(elf_extra_libraries_default "c")
	endif()
	cmake_dependent_variable(elf_extra_libraries "Libraries to link to the shared library" STRING "${elf_extra_libraries_default}" "DO_SHLIB" "")
	if(DO_SHLIB)
		mr_TARGET_ELF()		
		set(mr_enable_gnu_names ${gnu-names})
		message(STATUS "Checking whether GNU naming conventions are requested..." ${mr_enable_gnu_names})
	endif()
endmacro()

macro(mr_ENABLE_DEBUG)
  option(debug "include extra debugging code" FALSE)
  set(mr_enable_debug ${debug})
  if(mr_enable_debug)
      set(ENABLE_DEBUG TRUE)
  endif()
endmacro()

# vi: set ts=8 sw=2 :
