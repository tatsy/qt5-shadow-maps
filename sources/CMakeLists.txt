set(BUILD_TARGET "shadowmaps")
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(common.h.in ${CMAKE_CURRENT_LIST_DIR}/common.h @ONLY)

file(GLOB SOURCES "*.cpp" "*.h")
file(GLOB SHADERS "shaders/*.[vgf]s")

add_executable(${BUILD_TARGET} ${SOURCES} ${HEADERS} ${SHADERS})
qt5_use_modules(${BUILD_TARGET} Widgets OpenGL)
target_link_libraries(${BUILD_TARGET} ${QT_LIBRARIES} ${OPENGL_LIBRARIES})

source_group("Source Files" FILES ${SOURCES} ${HEADERS})
source_group("Shaders" FILES ${SHADERS})
