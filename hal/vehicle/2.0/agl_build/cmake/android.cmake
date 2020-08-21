set(trout_ANDROID_SYSCORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/android/system_core")

set(_trout_ANDROID_CXX_FLAGS -Wall -Werror -Wextra -std=c++17)

set(trout_ANDROID_LIBLOG_DIR ${trout_ANDROID_SYSCORE_DIR}/liblog)
set(trout_ANDROID_LIBLOG_INCLUDE_DIR ${trout_ANDROID_LIBLOG_DIR}/include)
set(trout_ANDROID_LIBLOG_LIBRARY "android_liblog")

set(trout_ANDROID_LIBBASE_DIR ${trout_ANDROID_SYSCORE_DIR}/base)
set(trout_ANDROID_LIBBASE_INCLUDE_DIR ${trout_ANDROID_LIBBASE_DIR}/include)
set(trout_ANDROID_LIBBASE_LIBRARY "android_libbase")

set(trout_ANDROID_LIBUTILS_DIR ${trout_ANDROID_SYSCORE_DIR}/libutils)
set(trout_ANDROID_LIBUTLS_INCLUDE_DIR ${trout_ANDROID_LIBUTILS_DIR}/include)
set(trout_ANDROID_LIBUTILS_LIBRARY "android_libutils")

set(trout_ANDROID_LIBCUTLS_INCLUDE_DIR ${trout_ANDROID_SYSCORE_DIR}/libcutils/include)


# =========== libbase =================

add_library(${trout_ANDROID_LIBBASE_LIBRARY}
    ${trout_ANDROID_LIBBASE_DIR}/liblog_symbols.cpp
    ${trout_ANDROID_LIBBASE_DIR}/logging.cpp
    ${trout_ANDROID_LIBBASE_DIR}/strings.cpp
    ${trout_ANDROID_LIBBASE_DIR}/threads.cpp
)

target_include_directories(${trout_ANDROID_LIBBASE_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
    PRIVATE ${trout_FMTLIB_INCLUDE_DIRS}
)

target_link_libraries(${trout_ANDROID_LIBBASE_LIBRARY}
    ${trout_ANDROID_LIBLOG_LIBRARY}
    ${trout_FMTLIB_LIBRARIES}
)

target_compile_options(${trout_ANDROID_LIBBASE_LIBRARY} PRIVATE ${_trout_ANDROID_CXX_FLAGS})


# =========== liblog =================

add_library(${trout_ANDROID_LIBLOG_LIBRARY}
    ${trout_ANDROID_LIBLOG_DIR}/logger_write.cpp
    ${trout_ANDROID_LIBLOG_DIR}/properties.cpp
)

target_include_directories(${trout_ANDROID_LIBLOG_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBLOG_INCLUDE_DIR}
    PRIVATE ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
    PRIVATE ${trout_ANDROID_LIBCUTLS_INCLUDE_DIR}
)

target_compile_options(${trout_ANDROID_LIBLOG_LIBRARY} PRIVATE ${_trout_ANDROID_CXX_FLAGS})


# =========== libutils =================

add_library(${trout_ANDROID_LIBUTILS_LIBRARY}
     ${trout_ANDROID_LIBUTILS_DIR}/SystemClock.cpp
     ${trout_ANDROID_LIBUTILS_DIR}/Timers.cpp
)

target_include_directories(${trout_ANDROID_LIBUTILS_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBUTLS_INCLUDE_DIR}
    PRIVATE ${trout_ANDROID_LIBCUTLS_INCLUDE_DIR}
)

target_link_libraries(${trout_ANDROID_LIBUTILS_LIBRARY}
    ${trout_ANDROID_LIBLOG_LIBRARY}
)

target_compile_options(${trout_ANDROID_LIBUTILS_LIBRARY} PRIVATE ${_trout_ANDROID_CXX_FLAGS})



# =========== export libraries =================

set(trout_ANDROID_INCLUDE_DIRS
    ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
)

set(trout_ANDROID_LIBRARIES
	${trout_ANDROID_LIBBASE_LIBRARY}
	${trout_ANDROID_LIBLOG_LIBRARY}
	${trout_ANDROID_LIBUTILS_LIBRARY}
)
