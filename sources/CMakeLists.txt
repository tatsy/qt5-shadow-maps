set(BUILD_TARGET "shadowmaps")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(common.h.in ${CMAKE_CURRENT_LIST_DIR}/common.h @ONLY)

set(SOURCES appmain.cc maingui.cc shadowmaps_widget.cc vbo.cc arcball_controller.cc)
set(HEADERS maingui.h shadowmaps_widget.h vbo.h arcball_controller.h common.h)
file(GLOB SHADERS "${CMAKE_CURRENT_LIST_DIR}/shaders/*.[vfg]s")
message(STATUS ${SHADERS})

add_definitions(-D_USE_MATH_DEFINES)
add_executable(${BUILD_TARGET} ${SOURCES} ${HEADERS} ${SHADERS})
qt5_use_modules(${BUILD_TARGET} Widgets OpenGL)
target_link_libraries(${BUILD_TARGET} ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

source_group("Source Files" FILES ${SOURCES} ${HEADERS})
source_group("Shaders" FILES ${SHADERS})

set (QT_LIBRARY_LIST Qt5Widgets Qt5Core Qt5Gui Qt5OpenGL
     Qt5Widgetsd Qt5Cored Qt5Guid Qt5OpenGLd
     icuin54 icuuc54 icudt54)

if (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
  set_property(TARGET ${BUILD_TARGET} APPEND PROPERTY LINK_FLAGS "/DEBUG /PROFILE")

  foreach (LIB ${QT_LIBRARY_LIST})
    add_custom_command(TARGET ${BUILD_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
  ARGS "${CMAKE_PREFIX_PATH}/bin/${LIB}.dll" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  endforeach()
endif()
