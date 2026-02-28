cmake_minimum_required(VERSION 3.16)
project(chdcalc C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable static linking
set(CMAKE_EXE_LINKER_FLAGS "-static")

include(FetchContent)

FetchContent_Declare(
    libchdr
    GIT_REPOSITORY https://github.com/rtissera/libchdr.git
    GIT_TAG master
)

FetchContent_Declare(
    rcheevos
    GIT_REPOSITORY https://github.com/RetroAchievements/rcheevos.git
    GIT_TAG master
)

FetchContent_Declare(
    libretro-common
    GIT_REPOSITORY https://github.com/libretro/libretro-common.git
    GIT_TAG master
)

set(CHD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CHD_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(libchdr rcheevos libretro-common)

# Build cdfs from libretro-common
add_library(cdfs STATIC
    ${libretro-common_SOURCE_DIR}/formats/cdfs/cdfs.c
    ${libretro-common_SOURCE_DIR}/streams/file_stream.c
    ${libretro-common_SOURCE_DIR}/streams/interface_stream.c
    ${libretro-common_SOURCE_DIR}/streams/memory_stream.c
    ${libretro-common_SOURCE_DIR}/streams/chd_stream.c
    ${libretro-common_SOURCE_DIR}/streams/trans_stream.c
    ${libretro-common_SOURCE_DIR}/streams/trans_stream_zlib.c
    ${libretro-common_SOURCE_DIR}/streams/file_stream_transforms.c
    ${libretro-common_SOURCE_DIR}/streams/rzip_stream.c
    ${libretro-common_SOURCE_DIR}/streams/trans_stream_pipe.c
    ${libretro-common_SOURCE_DIR}/streams/network_stream.c
    ${libretro-common_SOURCE_DIR}/streams/stdin_stream.c
    ${libretro-common_SOURCE_DIR}/compat/compat_strl.c
    ${libretro-common_SOURCE_DIR}/compat/compat_strldup.c
    ${libretro-common_SOURCE_DIR}/file/file_path.c
    ${libretro-common_SOURCE_DIR}/file/file_path_io.c
    ${libretro-common_SOURCE_DIR}/vfs/vfs_implementation.c
    ${libretro-common_SOURCE_DIR}/string/stdstring.c
    ${libretro-common_SOURCE_DIR}/encodings/encoding_crc32.c
    ${libretro-common_SOURCE_DIR}/encodings/encoding_utf.c
    ${libretro-common_SOURCE_DIR}/encodings/encoding_base64.c
    ${libretro-common_SOURCE_DIR}/dynamic/dylib.c
    ${libretro-common_SOURCE_DIR}/time/rtime.c
)

target_include_directories(cdfs PRIVATE 
    ${libretro-common_SOURCE_DIR}/include
    ${libchdr_SOURCE_DIR}/include
)

target_compile_definitions(cdfs PRIVATE HAVE_CHD)
target_link_libraries(cdfs PRIVATE chdr-static)

# Build rcheevos manually
add_library(rhash STATIC
    ${rcheevos_SOURCE_DIR}/src/rc_util.c
    ${rcheevos_SOURCE_DIR}/src/rhash/hash.c
    ${rcheevos_SOURCE_DIR}/src/rhash/hash_disc.c
    ${rcheevos_SOURCE_DIR}/src/rhash/hash_encrypted.c
    ${rcheevos_SOURCE_DIR}/src/rhash/hash_rom.c
    ${rcheevos_SOURCE_DIR}/src/rhash/hash_zip.c
    ${rcheevos_SOURCE_DIR}/src/rhash/md5.c
    ${rcheevos_SOURCE_DIR}/src/rhash/aes.c
    ${rcheevos_SOURCE_DIR}/src/rhash/cdreader.c
)

target_include_directories(rhash PUBLIC 
    ${rcheevos_SOURCE_DIR}/include
)

add_executable(chdhash main.c)

target_include_directories(chdhash PRIVATE 
    ${libchdr_SOURCE_DIR}/include
    ${rcheevos_SOURCE_DIR}/include
    ${libretro-common_SOURCE_DIR}/include
)

target_link_libraries(chdhash PRIVATE 
    chdr-static
    rhash
    cdfs
    m
    zlibstatic
    libzstd_static
    lzma
)
