cmake_minimum_required(VERSION 3.22)
## Use the variable PROJECT_NAME for changing the target name
set( PROJECT_NAME "blockchain" )

## Set our project name
project(${PROJECT_NAME})

## Use all the *.cpp files we found under this folder for the project
FILE(GLOB SRCS "${CMAKE_CURRENT_LIST_DIR}/src/blockchain/*.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/blockchain/storage/*.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/blockchain/net/*.cpp"
                "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

## Define the executable
add_executable(${PROJECT_NAME} ${SRCS})

target_link_libraries(${PROJECT_NAME} ssl crypto pthread)

