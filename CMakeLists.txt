cmake_minimum_required(VERSION 4.0)
project(GameEngine)

include(FetchContent)
FETCHCONTENT_DECLARE(GLFW GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FETCHCONTENT_MAKEAVAILABLE(GLFW)

include(FetchContent)
FETCHCONTENT_DECLARE(GLM GIT_REPOSITORY https://github.com/g-truc/glm)
FETCHCONTENT_MAKEAVAILABLE(GLM)

include(FetchContent)
FETCHCONTENT_DECLARE(NLOHMANN_JSON GIT_REPOSITORY https://github.com/nlohmann/json.git GIT_TAG v3.11.3)
FETCHCONTENT_MAKEAVAILABLE(NLOHMANN_JSON)

include(FetchContent)
FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui GIT_TAG docking)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
)

find_package(Vulkan REQUIRED)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${Vulkan_INCLUDE_DIRS})
target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.h" "${CMAKE_SOURCE_DIR}/src/*.hpp")

set(HEADER_DIRS "")
foreach(header ${HEADER_FILES})
    get_filename_component(header_dir "${header}" DIRECTORY)
    list(APPEND HEADER_DIRS "${header_dir}")
endforeach()
list(REMOVE_DUPLICATES HEADER_DIRS)

add_executable(GameEngine ${SRC_FILES})

target_include_directories(GameEngine PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    ${HEADER_DIRS}
)

target_compile_definitions(GameEngine PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}/")

target_link_libraries(GameEngine PRIVATE glfw glm nlohmann_json::nlohmann_json Vulkan::Vulkan imgui)

add_custom_target(CompileShaders ALL
    COMMAND cmd /c ${CMAKE_SOURCE_DIR}/compile.bat
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)

add_dependencies(GameEngine CompileShaders)