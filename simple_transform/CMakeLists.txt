set(SOURCE_FILES
"pluginMain.cpp"
"nodes_manip.cpp"
"nodes_dg.cpp"
"nodes_dag_aim.cpp"
"nodes_dag_static"
"nodes.h"
)

# find FindMaya.cmake package, all variables will be avaiable
find_package(Maya REQUIRED)
include_directories(${MAYA_INCLUDE_DIR})
link_directories(${MAYA_LIBRARY_DIR})

# add source files to CMake PROJECT_NAME is created when define project()
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${MAYA_LIBRARIES})

MAYA_PLUGIN(${PROJECT_NAME})

# cmake_current_binary_dir, where build directory are
install(TARGETS ${PROJECT_NAME} ${MAYA_TARGET_TYPE} DESTINATION plug-ins)
