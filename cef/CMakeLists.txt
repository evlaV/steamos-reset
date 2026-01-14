cmake_minimum_required(VERSION 3.18)

set(CMAKE_BUILD_TYPE Release)

project(holo-reset)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set_property(GLOBAL PROPERTY OS_FOLDERS ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Specify the CEF distribution version.
set(CEF_VERSION "98.1.19+g57be9e2+chromium-98.0.4758.80")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(CEF_PLATFORM "linux64_minimal")
  else()
    set(CEF_PLATFORM "linux32_minimal")
  endif()
else()
  message(FATAL_ERROR "Only linux build is supported")
endif()

include(DownloadCEF)
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${CMAKE_SOURCE_DIR}/third_party/cef")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

find_package(CEF REQUIRED)
add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)

list(APPEND CEF_C_COMPILER_FLAGS -fexceptions)
list(APPEND CEF_CXX_COMPILER_FLAGS -fexceptions)

# Target executable names.
set(APP_TARGET "holo-reset")
# Logical target used to link the libcef library.
ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
# Determine the target output directory.
SET_CEF_TARGET_OUT_DIR()

set(APP_SRCS
  src/app.cc
  src/app.h
  src/custom_scheme.cc
  src/custom_scheme.h
  src/handler.cc
  src/handler.h
  src/main.cc
  )

if(OS_LINUX)
  add_executable(${APP_TARGET} ${APP_SRCS})
  SET_EXECUTABLE_TARGET_PROPERTIES(${APP_TARGET})
  add_dependencies(${APP_TARGET} libcef_dll_wrapper)
  target_link_libraries(${APP_TARGET} app::rc libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

  # Set rpath so that libraries can be placed next to the executable.
  set_target_properties(${APP_TARGET} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib/holo-reset")
  set_target_properties(${APP_TARGET} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
  set_target_properties(${APP_TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CEF_TARGET_OUT_DIR}/bin")

  # Copy binary and resource files to the target output directory.
  COPY_FILES("${APP_TARGET}" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}/lib/holo-reset")
  COPY_FILES("${APP_TARGET}" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}/lib/holo-reset")
  if (EXISTS "${CEF_BINARY_DIR}/libminigbm.so")
    COPY_FILES("${APP_TARGET}" "libminigbm.so" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}/lib/holo-reset")
  endif()

  SET_LINUX_SUID_PERMISSIONS("${APP_TARGET}" "${CEF_TARGET_OUT_DIR}/bin/chrome-sandbox")
endif()

include(CMakeRC)
FILE(GLOB WEB_SRC web/*)
cmrc_add_resource_library(
  app-resources
  ALIAS app::rc
  NAMESPACE app

  ${WEB_SRC}
  )


PRINT_CEF_CONFIG()
