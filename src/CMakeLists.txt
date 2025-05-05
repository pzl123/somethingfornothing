file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
    "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.cc" "${PROJECT_SOURCE_DIR}/src/*.cxx"
)
add_executable(${EXECUTABLE_NAME} ${SOURCES})

option(WARNING_ERROR "Complier -Werror" ON)
# warnings
if(WARNING_ERROR)
    target_compile_options(${EXECUTABLE_NAME} PRIVATE
        -Wall
        -Wextra
        -Wshadow
        -pedantic
    )
endif()

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${EXECUTABLE_NAME}
    pthread
)