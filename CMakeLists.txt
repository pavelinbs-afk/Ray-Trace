cmake_minimum_required(VERSION 3.16)

# RayTrace — MetaMod:Source 2 plugin (CS2, KHook)
#
# Quick start (WSL):
#   bash build_cmake.sh
#
# Output: build/addons/RayTrace/bin/linuxsteamrt64/RayTrace.so

project(RayTrace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RT_OPTIMIZE "Enable -O3 in Release" ON)

function(rt_try_path OUT_VAR)
  set(${OUT_VAR} "" PARENT_SCOPE)
  foreach(candidate IN LISTS ARGN)
    if(candidate AND EXISTS "${candidate}")
      set(${OUT_VAR} "${candidate}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
endfunction()

if(NOT HL2SDK_PATH AND DEFINED ENV{HL2SDKCS2})
  set(HL2SDK_PATH "$ENV{HL2SDKCS2}")
endif()
if(NOT MMS_PATH AND DEFINED ENV{MMSOURCE112})
  set(MMS_PATH "$ENV{MMSOURCE112}")
elseif(NOT MMS_PATH AND DEFINED ENV{MMSOURCE_DEV})
  set(MMS_PATH "$ENV{MMSOURCE_DEV}")
endif()

set(_rt_roots
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/.."
  "${CMAKE_SOURCE_DIR}/../.."
)
foreach(_root IN LISTS _rt_roots)
  list(APPEND _rt_hl2sdk_candidates "${_root}/deps/hl2sdk-cs2")
  list(APPEND _rt_mms_candidates "${_root}/deps/metamod-source")
endforeach()

if(NOT HL2SDK_PATH OR NOT EXISTS "${HL2SDK_PATH}/public/tier0/dbg.h")
  rt_try_path(_found_hl2sdk ${_rt_hl2sdk_candidates})
  if(_found_hl2sdk)
    set(HL2SDK_PATH "${_found_hl2sdk}")
  endif()
endif()
if(NOT MMS_PATH OR NOT EXISTS "${MMS_PATH}/core/ISmmPlugin.h")
  rt_try_path(_found_mms ${_rt_mms_candidates})
  if(_found_mms)
    set(MMS_PATH "${_found_mms}")
  endif()
endif()

set(HL2SDK_PATH "${HL2SDK_PATH}" CACHE PATH "Path to hl2sdk-cs2")
set(MMS_PATH "${MMS_PATH}" CACHE PATH "Path to metamod-source")

if(NOT HL2SDK_PATH OR NOT EXISTS "${HL2SDK_PATH}/public/tier0/dbg.h")
  message(FATAL_ERROR "HL2SDK not found. Run: bash build_cmake.sh")
endif()
if(NOT MMS_PATH OR NOT EXISTS "${MMS_PATH}/core/ISmmPlugin.h")
  message(FATAL_ERROR "Metamod:Source not found. Run: bash build_cmake.sh")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(MMS_CORE "${MMS_PATH}/core")

# --- Version macros (plugin GetVersion) ------------------------------------
set(RT_GIT_SHA "Local")
if(DEFINED ENV{GITHUB_SHA_SHORT} AND NOT "$ENV{GITHUB_SHA_SHORT}" STREQUAL "")
  set(RT_GIT_SHA "$ENV{GITHUB_SHA_SHORT}")
else()
  find_package(Git QUIET)
  if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
      WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
      OUTPUT_VARIABLE RT_GIT_SHA
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
    )
  endif()
endif()

set(RT_SEMVER "Local")
if(DEFINED ENV{SEMVER} AND NOT "$ENV{SEMVER}" STREQUAL "")
  set(RT_SEMVER "$ENV{SEMVER}")
endif()

# --- Compile flags ---------------------------------------------------------
add_library(raytrace_flags INTERFACE)

