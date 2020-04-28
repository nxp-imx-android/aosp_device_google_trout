if(NOT trout_FMTLIB_ROOT_DIR)
  set(trout_FMTLIB_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fmtlib)
endif()

if(EXISTS "${trout_FMTLIB_ROOT_DIR}/CMakeLists.txt")
  add_subdirectory(${trout_FMTLIB_ROOT_DIR})

  set(trout_FMTLIB_INCLUDE_DIRS "${trout_FMTLIB_ROOT_DIR}/include")
  set(trout_FMTLIB_LIBRARIES "fmt")
else()
    message(FATAL_ERROR "${trout_FMTLIB_ROOT_DIR}/CMakeLists.txt not found")
endif()

