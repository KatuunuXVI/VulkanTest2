cmake_minimum_required(VERSION 3.20)
project(VulkanTest2 C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O3 -Wall -Wextra")
find_package(OpenAL REQUIRED)
message("${CMAKE_BUILD_TYPE}")
message("Ur Mom")

add_executable(VulkanSample main.c Extools.c Extools.h Display.c Display.h Textures.c Textures.h lodepng.c   )
target_link_libraries(VulkanSample vulkan glfw3 OpenAL32 ogg vorbis )

