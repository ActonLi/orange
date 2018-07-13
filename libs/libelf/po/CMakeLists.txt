message("CATALOGS = ${CATALOGS}")
foreach(cat ${CATALOGS})
	get_filename_component(lang ${cat} NAME_WE)
	if(CATOBJEXT STREQUAL ".mo")
		add_custom_command(OUTPUT ${cat} 
			COMMAND ${MSGFMT} -o ${cat} ${lang}.po
			DEPENDS ${lang}.po)
	else()
		add_custom_command(OUTPUT ${lang}.gmo
			COMMAND ${GMSGFMT} -o ${lang}.gmo ${lang}.po
			DEPENDS ${lang.po})
		if(CATOBJEXT STREQUAL ".cat")
			add_executable(gmo2msg gmo2msg.c)
			target_include_directories(gmo2msg PRIVATE ${CMAKE_SOURCE_DIR}/lib)
			target_compile_definitions(gmo2msg PRIVATE HAVE_CONFIG_H)
			target_link_libraries(gmo2msg PRIVATE "${LIBINTL}")
			
			# Create X/Open message catalog sources from .gmo files.
			add_custom_command(OUTPUT ${lang}.msg
				COMMAND gmo2msg ${lang}
				DEPENDS ${lang}.gmo)
			add_custom_command(OUTPUT ${cat}
				COMMAND ${GENCAT} ${cat} ${lang}.msg
				DEPENDS ${lang}.msg)
		endif()
	endif()	
	install(FILES ${cat}
		DESTINATION ${localedir}/${lang}/LC_MESSAGES
		RENAME ${PACKAGE}${INSTOBJEXT})
endforeach()

set(POTFILES ${CMAKE_SOURCE_DIR}/lib/errors.h)
add_custom_command(OUTPUT ${PACKAGE}.pot
	COMMAND ${XGETTEXT} -c -d${PACKAGE} -k_ ${POTFILES}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PACKAGE}.po ${PACKAGE}.pot
	COMMAND ${CMAKE_COMMAND} -E remove -f ${PACKAGE}.po
	DEPENDS ${POTFILES}
)

file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/stamp-po.cmake
	CONTENT
"foreach(po ${POFILES})
	execute_process(COMMAND ${MSGMERGE} \${po} ${PACKAGE}.pot
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		RESULT_VARIABLE msgmerge_res
		OUTPUT_VARIABLE msgmerge_out)
	if(msgmerge_res EQUAL 0)
		file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/\${po} \"\${msgmerge_out}\")
	else()
		message(WARNING \"update for \${po} failed!\")
	endif()
	file(GENERATE OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/stamp-po CONTENT \"timestamp\\n\")
endforeach()")

add_custom_target(stamp-po
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/stamp-po.cmake
	DEPENDS ${PACKAGE}.pot)

add_custom_target(update-po
	DEPENDS stamp-po)
