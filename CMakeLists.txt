cmake_minimum_required(VERSION 3.20)
project(qr-gen VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Sanitizers configuration option
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_executable(qr-gen
    main.cpp
    qr_renderer.cpp
    qrcodegen.cpp
    tinypngout.cpp
)

target_include_directories(qr-gen PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Domain Unit Test executable and CTest registration
enable_testing()

add_executable(test_domain
    tests/test_domain.cpp
    qr_renderer.cpp
    qrcodegen.cpp
    tinypngout.cpp
)

target_include_directories(test_domain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_test(NAME unit_domain COMMAND test_domain)
add_test(NAME regression_suite COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_regression.sh)
