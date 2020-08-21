set(trout_ANDROID_SYSCORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/android/system_core")

set(_trout_ANDROID_CXX_FLAGS "-Wall -Wno-unknown-pragmas -Wno-attributes -Werror -Wno-ignored-attributes -Wextra -std=gnu++17 -DPAGE_SIZE=4096")

set(trout_ANDROID_LIBLOG_DIR ${trout_ANDROID_SYSCORE_DIR}/liblog)
set(trout_ANDROID_LIBLOG_INCLUDE_DIR ${trout_ANDROID_LIBLOG_DIR}/include)
set(trout_ANDROID_LIBLOG_LIBRARY "android_liblog")

set(trout_ANDROID_LIBBASE_DIR ${trout_ANDROID_SYSCORE_DIR}/base)
set(trout_ANDROID_LIBBASE_INCLUDE_DIR ${trout_ANDROID_LIBBASE_DIR}/include)
set(trout_ANDROID_LIBBASE_LIBRARY "android_libbase")

set(trout_ANDROID_LIBUTILS_DIR ${trout_ANDROID_SYSCORE_DIR}/libutils)
set(trout_ANDROID_LIBUTLS_INCLUDE_DIR ${trout_ANDROID_LIBUTILS_DIR}/include)
set(trout_ANDROID_LIBUTILS_LIBRARY "android_libutils")


# =========== libbase =================

add_library(${trout_ANDROID_LIBBASE_LIBRARY}
    ${trout_ANDROID_LIBBASE_DIR}/liblog_symbols.cpp
    ${trout_ANDROID_LIBBASE_DIR}/logging.cpp
    ${trout_ANDROID_LIBBASE_DIR}/strings.cpp
    ${trout_ANDROID_LIBBASE_DIR}/threads.cpp
)

target_include_directories(${trout_ANDROID_LIBBASE_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
)

target_link_libraries(${trout_ANDROID_LIBBASE_LIBRARY}
    ${trout_ANDROID_LIBLOG_LIBRARY}
)

set_target_properties(${trout_ANDROID_LIBBASE_LIBRARY} PROPERTIES COMPILE_FLAGS ${_trout_ANDROID_CXX_FLAGS})


# =========== liblog =================

add_library(${trout_ANDROID_LIBLOG_LIBRARY}
    ${trout_ANDROID_LIBLOG_DIR}/logger_write.cpp
    ${trout_ANDROID_LIBLOG_DIR}/properties.cpp
)

target_include_directories(${trout_ANDROID_LIBLOG_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBLOG_INCLUDE_DIR}
    PRIVATE ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/android-include/libcutils"
)

set_target_properties(${trout_ANDROID_LIBLOG_LIBRARY} PROPERTIES COMPILE_FLAGS ${_trout_ANDROID_CXX_FLAGS})


# =========== libutils =================

add_library(${trout_ANDROID_LIBUTILS_LIBRARY}
     ${trout_ANDROID_LIBUTILS_DIR}/SystemClock.cpp
     ${trout_ANDROID_LIBUTILS_DIR}/Timers.cpp
)

target_include_directories(${trout_ANDROID_LIBUTILS_LIBRARY}
    PUBLIC ${trout_ANDROID_LIBUTLS_INCLUDE_DIR}
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/android-include/libcutils"
)

target_link_libraries(${trout_ANDROID_LIBUTILS_LIBRARY}
    ${trout_ANDROID_LIBLOG_LIBRARY}
)

set_target_properties(${trout_ANDROID_LIBUTILS_LIBRARY} PROPERTIES COMPILE_FLAGS ${_trout_ANDROID_CXX_FLAGS})



# =========== export libraries =================

set(trout_ANDROID_INCLUDE_DIRS
    ${trout_ANDROID_LIBBASE_INCLUDE_DIR}
)

set(trout_ANDROID_LIBRARIES
	${trout_ANDROID_LIBBASE_LIBRARY}
	${trout_ANDROID_LIBLOG_LIBRARY}
	${trout_ANDROID_LIBUTILS_LIBRARY}
)
