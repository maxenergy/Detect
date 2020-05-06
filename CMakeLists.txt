cmake_minimum_required (VERSION 2.6)
project (Detect)
add_definitions(-D Linux -D __LINUX__ )
set (Developing_VERSION_MAJOR 1)
set (Developing_VERSION_MINOR 0)
set( CMAKE_CXX_FLAGS "-std=c++11 -O3")
set( lib_SDK
   -lhcnetsdk
   -lPlayCtrl
   -lAudioRender
   -lSuperRender
   -ltempsdk
   -lUlirNetDevSDK
   -pthread
    )


find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:") 
message(STATUS "    version: ${OpenCV_VERSION}") 
message(STATUS "    libraries: ${OpenCV_LIBS}") 
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories("./src")
include_directories("./src/lib")
include_directories("./src/example")
include_directories("./sdk64/include")
include_directories("../Socket_connect_v2/socket_connect_v2")
include_directories("../Socket_connect_v2/socket_connect_v2/base")
include_directories("../Socket_connect_v2/socket_connect_v2/systems")
include_directories("../Socket_connect_v2/socket_connect_v2/types")
#include_directories("../socket_connect/lib")
#include_directories("../socket_connect/src/client")
#include_directories("../socket_connect/src/server")
#include_directories("../socket_connect/src/relay_server")
#include_directories("../socket_connect/src/developing")

link_directories(${PROJECT_SOURCE_DIR}/sdk64)
link_directories(${PROJECT_SOURCE_DIR}/sdk64/HCNetSDKCom)

file(GLOB cpp "./src/*.cpp" , "./src/lib/*.cpp")

add_library(
                test-lib
                "./sdk64/include/GetStream.cpp"
                ${cpp}
                ../Socket_connect_v2/socket_connect_v2/socket_connect_v2.cpp
                ../Socket_connect_v2/socket_connect_v2/base/socket_base.cpp
                ../Socket_connect_v2/socket_connect_v2/systems/imp.cpp
                ../Socket_connect_v2/socket_connect_v2/systems/log.cpp
                ../Socket_connect_v2/socket_connect_v2/types/frame.cpp
                ../Socket_connect_v2/socket_connect_v2/types/type_base.cpp
                ../Socket_connect_v2/socket_connect_v2/types/types.cpp
)


# add the executable
add_executable(main_fire ./src/example/main_fire.cpp)
add_executable(main_flash ./src/example/main_flash.cpp)
add_executable(main_heat ./src/example/main_heat.cpp)
add_executable(main_water ./src/example/main_water.cpp)
add_executable(main_detect ./src/example/main_detect.cpp)


target_link_libraries(main_fire test-lib ${OpenCV_LIBS} ${lib_SDK})
target_link_libraries(main_flash test-lib ${OpenCV_LIBS} ${lib_SDK})
target_link_libraries(main_heat test-lib ${OpenCV_LIBS} ${lib_SDK})
target_link_libraries(main_water test-lib ${OpenCV_LIBS} ${lib_SDK})
target_link_libraries(main_detect test-lib ${OpenCV_LIBS} ${lib_SDK})

