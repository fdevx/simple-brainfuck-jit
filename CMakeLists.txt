cmake_minimum_required(VERSION 3.27)
project(BrainfuckJIT)

set(ENABLE_TESTS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Use find LLVM cut down version of (https://layle.me/posts/using-llvm-with-cmake/)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

separate_arguments(LLVM_DEFINITIONS)

message(STATUS "LLVM libraries: ${LLVM_LIBRARIES}")
message(STATUS "LLVM includes: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")
message(STATUS "LLVM tools: ${LLVM_TOOLS_BINARY_DIR}")

add_executable(BrainfuckJIT src/main.cpp
        src/Parser.cpp
        src/Parser.h
        src/Lexer.cpp
        src/Lexer.h
        src/ExprAST.cpp
        src/ExprAST.h
        src/BrainfuckJIT.cpp
        src/BrainfuckJIT.h
        src/BrainfuckIRBuilder.cpp
        src/BrainfuckIRBuilder.h
        src/BrainfuckOptimizer.cpp
        src/BrainfuckOptimizer.h)

add_library(BrainfuckJITRuntime SHARED src/BrainfuckRuntime.cpp
        src/BrainfuckRuntime.h)

# configure llvm
#llvm_config(BrainfuckJIT USE_SHARED)
target_link_libraries(
        BrainfuckJIT
        PRIVATE
        BrainfuckJITRuntime
        LLVM
)

target_link_options(BrainfuckJIT PRIVATE "-fuse-ld=lld")
target_link_options(BrainfuckJITRuntime PRIVATE "-fuse-ld=lld")

if (ENABLE_TESTS)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    get_target_property(BrainfuckJIT_SOURCES BrainfuckJIT SOURCES)
    list(REMOVE_ITEM BrainfuckJIT_SOURCES src/main.cpp)

    add_executable(
            brainfuck_test
            test/brainfuck_test.cpp
            ${BrainfuckJIT_SOURCES}
    )

    target_link_libraries(
            brainfuck_test
            PRIVATE
            GTest::gtest_main
            BrainfuckJITRuntime
            LLVM
    )

    include(GoogleTest)
    gtest_discover_tests(brainfuck_test)

    target_link_options(brainfuck_test PRIVATE "-fuse-ld=lld")
endif ()

