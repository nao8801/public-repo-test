cmake_minimum_required(VERSION 3.10)
project(M88_SDL2_Prototype LANGUAGES CXX)

# C++11を使用
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2の検索
find_package(SDL2 REQUIRED)

# インクルードディレクトリ
include_directories(${SDL2_INCLUDE_DIRS})

# 数学ライブラリ（Linux用）
if(UNIX AND NOT APPLE)
    set(MATH_LIB m)
endif()

# 実行ファイルのビルド
add_executable(test_graphics test_graphics.cpp)
target_link_libraries(test_graphics ${SDL2_LIBRARIES} ${MATH_LIB})

add_executable(test_audio test_audio.cpp)
target_link_libraries(test_audio ${SDL2_LIBRARIES} ${MATH_LIB})

add_executable(test_audio_debug test_audio_debug.cpp)
target_link_libraries(test_audio_debug ${SDL2_LIBRARIES} ${MATH_LIB})

add_executable(test_audio_terminal test_audio_terminal.cpp)
target_link_libraries(test_audio_terminal ${SDL2_LIBRARIES} ${MATH_LIB})

add_executable(hal_demo hal_demo.cpp hal_sdl2_impl.cpp)
target_link_libraries(hal_demo ${SDL2_LIBRARIES} ${MATH_LIB})

# インストール設定（オプション）
install(TARGETS test_graphics test_audio test_audio_debug test_audio_terminal hal_demo
        RUNTIME DESTINATION bin)

# 情報表示
message(STATUS "")
message(STATUS "====================================")
message(STATUS "M88 SDL2 Prototype Configuration")
message(STATUS "====================================")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "SDL2 found: ${SDL2_FOUND}")
message(STATUS "SDL2 include dirs: ${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
message(STATUS "====================================")
message(STATUS "")
