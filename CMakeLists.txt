# CMakeLists.txt for Pou (Wii U Port & Desktop Simulation)
cmake_minimum_required(VERSION 3.15)
project(Pou VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB_RECURSE POU_SOURCES
    "src/*.cpp"
    "src/*.c"
)

# Header include paths
include_directories(
    src
    src/core
    src/entities
    src/rooms
    src/games
    src/ui
)

if(WIIU)
    # Wii U Toolchain build (WUT)
    add_definitions(-D__WIIU__ -D__WUT__)

    find_package(WUT REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)

    wut_add_rpx(Pou ${POU_SOURCES})

    target_include_directories(Pou PRIVATE
        ${WUT_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
    )

    target_link_libraries(Pou PRIVATE
        ${SDL2_MIXER_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_LIBRARIES}
        wut::whb
        wut::coreinit
        wut::vpad
        wut::sndcore2
        m
    )

    # Copy output binary to code/Pou.rpx
    add_custom_command(TARGET Pou POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/code
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Pou> ${CMAKE_CURRENT_SOURCE_DIR}/code/Pou.rpx
    )
else()
    # Desktop PC Simulation Build (Windows / Linux / macOS)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)

    add_executable(PouSimulator ${POU_SOURCES})

    target_include_directories(PouSimulator PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
    )

    target_link_libraries(PouSimulator PRIVATE
        SDL2::SDL2
        SDL2_image::SDL2_image
        SDL2_ttf::SDL2_ttf
        SDL2_mixer::SDL2_mixer
    )
endif()
