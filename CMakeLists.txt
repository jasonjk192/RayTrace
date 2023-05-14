cmake_minimum_required (VERSION 3.21)

project ("Raytrace")

set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS FALSE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

include(FetchContent)

#-----------------------------
# External libraries from git repos
#-----------------------------
# glfw
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.3.6)
FetchContent_MakeAvailable(glfw)
#-----------------------------

#-----------------------------
# Dear ImGui
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git GIT_TAG origin/docking)
FetchContent_MakeAvailable(imgui)
add_subdirectory("CMake Subfolders/ImGui")
#-----------------------------

#-----------------------------
# GLM
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 0.9.9.8)
FetchContent_MakeAvailable(glm)
add_compile_definitions(GLM_FORCE_LEFT_HANDED)
#-----------------------------

#-----------------------------
# GLAD
add_subdirectory("CMake Subfolders/GLAD")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/glad/include)
#-----------------------------

#-----------------------------
# stb_image
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG origin/master)
FetchContent_MakeAvailable(stb)
#-----------------------------


#-----------------------------
# Source files
#-----------------------------

set(PROJECT_SOURCES_WITHOUT_MAIN
	""
	)

# Using OpenGL compute shaders
#add_executable(${PROJECT_NAME} src/glraytrace.cpp ${PROJECT_SOURCES_WITHOUT_MAIN})

# Using C++
#add_executable(${PROJECT_NAME} src/cppraytrace.cpp ${PROJECT_SOURCES_WITHOUT_MAIN})

# Experimentation
add_executable(${PROJECT_NAME} src/expraytrace.cpp ${PROJECT_SOURCES_WITHOUT_MAIN})

#-----------------------------

#-----------------------------
# Include files
#-----------------------------

set(PROJECT_INCLUDES
	"include;"
	"shaders;"
	"${FETCHCONTENT_BASE_DIR}/stb-src;"
	)
	
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDES})
#-----------------------------

#-----------------------------
# Libraries
#-----------------------------

set(PROJECT_LIBRARIES
	"imgui;"
	"glfw;"
	"glm;"
	"glad;"
  )

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBRARIES})