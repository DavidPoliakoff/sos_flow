cmake_minimum_required (VERSION 2.8)

option(ENABLE_ADIOS_EXAMPLES "Build the examples that require the presence of ADIOS?"
       OFF)

option(ENABLE_TAU_WRAPPER "Compile the codes with tau_cc.sh?"
       OFF)

option(SOSD_DAEMON_DETACH "Have the daemon fork, detach, and get its own session?  (EXPERIMENTAL)"
       OFF)

option(SOSD_CLOUD_SYNC "Activate cloud_sync operations for the daemon?"
       ON)

option(SOSD_CLOUD_SYNC_WITH_MPI "Use MPI discover and synchronize data between daemons?"
       OFF)

option(SOSD_CLOUD_SYNC_WITH_EVPATH "Use EVPATH synchronize data between daemons?"
       ON)

###########
###########

if(SOSD_CLOUD_SYNC_WITH_MPI AND SOSD_CLOUD_SYNC_WITH_EVPATH)
  message(FATAL_ERROR
  "You cannot use both MPI and EVPATH for daemon cloud_sync operations!  Please select only one.")
endif()

if((SOSD_CLOUD_SYNC_WITH_MPI OR SOSD_CLOUD_SYNC_WITH_EVPATH) AND NOT SOSD_CLOUD_SYNC)
  message(FATAL_ERROR
  "You have specified a method for cloud_sync, but disabled cloud_sync.  Verify your build options.")
endif()

#set(RULE_MESSAGES OFF)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(PROJECT_NAME sos_flow)

set(PEDANTIC TRUE CACHE BOOL "Treat all compiler warnings as errors")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
add_definitions("-DGIT_SHA1=${GIT_SHA1}")
add_definitions("-DUSE_MPI=1")

project(${PROJECT_NAME} C CXX)
set(CMAKE_C_FLAGS "-std=gnu99 -DSOS_BUILDER=$ENV{USER} -DSOS_BUILT_FOR=${CMAKE_SYSTEM_NAME} -DSOS_HOST_NODE_NAME=$ENV{SOS_HOST_NODE_NAME} -DSOS_HOST_DETAILED=$ENV{SOS_HOST_DETAILED} -DSOS_HOST_KNOWN_AS=$ENV{SOS_HOST_KNOWN_AS} -DGIT_SHA1=${GIT_SHA1}")
set(CMAKE_PREFIX_PATH $ENV{HOME}/local/include $CMAKE_PREFIX_PATH)
set(CMAKE_LIBRARY_PATH $ENV{HOME}/local/lib $CMAKE_LIBRARY_PATH)
if(DEFINED SOS_SANITIZE_ADDRESS AND SOS_SANITIZE_ADDRESS)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif(DEFINED SOS_SANITIZE_ADDRESS AND SOS_SANITIZE_ADDRESS)
if(DEFINED SOS_SANITIZE_THREAD AND SOS_SANITIZE_THREAD)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=thread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=thread")
endif(DEFINED SOS_SANITIZE_THREAD AND SOS_SANITIZE_THREAD)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

if(ENABLE_TAU_WRAPPER)
  message(STATUS ">>> TAU:")
  message(STATUS ">>> TAU:  Building SOS components with TAU instrumentation (...of SOS itself).")
  message(STATUS ">>> TAU:")
  set (CMAKE_C_COMPILER /usr/local/tau-2.24.1/x86_64/bin/tau_cc.sh)
  add_definitions(-optMemDbg -optTrackPthread -optMpi -optTrackIO -optPDTInst)
  include_directories(/usr/local/tau-2.24.1/include)
  link_directories(/usr/local/tau-2.24.1/x86_64/lib)
endif()

include_directories($ENV{HOME}/local/include)

message(STATUS "Building on: ${CMAKE_SYSTEM_NAME}")

find_package(PkgConfig REQUIRED)
find_package("Sqlite3" REQUIRED)
# pkg_search_module(LIBTCMALLOC libtcmalloc)

#  Always include the MPI stuff, some components reference it
#  and not including it will cause the project not to build.
#  Despite the fact that we are including it, if you specify
#  EVPath as the communications framework, the SOS daemon
#  will not conduct any of its business using MPI.
message(STATUS ">>> MPI:  Locating package...")
find_package("MPI" REQUIRED QUIET)
if(MPI_FOUND)
message(STATUS ">>> MPI:  Found!")
endif(MPI_FOUND)

message(STATUS ">>> PYTHON:  Locating package...")
if(ENABLE_PYTHON)
    #pkg_search_module(PYTHON REQUIRED python QUIET)
    if(PYTHON_FOUND)
    message(STATUS ">>> PYTHON:  Found!")
    endif(PYTHON_FOUND)
endif(ENABLE_PYTHON)

if(SOSD_CLOUD_SYNC_WITH_MPI)
  add_definitions(-DSOSD_CLOUD_SYNC_WITH_MPI)
endif()

if(SOSD_CLOUD_SYNC_WITH_EVPATH)
  message(STATUS ">>> EVPATH:  Locating package...")
  #find_package("evpath" REQUIRED)
  #pkg_search_module(EVPATH REQUIRED libenet QUIET)
  #pkg_search_module(FABRIC REQUIRED libfabric)
  message(STATUS ">>> EVPATH:  Found!")
  message(STATUS ">>> EVPATH:  Using EVPath for communication...")
  add_definitions(-DSOSD_CLOUD_SYNC_WITH_EVPATH)
endif()

# set everything up to use -pthread
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
# check for Cray static linking
#if (CMAKE_C_COMPILER EQUAL "cc" AND CMAKE_CXX_COMPILER EQUAL "CC")
	find_library(DYNAMICLIB dl)
	set(LIBS ${LIBS} ${DYNAMICLIB})
#endif (CMAKE_C_COMPILER EQUAL "cc" AND CMAKE_CXX_COMPILER EQUAL "CC")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${sos_flow_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${sos_flow_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${sos_flow_BINARY_DIR}/lib")

subdirs (src tests examples/lulesh)

