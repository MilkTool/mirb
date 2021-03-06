cmake_minimum_required(VERSION 2.8)
project (mirb)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall -Wextra -Wunused")
add_definitions(-DMIRB_THREADED -DMIRB_GC_SKIP_BLOCKS -DDEBUG)
include_directories("${mirb_SOURCE_DIR}/../Prelude/include", "${mirb_SOURCE_DIR}/../Libraries/pcre", "${mirb_SOURCE_DIR}/../Libraries/libtommath")
link_directories("${mirb_SOURCE_DIR}/../Libraries/pcre", "${mirb_SOURCE_DIR}/../Libraries/libtommath")

file(GLOB MIRB_SOURCES
    "src/*.cpp"
    "src/classes/*.cpp"
    "src/platform/*.cpp"
    "src/codegen/*.cpp"
    "src/generic/*.cpp"
    "src/lexer/*.cpp"
    "src/modules/*.cpp"
    "src/parser/*.cpp"
    "src/tree/*.cpp"
)

add_executable(mirb ${MIRB_SOURCES})
target_link_libraries(mirb pcre tommath)

set(CMAKE_CXX_FLAGS_ADDRESSSANITIZER "-fno-omit-frame-pointer -faddress-sanitizer -O1 -g")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	target_link_libraries(mirb c++abi)
endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
