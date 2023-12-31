include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")
 
cmake_minimum_required(VERSION 3.11.0)
 
set(NAME engine)
 
message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH) 
      message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()
 
project(${NAME} VERSION 0.0.01)
 
# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
else()
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()
 
 
# 2. Set SDL2_PATH in .env.cmake to target specific sdl2
if (DEFINED SDL2_PATH)
  message(STATUS "Using SDL2 path specified in .env")
  set(SDL2_INCLUDE_DIRS "${SDL2_PATH}/include")
  set(SDL2_LIB "${SDL2_PATH}/lib/x64")
else()
  find_package(sdl2 2.0.22 REQUIRED)
  set(SDL2_LIB sdl2)
  message(STATUS "Found SDL2")
endif()
if (NOT SDL2_LIB)
    message(FATAL_ERROR "Could not find sdl2 library!")
else()
    message(STATUS "Using sdl2 lib at: ${SDL2_LIB}")
endif()

# 3. Set SDL2_IMAGE_PATH in .env.cmake to target specific sdl2
if (DEFINED SDL2_IMAGE_PATH)
  message(STATUS "Using SDL2_IMAGE path specified in .env")
  set(SDL2_IMAGE_INCLUDE_DIRS "${SDL2_IMAGE_PATH}/include")
  set(SDL2_IMAGE_LIB "${SDL2_IMAGE_PATH}/lib/x64")
else()
  find_package(sdl2_image 2.0.5 REQUIRED)
  set(SDL2_IMAGE_LIB sdl2)
  message(STATUS "Found SDL2_image")
endif()
if (NOT SDL2_IMAGE_LIB)
    message(FATAL_ERROR "Could not find sdl2_image library!")
else()
    message(STATUS "Using sdl2_image lib at: ${SDL2_IMAGE_LIB}")
endif()
 
include_directories(external)
 
# If TINYOBJ_PATH not specified in .env.cmake, try fetching from git repo
if (NOT TINYOBJ_PATH)
  message(STATUS "TINYOBJ_PATH not specified in .env.cmake, using external/tinyobjloader")
  set(TINYOBJ_PATH external/tinyobjloader)
endif()

# If STB_IMAGE not specified in .env.cmake, try fetching from git repo
if (NOT STB_IMAGE)
  message(STATUS "STB_IMAGE not specified in .env.cmake, using external/stb_image")
  set(STB_IMAGE external/stbimage)
endif()
 
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
 
add_executable(${PROJECT_NAME} ${SOURCES})
 
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE -g -O0)
 
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")
 
if (WIN32)
  message(STATUS "CREATING BUILD FOR WINDOWS")
 
  if (USE_MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/include
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
      ${MINGW_PATH}/lib
    )
  endif()
 
  target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/src
    ${Vulkan_INCLUDE_DIRS}
    ${TINYOBJ_PATH}
    ${STB_IMAGE}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GLM_PATH}
    )
 
  target_link_directories(${PROJECT_NAME} PUBLIC
    ${Vulkan_LIBRARIES}
    ${SDL2_LIB}
  )
 
  target_link_libraries(${PROJECT_NAME} sdl2 vulkan-1)
elseif (UNIX)
    message(STATUS "CREATING BUILD FOR UNIX")
    target_include_directories(${PROJECT_NAME} PUBLIC
      ${PROJECT_SOURCE_DIR}/src
      ${TINYOBJ_PATH}
    )
    target_link_libraries(${PROJECT_NAME} glfw ${Vulkan_LIBRARIES})
endif()
 
 
############## Build SHADERS #######################
 
# Find all vertex and fragment sources within shaders directory
# taken from VBlancos vulkan tutorial
# https://github.com/vblanco20-1/vulkan-guide/blob/all-chapters/CMakeLists.txt
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)
 
# get all .vert and .frag files in shaders directory
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)
 
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)
 
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)