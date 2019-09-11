cmake_minimum_required (VERSION 2.6)
project (developing)
add_definitions(-D Linux -D __LINUX__ )
set (Developing_VERSION_MAJOR 1)
set (Developing_VERSION_MINOR 0)
set( CMAKE_CXX_FLAGS "-std=c++11 -O3")

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV library status:") 
message(STATUS "    version: ${OpenCV_VERSION}") 
message(STATUS "    libraries: ${OpenCV_LIBS}") 
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories("./src")
include_directories("./src/lib")
include_directories("./src/example")
include_directories("./sdk64/include")

link_directories(${PROJECT_SOURCE_DIR}/sdk64)
link_directories(${PROJECT_SOURCE_DIR}/sdk64/HCNetSDKCom)

file(GLOB cpp "./src/*.cpp")
file(GLOB cpp2 "./src/lib/*.cpp")
add_library(
                test-lib
                "./sdk64/include/GetStream.cpp"
                ${cpp}
                ${cpp2}
)


# add the executable
add_executable(main_fire ./src/example/main_fire.cpp)
add_executable(main_flash ./src/example/main_flash.cpp)
add_executable(main_heat ./src/example/main_heat.cpp)
add_executable(main_water ./src/example/main_water.cpp)


target_link_libraries(main_fire test-lib ${OpenCV_LIBS} -lhcnetsdk -lPlayCtrl -lAudioRender -lSuperRender -ltempsdk -lUlirNetDevSDK)
target_link_libraries(main_flash test-lib ${OpenCV_LIBS} -lhcnetsdk -lPlayCtrl -lAudioRender -lSuperRender -ltempsdk -lUlirNetDevSDK)
target_link_libraries(main_heat test-lib ${OpenCV_LIBS} -lhcnetsdk -lPlayCtrl -lAudioRender -lSuperRender -ltempsdk -lUlirNetDevSDK)
target_link_libraries(main_water test-lib ${OpenCV_LIBS} -lhcnetsdk -lPlayCtrl -lAudioRender -lSuperRender -ltempsdk -lUlirNetDevSDK)

# -ltempsdk -lUlirNetDevSDK
