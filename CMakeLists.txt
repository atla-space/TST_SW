cmake_minimum_required(VERSION 3.19)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/build/${CMAKE_BUILD_TYPE})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/out)

project(teststand LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API 2182bf5c-ef0d-489a-91da-49dbc3090d2a)
set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP 1)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-Wall -Wextra -pedantic)
    set(CMAKE_LINK_WHAT_YOU_USE ON)
    # find clang-tidy and cppcheck
    
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")
    if (${CLANG_TIDY_EXE_FOUND})
        set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXE} -checks=-*,readability-*,modernize-*,performance-*,clang-analyzer-*,cppcoreguidelines-*,bugprone-*,portability-*)
    endif()

    find_program(CPPCHECK_EXE NAMES "cppcheck" DOC "Path to cppcheck executable")
    if (${CPPCHECK_EXE_FOUND})
        set(CMAKE_CXX_CPPCHECK ${CPPCHECK_EXE} --enable=all --inconclusive)
    endif()
endif()


set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)


file(GLOB_RECURSE sources "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/modules/*.cpp") 
file(GLOB_RECURSE modules "${CMAKE_SOURCE_DIR}/modules/*.cppm") 

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_sources(${PROJECT_NAME}
    PRIVATE ${sources}
)
target_sources(${PROJECT_NAME}
    PUBLIC FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${modules}
)

# enable modules on GCC only
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -fmodules-ts -stdlib=libstdc++)
    target_link_options(${PROJECT_NAME} PRIVATE -fmodules-ts -stdlib=libstdc++)
endif()

# static link for the release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(${PROJECT_NAME} PRIVATE -static)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=leak)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=leak)
endif()

find_package(magic_enum CONFIG REQUIRED)
find_package(civetweb CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(simple-yaml CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)

find_package(PkgConfig)
pkg_check_modules(WIRINGPI REQUIRED libwiringpi2)
target_include_directories(${PROJECT_NAME} PRIVATE ${WIRINGPI_INCLUDE_DIRS})

target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/mariadb/)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    magic_enum::magic_enum
    civetweb::civetweb-cpp
    fmt::fmt
    simple-yaml::simple-yaml 
    range-v3::range-v3

    soci_core
    soci_mysql

PRIVATE
    ${wiringpi_LIBRARIES}
)
