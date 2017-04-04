cmake_minimum_required(VERSION 3.6)
project(Assignment)

set(CMAKE_CXX_STANDARD 14)

# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory(third/glfw)
add_definitions(-DGLFW_INCLUDE_NONE -DGLM_FORCE_SIZE_T_LENGTH)

# OpenGL
find_package(OpenGL REQUIRED)
link_libraries(${OPENGL_LIBRARIES})

# Warning Level
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

file(GLOB INCS include/*.h include/*.hpp)
file(GLOB SRCS src/*.cpp src/*.hpp src/*.c src/*.h)

file(GLOB SHADERS   shader/*.vert
                    shader/*.tesc
                    shader/*.tese
                    shader/*.geom
                    shader/*.comp
					shader/*.frag)
file(GLOB CONTENT	content/*)

file(GLOB STB_SRCS third/stb/stb_image.h)
file(GLOB GL_SRCS third/gl/include/*.hpp third/gl/src/*.cpp)
file(GLOB GLM_SRCS	third/glm/glm/*.hpp
					third/glm/glm/detail/*.hpp
					third/glm/glm/gtc/*.hpp
					third/glm/glm/gtx/*.hpp
					third/glm/glm/simd/*.hpp)
add_definitions(-DGLM_FORCE_RADIANS)

file(GLOB THIRD_SRCS ${GL_SRCS} ${GLM_SRCS} ${STB_SRCS})

source_group("Content" FILES ${CONTENT})
source_group("Source" FILES ${SRCS})
source_group("Header" FILES ${INCS})
source_group("GLSL" FILES ${SHADERS})
source_group("Third\\STB" FILES ${STB_SRCS})
source_group("Third\\GL" FILES ${GL_SRCS})
source_group("Third\\GLM" FILES ${GLM_SRCS})

include_directories(include/
                    third/gl/include/
                    third/glfw/include/
                    third/glm/
					third/stb/)

add_executable(${PROJECT_NAME} ${SRCS} ${INCS} ${SHADERS} ${THIRD_SRCS} ${CONTENT})
target_link_libraries(${PROJECT_NAME} glfw ${GL_LIBRARIES})

if(MSVC)
    add_custom_target(CopyStuff
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shader ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/shader
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/content ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/content)
else()
    add_custom_target(CopyStuff
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/shader ${CMAKE_CURRENT_BINARY_DIR}/shader
            COMMAND "${CMAKE_COMMAND}" -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/content ${CMAKE_CURRENT_BINARY_DIR}/content)
endif()

option(AUTO_COPY "Automatically copy shader and content to binary directory" OFF)
if(AUTO_COPY)
    add_dependencies(${PROJECT_NAME} CopyStuff)
endif()