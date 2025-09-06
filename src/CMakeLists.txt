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
        # -pedantic
    )
endif()

find_package(OpenSSL REQUIRED)

message(STATUS "OpenSSL found: ${OPENSSL_FOUND}")
message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
message(STATUS "Include dir: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "Libraries: ${OPENSSL_LIBRARIES}")

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
    ${PROJECT_SOURCE_DIR}/include/websocket
)


set(LIB_PATH ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(${EXECUTABLE_NAME}
    PRIVATE
        ${LIB_PATH}/libsqlite3.a
        ${LIB_PATH}/libhv_static.a
        ${LIB_PATH}/libwebsockets.a
        ${OPENSSL_LIBRARIES}
        ${LIB_PATH}/libfcgi.a
        ${LIB_PATH}/libcjsonx.a
        pthread
        dl
)