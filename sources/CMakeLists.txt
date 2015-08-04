set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(common.h.in ${CMAKE_CURRENT_LIST_DIR}/common.h @ONLY)

set(SOURCES appmain.cc shadow_mapping_widget.cc vbo.cc)
set(HEADERS shadow_mapping_widget.h vbo.h common.h)
set(SHADERS render.fs render.vs shadow_mapping.fs shadow_mapping.vs)

add_executable(qt5shadowmapping ${SOURCES} ${HEADERS} ${SHADERS})
qt5_use_modules(qt5shadowmapping Widgets OpenGL)
target_link_libraries(qt5shadowmapping ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

set (QT_LIBRARY_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL)
set (ICXXX_LIBRARY_LIST icuin53 icuuc53 icudt53)

if (MSVC)
  set_property(TARGET qt5shadowmapping APPEND PROPERTY LINK_FLAGS /DEBUG)

  foreach (LIB ${QT_LIBRARY_LIST})
    add_custom_command(TARGET qt5shadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
  endforeach()

  foreach (LIB ${QT_LIBRARY_LIST})
    add_custom_command(TARGET qt5shadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}d.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
  endforeach()

  foreach (LIB ${ICXXX_LIBRARY_LIST})
    add_custom_command(TARGET qt5shadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
  endforeach()

  foreach (LIB ${ICXXX_LIBRARY_LIST})
    add_custom_command(TARGET qt5shadowmapping POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ARGS ${CMAKE_PREFIX_PATH}/bin/${LIB}.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
  endforeach()
endif()
