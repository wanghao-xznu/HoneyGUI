add_subdirectory(nanovg)
add_subdirectory(ezXML)
add_subdirectory(nanosvg)
add_subdirectory(painter-engine)
add_subdirectory(stb)
add_subdirectory(cJSON)
add_subdirectory(box2d/src)
add_subdirectory(minimp3)
add_subdirectory(letter_shell)
add_subdirectory(tinyobj)
if(CONFIG_JAVASCRIPT_PARSER_SaaA)
add_subdirectory(js)
endif()
list(APPEND gui_3rd_sources 
			${gui_3rd_common_sources} 
			${gui_3rd_ezxml_sources}
			${gui_3rd_js_sources} 
			${gui_3rd_nanosvg_sources}
			${gui_3rd_nanovg_sources} 
			${gui_3rd_px_sources} 
			${gui_3rd_stb_sources}
			${gui_widget_cJSIN_sources}
			${BOX2D_SOURCE_FILES}
			${gui_minimp3_sources}
			${gui_3rd_letter_sources}
			${gui_tinyobj_sources}
			)
set(gui_3rd_sources ${gui_3rd_sources} CACHE INTERNAL  "gui engine 3rd ")

list(APPEND gui_3rd_headers
			${gui_3rd_common_headers} 
			${gui_3rd_ezxml_headers}
			${gui_3rd_js_headers} 
			${gui_3rd_nanosvg_headers}
			${gui_3rd_nanovg_headers} 
			${gui_3rd_px_headers} 
			${gui_3rd_stb_headers}
			${gui_widget_cJSIN_headers}
			${BOX2D_HEADER_FILES}
			${gui_minimp3_headers}
			${gui_3rd_letter_headers}
			${gui_tinyobj_headers}
			)
set(gui_3rd_headers ${gui_3rd_headers} CACHE INTERNAL  "gui 3rd headers ")

if(gui_debug_enable)  
	foreach(ITEM ${gui_3rd_sources})
		message([GUI][3rd srcs] ${ITEM})
	endforeach()
	
	foreach(ITEM ${gui_3rd_headers})
		message([GUI][3rd srcs] ${ITEM})
	endforeach()
endif()