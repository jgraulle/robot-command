cmake_minimum_required(VERSION 3.15)

project(robotCommand)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_VERBOSE_MAKEFILE ON)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")

include(ExternalProject)
ExternalProject_Add(Jsoncpp
    GIT_REPOSITORY git@github.com:open-source-parsers/jsoncpp.git
    GIT_TAG 1.9.5
    CMAKE_ARGS -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
)

ExternalProject_Add(libjson-rpc-cpp
    GIT_REPOSITORY git@github.com:cinemast/libjson-rpc-cpp.git
    GIT_TAG v1.4.1
    CMAKE_ARGS -DTCP_SOCKET_SERVER=YES -DTCP_SOCKET_CLIENT=YES -DHTTP_SERVER=NO
        -DHTTP_CLIENT=NO -DREDIS_SERVER=NO -DREDIS_CLIENT=NO -DCOMPILE_TESTS=NO
        -DCOMPILE_STUBGEN=NO -DCOMPILE_EXAMPLES=NO -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
)
ExternalProject_Add_StepDependencies(libjson-rpc-cpp build Jsoncpp)

link_directories(${CMAKE_BINARY_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/include)

add_executable(robotCommand
    src/main.cpp
    src/robot.cpp
)

target_link_libraries(robotCommand jsonrpccpp-common jsonrpccpp-client jsoncpp)
add_dependencies(robotCommand Jsoncpp libjson-rpc-cpp)

target_compile_options(robotCommand PRIVATE
  "$<${gcc_like_cxx}:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>"
  "$<${msvc_cxx}:-W3>"
)

#add_compile_definitions(COLLISION_DEBUG)
