set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCES appmain.cc shadow_mapping_widget.cc)
set(HEADERS shadow_mapping_widget.h)
set(SHADERS render.fs render.vs shadow_mapping.fs shadow_mapping.vs)

add_executable(qtshadowmapping ${SOURCES} ${HEADERS} ${SHADERS})
qt5_use_modules(qtshadowmapping Widgets OpenGL)
target_link_libraries(qtshadowmapping ${QT_LIBRARIES})

set (QT_LIBRARY_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL)
set (ICXXX_LIBRARY_LIST icuin53 icuuc53 icudt53)

if (MSVC)
  foreach (LIB ${QT_LIBRARY_LIST})
    add_custom_command(TARGET qtshadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
  endforeach()

  foreach (LIB ${QT_LIBRARY_LIST})
    add_custom_command(TARGET qtshadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}d.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
  endforeach()

  foreach (LIB ${ICXXX_LIBRARY_LIST})
    add_custom_command(TARGET qtshadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
  endforeach()

  foreach (LIB ${ICXXX_LIBRARY_LIST})
    add_custom_command(TARGET qtshadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
  endforeach()
endif()
