cmake_minimum_required(VERSION 3.10)
project(M88 VERSION 2.51 LANGUAGES CXX)

# C++11を使用（VS2022対応）
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ビルドタイプのデフォルト設定
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# プラットフォーム検出
if(WIN32)
    set(PLATFORM "Windows")
    set(PLATFORM_WIN32 ON)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM "Linux")
    set(PLATFORM_LINUX ON)
elseif(APPLE)
    set(PLATFORM "macOS")
    set(PLATFORM_MACOS ON)
else()
    set(PLATFORM "Unknown")
endif()

message(STATUS "")
message(STATUS "====================================")
message(STATUS "M88 - PC-8801 Emulator")
message(STATUS "====================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Platform: ${PLATFORM}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "====================================")
message(STATUS "")

# ソースファイルのリスト
# ====================================
# Common（プラットフォーム非依存）
# ====================================
set(COMMON_SOURCES
    src/common/device.cpp
    src/common/error.cpp
    src/common/file.cpp
    src/common/lpf.cpp
    src/common/lz77d.cpp
    src/common/memmgr.cpp
    src/common/schedule.cpp
    src/common/sndbuf2.cpp
    src/common/soundbuf.cpp
    src/common/srcbuf.cpp
    src/common/status.cpp
)

# ====================================
# Devices（Z80, OPNA, PSGなど）
# ====================================
set(DEVICES_SOURCES
    src/devices/fmgen.cpp
    src/devices/fmtimer.cpp
    src/devices/opm.cpp
    src/devices/opna.cpp
    src/devices/psg.cpp
    src/devices/Z80c.cpp
    src/devices/z80diag.cpp
    # Z80Debug.cpp and Z80Test.cpp excluded (debug/test only)
)

# Z80_x86.cpp は32bit x86専用なので、条件付きで追加
if(PLATFORM_WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
    list(APPEND DEVICES_SOURCES src/devices/Z80_x86.cpp)
    message(STATUS "Z80_x86.cpp enabled (32bit x86 build)")
else()
    message(STATUS "Z80_x86.cpp disabled (not 32bit x86)")
endif()

# ====================================
# PC88（PC-8801エミュレーションコア）
# ====================================
set(PC88_SOURCES
    src/pc88/base.cpp
    src/pc88/beep.cpp
    src/pc88/calender.cpp
    src/pc88/crtc.cpp
    src/pc88/diskmgr.cpp
    src/pc88/fdc.cpp
    src/pc88/fdu.cpp
    src/pc88/floppy.cpp
    src/pc88/intc.cpp
    # src/pc88/ioview.cpp  # Excluded: I/O viewer (debug/UI)
    src/pc88/joypad.cpp
    src/pc88/kanjirom.cpp
    src/pc88/memory.cpp
    # src/pc88/memview.cpp  # Excluded: Memory viewer (debug/UI)
    # src/pc88/mouse.cpp    # Excluded: Mouse (UI-dependent)
    src/pc88/opnif.cpp
    src/pc88/pc88.cpp
    src/pc88/pd8257.cpp
    src/pc88/pio.cpp
    src/pc88/screen.cpp
    src/pc88/sio.cpp
    src/pc88/sound.cpp
    src/pc88/subsys.cpp
    src/pc88/tapemgr.cpp
)

# ====================================
# Win32（Windows固有）
# ====================================
if(PLATFORM_WIN32)
    set(WIN32_SOURCES
        src/win32/88config.cpp
        src/win32/about.cpp
        src/win32/basmon.cpp
        src/win32/cfgpage.cpp
        src/win32/codemon.cpp
        src/win32/dderr.cpp
        src/win32/diag.cpp
        src/win32/disasm.cpp
        src/win32/diskerr.cpp
        src/win32/DrawD2D.cpp
        src/win32/DrawDDS.cpp
        src/win32/DrawGDI.cpp
        src/win32/drawddw.cpp
        src/win32/file.cpp
        src/win32/guid.cpp
        src/win32/joymgr.cpp
        src/win32/m88ini.cpp
        src/win32/mcc.cpp
        src/win32/misc.cpp
        src/win32/mmon.cpp
        src/win32/monitor.cpp
        src/win32/mouse.cpp
        src/win32/movie.cpp
        src/win32/msgmon.cpp
        src/win32/pio.cpp
        src/win32/regmon.cpp
        src/win32/resource.cpp
        src/win32/rmon.cpp
        src/win32/romeo/c86if.cpp
        src/win32/romeo/c86pcm.cpp
        src/win32/romeo/c86psg.cpp
        src/win32/schedule.cpp
        src/win32/sequence.cpp
        src/win32/soundds.cpp
        src/win32/soundds2.cpp
        src/win32/soundmon.cpp
        src/win32/soundwo.cpp
        src/win32/statusdisplay.cpp
        src/win32/tapemgr.cpp
        src/win32/timekeep.cpp
        src/win32/ui.cpp
        src/win32/windraw.cpp
        src/win32/wincfg.cpp
        src/win32/wincore.cpp
        src/win32/winsound.cpp
    )
endif()

# ====================================
# インクルードディレクトリ
# ====================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/pc88
    ${CMAKE_SOURCE_DIR}/src/if
)

if(PLATFORM_WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/src/win32)
endif()

# ====================================
# ライブラリまたは実行ファイルの作成
# ====================================

# まずはコア部分をライブラリとしてビルド
add_library(m88core STATIC
    ${COMMON_SOURCES}
    ${DEVICES_SOURCES}
    ${PC88_SOURCES}
)

# プリプロセッサ定義
if(PLATFORM_WIN32)
    target_compile_definitions(m88core PRIVATE
        _WINDOWS
        WIN32
        UNICODE
        _UNICODE
    )

    # Windows版の実行ファイル
    if(WIN32_SOURCES)
        add_executable(M88 WIN32 ${WIN32_SOURCES})
        target_link_libraries(M88 m88core)

        # Windows固有のリンクライブラリ
        target_link_libraries(M88
            winmm
            comctl32
            comdlg32
            ole32
            uuid
        )
    endif()
else()
    # Linux/macOS版 - SDL2ベース
    message(STATUS "Building SDL2-based version for non-Windows platforms")

    # SDL2の検索
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        message(STATUS "SDL2 found: ${SDL2_LIBRARIES}")
    else()
        message(FATAL_ERROR "SDL2 not found. Please install SDL2 development packages.")
    endif()

    # zlib の検索
    find_package(ZLIB REQUIRED)
    if(ZLIB_FOUND)
        message(STATUS "zlib found: ${ZLIB_LIBRARIES}")
    else()
        message(FATAL_ERROR "zlib not found. Please install zlib development packages.")
    endif()

    # SDL2版のソースファイル
    set(SDL2_SOURCES
        src/sdl2/main_sdl2.cpp
        src/sdl2/WinCoreSDL2.cpp
        src/sdl2/ConfigSDL2.cpp
        src/sdl2/DrawSDL2.cpp
        src/sdl2/KeyboardSDL2.cpp
        src/sdl2/SoundSDL2.cpp
    )

    # SDL2版の実行ファイル
    add_executable(m88 ${SDL2_SOURCES})

    # インクルードディレクトリ
    target_include_directories(m88 PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/common
        ${CMAKE_SOURCE_DIR}/src/devices
        ${CMAKE_SOURCE_DIR}/src/pc88
        ${CMAKE_SOURCE_DIR}/src/sdl2
        ${SDL2_INCLUDE_DIRS}
    )

    # ライブラリリンク
    target_link_libraries(m88
        m88core
        ${SDL2_LIBRARIES}
        ${ZLIB_LIBRARIES}
        pthread
        m
    )

    message(STATUS "SDL2 version configured: m88 executable")
endif()

# ====================================
# コンパイラフラグ
# ====================================
if(MSVC)
    # Visual Studio用の設定
    target_compile_options(m88core PRIVATE
        /W3
        /MP  # マルチプロセッサコンパイル
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(m88core PRIVATE
            /O2
            /GL  # プログラム全体の最適化
        )
    endif()
else()
    # GCC/Clang用の設定
    target_compile_options(m88core PRIVATE
        -Wall
        -Wno-unknown-pragmas
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(m88core PRIVATE -O2)
    endif()
endif()

# ====================================
# インストール
# ====================================
if(PLATFORM_WIN32 AND TARGET M88)
    install(TARGETS M88 RUNTIME DESTINATION bin)
endif()

install(TARGETS m88core ARCHIVE DESTINATION lib)

# ====================================
# サブディレクトリ（プロトタイプなど）
# ====================================
if(EXISTS "${CMAKE_SOURCE_DIR}/prototype/sdl2_test/CMakeLists.txt")
    add_subdirectory(prototype/sdl2_test)
endif()

# ====================================
# サマリー表示
# ====================================
list(LENGTH COMMON_SOURCES COMMON_COUNT)
list(LENGTH DEVICES_SOURCES DEVICES_COUNT)
list(LENGTH PC88_SOURCES PC88_COUNT)

message(STATUS "")
message(STATUS "====================================")
message(STATUS "Build Configuration Summary")
message(STATUS "====================================")
message(STATUS "Core library: m88core (STATIC)")
if(PLATFORM_WIN32 AND TARGET M88)
    message(STATUS "Windows GUI: M88 (executable)")
else()
    message(STATUS "GUI: Not built (platform-specific implementation needed)")
endif()
message(STATUS "")
message(STATUS "Source file counts:")
message(STATUS "  Common:  ${COMMON_COUNT} files")
message(STATUS "  Devices: ${DEVICES_COUNT} files")
message(STATUS "  PC88:    ${PC88_COUNT} files")
if(PLATFORM_WIN32)
    list(LENGTH WIN32_SOURCES WIN32_COUNT)
    message(STATUS "  Win32:   ${WIN32_COUNT} files")
endif()
message(STATUS "====================================")
message(STATUS "")