target_compile_definitions(raytrace_flags INTERFACE
  META_IS_SOURCE2
  SOURCE_ENGINE=25
  SE_CS2=25
  GAME_DLL
  RAD_TELEMETRY_DISABLED
  _ITERATOR_DEBUG_LEVEL=0
  stricmp=strcasecmp
  _stricmp=strcasecmp
  _strnicmp=strncasecmp
  strnicmp=strncasecmp
  _snprintf=snprintf
  _vsnprintf=vsnprintf
  HAVE_STDINT_H
  GNUC
  GITHUB_SHA="${RT_GIT_SHA}"
  SEMVER="${RT_SEMVER}"
  PROJECT_NAMESPACE=RayTrace
  PROJECT_NAMESPACE_STR=RayTrace
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(raytrace_flags INTERFACE
    -pipe
    -fno-strict-aliasing
    -Wall
    -Wno-uninitialized
    -Wno-parentheses
    -Wno-unused
    -Wno-switch
    -Wno-sign-compare
    -Wno-attributes
    -Wno-invalid-offsetof
    -Wno-non-virtual-dtor
    -Wno-overloaded-virtual
    -Wno-conversion-null
    -Wno-write-strings
    -Wno-reorder
    -fno-threadsafe-statics
    -fvisibility=default
    -fno-omit-frame-pointer
  )
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(raytrace_flags INTERFACE
    LINUX
    _LINUX
    POSIX
    _FILE_OFFSET_BITS=64
    X64BITS
    PLATFORM_64BITS
    COMPILER_GCC
    _GLIBCXX_USE_CXX11_ABI=0
  )
  target_compile_options(raytrace_flags INTERFACE -fPIC -msse -mfpmath=sse)
  if(CMAKE_BUILD_TYPE STREQUAL "Release" AND RT_OPTIMIZE)
    target_compile_options(raytrace_flags INTERFACE -O3)
    target_compile_definitions(raytrace_flags INTERFACE NDEBUG)
  endif()
endif()

target_include_directories(raytrace_flags INTERFACE
  "${CMAKE_SOURCE_DIR}"
  "${CMAKE_SOURCE_DIR}/src"
  "${CMAKE_SOURCE_DIR}/public"
  "${CMAKE_SOURCE_DIR}/vendor"
  "${CMAKE_SOURCE_DIR}/vendor/nlohmann"
  "${CMAKE_SOURCE_DIR}/vendor/spdlog/include"
  "${MMS_CORE}"
  "${MMS_PATH}/third_party/khook/include"
  "${HL2SDK_PATH}"
  "${HL2SDK_PATH}/thirdparty/protobuf-3.21.8/src"
  "${HL2SDK_PATH}/public"
  "${HL2SDK_PATH}/public/engine"
  "${HL2SDK_PATH}/public/mathlib"
  "${HL2SDK_PATH}/public/tier0"
  "${HL2SDK_PATH}/public/tier1"
  "${HL2SDK_PATH}/public/entity2"
  "${HL2SDK_PATH}/public/game/server"
  "${HL2SDK_PATH}/public/schemasystem"
  "${HL2SDK_PATH}/game/shared"
  "${HL2SDK_PATH}/game/server"
  "${HL2SDK_PATH}/common"
)

# --- Protobuf (from hl2sdk common; Valve-extended descriptor first) --------
set(RT_PROTO_FILES
  "${HL2SDK_PATH}/common/valveextensions.proto"
  "${HL2SDK_PATH}/common/network_connection.proto"
  "${HL2SDK_PATH}/common/networkbasetypes.proto"
  "${HL2SDK_PATH}/common/netmessages.proto"
  "${HL2SDK_PATH}/common/engine_gcmessages.proto"
  "${HL2SDK_PATH}/common/source2_steam_stats.proto"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(RT_PROTOC "${HL2SDK_PATH}/devtools/bin/linux/protoc")
else()
  set(RT_PROTOC "${HL2SDK_PATH}/devtools/bin/protoc.exe")
endif()

if(NOT EXISTS "${RT_PROTOC}")
  message(FATAL_ERROR "protoc not found: ${RT_PROTOC}")
endif()

set(RT_PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${RT_PROTO_GEN_DIR}")

set(RT_PROTO_SRCS "")
set(RT_PROTO_HDRS "")
foreach(proto ${RT_PROTO_FILES})
  if(NOT EXISTS "${proto}")
    message(FATAL_ERROR "Missing proto: ${proto}")
  endif()
  get_filename_component(proto_name "${proto}" NAME_WE)
  set(out_cc "${RT_PROTO_GEN_DIR}/${proto_name}.pb.cc")
  set(out_h "${RT_PROTO_GEN_DIR}/${proto_name}.pb.h")
  list(APPEND RT_PROTO_SRCS "${out_cc}")
  list(APPEND RT_PROTO_HDRS "${out_h}")

  add_custom_command(
    OUTPUT "${out_cc}" "${out_h}"
    COMMAND "${RT_PROTOC}"
      --cpp_out=${RT_PROTO_GEN_DIR}
      "-I${HL2SDK_PATH}/common"
      "-I${HL2SDK_PATH}/thirdparty/protobuf-3.21.8/src"
      "${proto}"
    DEPENDS "${proto}"
    COMMENT "Generating ${proto_name}.pb.cc"
    VERBATIM
  )
endforeach()

add_custom_target(raytrace_protoc DEPENDS ${RT_PROTO_SRCS} ${RT_PROTO_HDRS})

# --- spdlog ----------------------------------------------------------------
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
set(SPDLOG_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_WERROR OFF CACHE BOOL "" FORCE)
# Shared plugin needs PIC for static libspdlog.a
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Must match CS2 / RayTrace ABI, otherwise load fails with undefined
# spdlog::...::set_pattern(std::string) (Ss vs cxx11 string).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_compile_definitions(_GLIBCXX_USE_CXX11_ABI=0)
endif()
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vendor/spdlog/CMakeLists.txt")
  message(FATAL_ERROR "vendor/spdlog missing. Run: git submodule update --init vendor/spdlog")
endif()
add_subdirectory(vendor/spdlog)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(spdlog PUBLIC _GLIBCXX_USE_CXX11_ABI=0)
endif()

# --- Sources ---------------------------------------------------------------
set(RT_PLUGIN_SRCS
  src/plugin.cpp
  src/shared.cpp
  src/raytrace.cpp
  src/gameconfig.cpp
  src/log.cpp
  src/listeners/listeners.cpp
  src/schema/schemasystem.cpp
  vendor/dynlibutils/module.cpp
)

set(RT_SDK_SRCS
  "${HL2SDK_PATH}/public/tier0/memoverride.cpp"
  "${HL2SDK_PATH}/tier1/keyvalues3.cpp"
  "${HL2SDK_PATH}/tier1/convar.cpp"
  "${HL2SDK_PATH}/entity2/entityidentity.cpp"
  "${HL2SDK_PATH}/entity2/entitysystem.cpp"
  "${HL2SDK_PATH}/entity2/entitykeyvalues.cpp"
)

add_library(RayTrace SHARED
  ${RT_PLUGIN_SRCS}
  ${RT_SDK_SRCS}
  ${RT_PROTO_SRCS}
)

add_dependencies(RayTrace raytrace_protoc)

target_link_libraries(RayTrace PRIVATE raytrace_flags spdlog)
target_include_directories(RayTrace PRIVATE "${RT_PROTO_GEN_DIR}")

set_target_properties(RayTrace PROPERTIES
  PREFIX ""
  OUTPUT_NAME "RayTrace"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(HL2SDK_LIB "${HL2SDK_PATH}/lib/linux64")
  set(RT_SDK_LIBS
    "${HL2SDK_LIB}/mathlib.a"
    "${HL2SDK_LIB}/interfaces.a"
    "${HL2SDK_LIB}/release/libprotobuf.a"
  )
  foreach(lib ${RT_SDK_LIBS})
    if(NOT EXISTS "${lib}")
      message(FATAL_ERROR "Missing HL2SDK library: ${lib}")
    endif()
  endforeach()

  target_link_libraries(RayTrace PRIVATE
    ${RT_SDK_LIBS}
    dl
    pthread
    m
  )

  target_link_options(RayTrace PRIVATE
    -static-libgcc
    -static-libstdc++
    -Wl,--exclude-libs=libprotobuf.a
  )

  set(RT_OUT_DIR "${CMAKE_BINARY_DIR}/addons/RayTrace/bin/linuxsteamrt64")
  set_target_properties(RayTrace PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${RT_OUT_DIR}"
  )

  set(RT_PKG_METAMOD "${CMAKE_BINARY_DIR}/addons/metamod")
  set(RT_PKG_CFG "${CMAKE_BINARY_DIR}/addons/RayTrace")
  add_custom_target(raytrace_package ALL
    DEPENDS RayTrace
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RT_PKG_METAMOD}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RT_PKG_CFG}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_SOURCE_DIR}/configs/addons/metamod/RayTrace.vdf" "${RT_PKG_METAMOD}/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_SOURCE_DIR}/configs/addons/RayTrace/gamedata.json" "${RT_PKG_CFG}/"
    COMMENT "Packaging addons/RayTrace layout"
  )
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME} (use WSL/Linux build_cmake.sh)")
endif()

message(STATUS "RayTrace: HL2SDK=${HL2SDK_PATH}")
message(STATUS "RayTrace: MMS=${MMS_PATH}")
message(STATUS "RayTrace: SEMVER=${RT_SEMVER} SHA=${RT_GIT_SHA}")
