cmake_minimum_required(VERSION 3.15)

project(robotCommand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

add_executable(robotCommand
    src/main.cpp
    src/robot.cpp
    src/jsonrpctcpclient.cpp
)

find_package(jsoncpp REQUIRED)
target_link_libraries(robotCommand jsoncpp_lib)
if (WIN32)
    target_link_libraries(robotCommand ws2_32)
endif ()

target_compile_options(robotCommand PRIVATE
  "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>"
  "$<${msvc_cxx}:-W3>"
)

#add_compile_definitions(JSONRPC_DEBUG)
if (WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0501)
endif ()
