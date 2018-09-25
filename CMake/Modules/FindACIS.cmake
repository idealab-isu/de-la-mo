# FindACIS
# --------
#
# Find Spatial Corp.'s 3D ACIS Modeler Solid Modeling Kernel includes and libraries
#

# Required CMake Flags when NOT using Visual Studio
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Prepare library search paths
set(_ACIS_SEARCH_PATHS)

# Users can set ACIS_ROOT before calling find_package() for custom ACIS install directories
if(ACIS_ROOT)
	set(_ACIS_SEARCH_ROOT PATHS ${ACIS_ROOT} NO_DEFAULT_PATH)
	list(APPEND _ACIS_SEARCH_PATHS _ACIS_SEARCH_ROOT)
endif()

# Use the A3DT environmental variable and the default install locations
set(_ACIS_SEARCH_NORMAL
	PATHS 
		"$ENV{A3DT}"
		"$ENV{PROGRAMFILES}/Spatial/acisR26"
		"$ENV{PROGRAMFILES}/Spatial/acis2017.1.0.0"
)
list(APPEND _ACIS_SEARCH_PATHS _ACIS_SEARCH_NORMAL)

# Find ACIS include directory
foreach(search ${_ACIS_SEARCH_PATHS})
	find_path(ACIS_INCLUDE_DIR NAMES acis.hxx ${${search}} PATH_SUFFIXES include)
endforeach()

# Try to find ACIS ARCH using the environmental variables
if(DEFINED ENV{ARCH})
	set(ACIS_ARCH "$ENV{ARCH}")
else()
	if(WIN32)
		# Automatically set ACIS ARCH value according to the CMake generator name
		if(CMAKE_GENERATOR STREQUAL "Visual Studio 14 2015 Win64")
			set(ACIS_ARCH "NT_VC14_64_DLL")
		endif()
		if(CMAKE_GENERATOR STREQUAL "Visual Studio 12 2013 Win64")
			set(ACIS_ARCH "NT_VC12_64_DLL")
		endif()
	endif()
	if(UNIX AND NOT APPLE)
		set(ACIS_ARCH "linux_a64")
	endif()
	if(APPLE)
		set(ACIS_ARCH "macos_b64")
	endif()
endif()

# Find ACIS library
if(NOT ACIS_LIBRARY)
  foreach(search ${_ACIS_SEARCH_PATHS})
    if (WIN32)
      find_library(ACIS_LIBRARY_DEBUG NAMES SpaACISd ${${search}} PATH_SUFFIXES ${ACIS_ARCH}D/code/lib ${ACIS_ARCH}/code/bin)
    endif()
    find_library(ACIS_LIBRARY_RELEASE NAMES	SpaACIS ${${search}} PATH_SUFFIXES ${ACIS_ARCH}/code/lib ${ACIS_ARCH}/code/bin)
  endforeach()
endif()

# Use SELECT_LIBRARY_CONFIGURATIONS() to find the debug and optimized ACIS library
include(SelectLibraryConfigurations)
SELECT_LIBRARY_CONFIGURATIONS(ACIS)

# ACIS requires the Threads package
find_package(Threads REQUIRED)

if(ACIS_FOUND)
	# Print library found status message
	message(STATUS "Found ACIS: ${ACIS_FOUND}")
	message(STATUS "ACIS Arch: ${ACIS_ARCH}")
	set(ACIS_INCLUDE_DIRS ${ACIS_INCLUDE_DIR})
	# Set a variable to be used for linking ACIS and Threads to the project
	set(ACIS_TARGET_LINK_LIBRARIES ${ACIS_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
else(ACIS_FOUND)
    message(FATAL_ERROR "ACIS not found!")
endif()

# These are some internal variables and they should be muted
mark_as_advanced(
	ACIS_ARCH
	ACIS_LIBRARY_RELEASE
	ACIS_LIBRARY_DEBUG
	ACIS_INCLUDE_DIR
)

# Unset the variables that we won't use anymore
unset(_ACIS_SEARCH_ROOT)
unset(_ACIS_SEARCH_NORMAL)
unset(_ACIS_SEARCH_PATHS)
