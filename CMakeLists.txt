cmake_minimum_required(VERSION 2.8)
project(delamo)

# Extend CMake module path for loading custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/CMake/Modules/")

# Set a build type if none specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'RelWithDebInfo' since none specified")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "MinSizeRel" "RelWithDebInfo")
endif()


# 'make commit' step
add_custom_target(commit make clean
                  COMMAND git add .
                  COMMAND git commit)
                  # DEPENDS clean)


# ----------------------------------------
# Set installation location rules

IF(APPLE)    # Apple
  SET(APP_INSTALL_PREFIX "~/install" CACHE PATH "Prefix prepended to install directories.  Must be writable by your user!")
ELSEIF(UNIX) # Linux (not Apple due to use of if/else-if construct)
  SET(APP_INSTALL_PREFIX "/usr/local" CACHE PATH "Prefix prepended to install directories.  Must be writable by your user!")
ENDIF()
IF(WIN32)
  SET(APP_INSTALL_DIR ${PROJECT_BINARY_DIR}/install/de-la-mo/ CACHE PATH "Installation directory.  Must be writable by your user!")
ELSE()
  SET(APP_INSTALL_DIR ${APP_INSTALL_PREFIX}/de-la-mo) # CACHE PATH "Installation directory.  Must be writable by your user!"
ENDIF()

SET(CMAKE_INSTALL_PREFIX "${APP_INSTALL_PREFIX}" CACHE PATH "Prefix prepended to install directories." FORCE)

#set_property(GLOBAL PROPERTY ADDITIONAL_MAKE_CLEAN_FILES abaqus.rpy abaqus1.rec abacus_acis.log genabqscript.py Block.SAT data.json)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
# !!! *** Should more carefully define appropriate RPATH's
set(CMAKE_INSTALL_RPATH "${APP_INSTALL_DIR};${APP_INSTALL_DIR}/delamo")








#############################################################################################################
# Configure the API
#   - Minimalist C++-based wrapper of the user-selected solid-modeling kernel.  Use of a
#     wrapper enables delamo to work with a variety of kernels.
#   - SWIG wrapper of the API to enable Python access
#

# ------------------------- #
#
# REQUIRED VARIABLES
#

# Silence MSVC warnings regarding to strdup, fopen, etc.
if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

# Required for GenerateExportHeader module
include(GenerateExportHeader)

# Activate GCC C++11 support for CMake versions before v3.1
if(CMAKE_VERSION VERSION_LESS 3.1)
	message(STATUS "Using CMake " ${CMAKE_VERSION})
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
			message(FATAL_ERROR "You need at least GCC v4.7")
		elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
			# Enable experimental C++11 support in GCC
			add_compile_options(-std=c++11)
		else(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
			# We are good to go!
		endif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
	endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
endif(CMAKE_VERSION VERSION_LESS 3.1)


## Python version to use with SWIG
#set(MODELBUILDER_PYTHON_VERSION "2.7" CACHE STRING "Python version to use")
#set_property(CACHE MODELBUILDER_PYTHON_VERSION PROPERTY STRINGS "2.7" "3.5")

## Set an internally variable to store the previously chosen Python version
#if(NOT DEFINED ${MODELBUILDER_CURRENT_PYTHON_VERSION})
#	set(MODELBUILDER_CURRENT_PYTHON_VERSION ${MODELBUILDER_PYTHON_VERSION} CACHE STRING "Previous Python Version")
#endif()

## Python version is changed, unset the PYTHON_* variables
#if (NOT ${MODELBUILDER_CURRENT_PYTHON_VERSION} STREQUAL ${MODELBUILDER_PYTHON_VERSION})
#	# Display a status message and unset the internal variable
#	message(STATUS "Unsetting old Python library paths...")
#	unset(MODELBUILDER_CURRENT_PYTHON_VERSION)
#	unset(MODELBUILDER_CURRENT_PYTHON_VERSION CACHE)
#
#	# Unset Python variables to change Python version after "configure" operation
#	# @see: https://github.com/Itseez/opencv/blob/master/cmake/OpenCVDetectPython.cmake
#	unset(PYTHONLIBS_FOUND)
#	unset(PYTHON_LIBRARIES)
#	unset(PYTHON_INCLUDE_PATH)
#	unset(PYTHON_INCLUDE_DIRS)
#	unset(PYTHON_DEBUG_LIBRARIES)
#	unset(PYTHONLIBS_VERSION_STRING)
#	unset(PYTHON_DEBUG_LIBRARY CACHE)
#	unset(PYTHON_LIBRARY)
#	unset(PYTHON_LIBRARY_DEBUG)
#	unset(PYTHON_LIBRARY_RELEASE)
#	unset(PYTHON_LIBRARY CACHE)
#	unset(PYTHON_LIBRARY_DEBUG CACHE)
#	unset(PYTHON_LIBRARY_RELEASE CACHE)
#	unset(PYTHON_INCLUDE_DIR CACHE)
#	unset(PYTHON_INCLUDE_DIR2 CACHE)
#endif()

## Make the internal variable private
#mark_as_advanced(MODELBUILDER_CURRENT_PYTHON_VERSION)

## Print a message on Python version to be used
#message(STATUS "Python version to use: ${MODELBUILDER_PYTHON_VERSION}")



# Find Python package according to the chosen version
#find_package(PythonLibs ${MODELBUILDER_PYTHON_VERSION} EXACT)
find_package(PythonInterp)
find_package(PythonLibs)

find_package(Sphinx)

# ------------------------- #
#
# NURBS API
#

add_subdirectory(${CMAKE_SOURCE_DIR}/NURBS)
include_directories(${CMAKE_SOURCE_DIR}/NURBS/src)
# Change the NURBS install location to the DeLaMo's install location
unset(NURBS_INSTALL_PREFIX)
unset(NURBS_INSTALL_PREFIX CACHE)
unset(NURBS_INSTALL_DIR)
unset(NURBS_INSTALL_DIR CACHE)
unset(NURBS_EXAMPLES_INSTALL_DIR)
unset(NURBS_EXAMPLES_INSTALL_DIR CACHE)
#unset(NURBS_PYTHON_PACKAGE_NAME)
#unset(NURBS_PYTHON_PACKAGE_NAME CACHE)
unset(NURBS_PYTHON_PACKAGE_PATH)
unset(NURBS_PYTHON_PACKAGE_PATH CACHE)

set(NURBS_INSTALL_PREFIX ${APP_INSTALL_PREFIX} CACHE STRING "NURBS Library install location")
set(NURBS_EXAMPLES_INSTALL_DIR ${APP_INSTALL_DIR}/examples) # CACHE STRING "Installation location for NURBS examples")
set(NURBS_INSTALL_DIR ${APP_INSTALL_DIR} CACHE STRING "Installation location for NURBS examples")
#set(NURBS_PYTHON_PACKAGE_NAME "delamo" CACHE STRING "Python package name for the SWIG-wrapped module")
set(NURBS_PYTHON_PACKAGE_PATH "../de-la-mo/${NURBS_PYTHON_PACKAGE_NAME}" CACHE STRING "Path for installation of the SWIG-wrapped NURBS module")






# ------------------------- #
#
# MODELBUILDER DEBUGGING OPTIONS
#

# Add an option to CMake to choose a debug level
set(MODELBUILDER_DEBUG_LEVEL "ERROR" CACHE STRING "MODELBUILDER Debug Level")
set_property(CACHE MODELBUILDER_DEBUG_LEVEL PROPERTY STRINGS "NONE" "INFO" "WARN" "ERROR" "DEBUG")
if(MODELBUILDER_DEBUG_LEVEL STREQUAL "INFO")
	add_definitions(-DMODELBUILDER_DEBUG_LEVEL=1)
elseif(MODELBUILDER_DEBUG_LEVEL STREQUAL "WARN")
	add_definitions(-DMODELBUILDER_DEBUG_LEVEL=2)
elseif(MODELBUILDER_DEBUG_LEVEL STREQUAL "ERROR")
	add_definitions(-DMODELBUILDER_DEBUG_LEVEL=3)
elseif(MODELBUILDER_DEBUG_LEVEL STREQUAL "DEBUG")
	add_definitions(-DMODELBUILDER_DEBUG_LEVEL=4)
else(MODELBUILDER_DEBUG_LEVEL STREQUAL "INFO")
	add_definitions(-DMODELBUILDER_DEBUG_LEVEL=0)
endif(MODELBUILDER_DEBUG_LEVEL STREQUAL "INFO")




# ------------------------- #
#
# Solid Modeling Kernel (SMK) Selection and Configuration
#

# Create a combobox to choose from various SMK libraries
set(MODELBUILDER_NAME "ACIS" CACHE STRING "Solid Modeling Kernel Library")
set_property(CACHE MODELBUILDER_NAME PROPERTY STRINGS "ACIS" "Parasolid")


# Unset some variables for solid modeling kernel change
unset(MODELBUILDER_LIBRARIES)
unset(MODELBUILDER_SOURCE_FILES)
unset(MODELBUILDER_SWIG_INTERFACE)
unset(MODELBUILDER_LIBRARIES_REDIST_RELEASE)
unset(MODELBUILDER_LIBRARIES_REDIST_DEBUG)

# Print a message on which SMK is in use
message(STATUS "Using solid modeling kernel: ${MODELBUILDER_NAME}")
SET(SMK_CONFIGURED FALSE)

# ACIS Settings
if(MODELBUILDER_NAME STREQUAL "ACIS")
	# findACIS module accepts an parameter for processing additional search paths
	set(ACIS_ROOT "/opt" CACHE PATH "Path to ACIS installation.  This path contains the root ACIS folder (e.g., 'acisR26').")

	# Find ACIS package
	find_package(ACIS REQUIRED)

	# Set some variables for compatibility (will fix these later)
	if(ACIS_FOUND)
		include_directories(${ACIS_INCLUDE_DIRS})
		set(MODELBUILDER_LIBRARIES ${ACIS_TARGET_LINK_LIBRARIES})
		# We don't need ACIS_ROOT anymore
		unset(ACIS_ROOT)
		unset(ACIS_ROOT CACHE)
    else()
        message(WARNING "ACIS not found")
	endif()

	# Add ACIS API to ModelBuilder source files
	set(MODELBUILDER_SOURCE_FILES
		src/ACIS.h
		src/ACISModelBuilder.h
		src/ACISModelBuilder.cpp
	)

	# Add SWIG interface
	set(MODELBUILDER_SWIG_INTERFACE
        src/swig/CADmodeler.i
    )

	# Pass required preprocessor variable to include ACIS-related files
	add_definitions(-DACISOBJ)


    # Get name of ACIS library to be redistributed on INSTALL
    set(MODELBUILDER_LIBRARY_REDIST_RELEASE ${ACIS_LIBRARY_RELEASE})
    set(MODELBUILDER_LIBRARY_REDIST_DEBUG ${ACIS_LIBRARY_DEBUG})

    # If on Windows, replace the redistributed *.LIB with the corresponding *.DLL
    if(WIN32)
        get_filename_component(DIR_LIB ${ACIS_LIBRARY_RELEASE} DIRECTORY)
        get_filename_component(NAME_LIB ${ACIS_LIBRARY_RELEASE} NAME_WE)
        SET(MODELBUILDER_LIBRARY_REDIST_RELEASE "${DIR_LIB}/../bin/${NAME_LIB}.dll")

        get_filename_component(DIR_LIB ${ACIS_LIBRARY_DEBUG} DIRECTORY)
        get_filename_component(NAME_LIB ${ACIS_LIBRARY_DEBUG} NAME_WE)
        set(MODELBUILDER_LIBRARY_REDIST_DEBUG "${DIR_LIB}/../bin/${NAME_LIB}.dll")
    endif(WIN32)

    # Set flag for successful configuration
    set(SMK_CONFIGURED TRUE)
endif()

# Parasolid Settings
if(MODELBUILDER_NAME STREQUAL "Parasolid")
	# Add other SMK settings here
    MESSAGE(WARNING "Parasolid support is not implemented.  Select an alternate kernel.")
endif()

IF(NOT SMK_CONFIGURED)
    MESSAGE(FATAL_ERROR "Must configure a Solid Modeling Kernel.  Configuration Aborted.")
ENDIF()





# ------------------------- #
#
# ModelBuilder Library
#

# Set source files for this project
set(MODELBUILDER_SOURCE_FILES
	${MODELBUILDER_SOURCE_FILES}
	src/APIConfig.h
	src/Layer.h
	src/Layer.cpp
	src/MBBody.h
	src/MBBody.cpp
	src/LayerBody.h
	src/LayerBody.cpp
	src/LayerSurface.h
	src/LayerSurface.cpp
	src/LayerMold.h
	src/LayerMold.cpp
	src/ModelBuilder.h
	src/ModelBuilder.cpp
	${PROJECT_BINARY_DIR}/modelbuilder_export.h
	# Include ModelBuilder utility functions
	src/mb_utilities.h
	src/mb_utilities.cpp
	# Point-Normal Finding
	src/PNFind_ABS.h
	src/PNFind_ABS.cpp
	src/PNFind_EdgeMove.h
	src/PNFind_EdgeMove.cpp
	src/PNFind_UVseek.h
	src/PNFind_UVseek.cpp
	src/PNFind_BBox.h
	src/PNFind_BBox.cpp
)

# Compile and link
set(CMAKE_MACOSX_RPATH 1)
add_library(ModelBuilder SHARED ${MODELBUILDER_SOURCE_FILES})
generate_export_header(ModelBuilder)
target_link_libraries(ModelBuilder ${MODELBUILDER_LINK_LIBS} ${MODELBUILDER_LIBRARIES})
target_include_directories(ModelBuilder PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

# Set required C++ standard for the ModelBuilder target
set_property(TARGET ModelBuilder PROPERTY CXX_STANDARD 11)
set_property(TARGET ModelBuilder PROPERTY CXX_STANDARD_REQUIRED ON)





# ------------------------- #
#
# Setup SWIG Interface for ModelBuilder
#

# Find SWIG Package
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# SWIG has better support for C++ templates in version 3.0.11 and we can get compile errors if an older version is used
if(SWIG_VERSION VERSION_LESS "3.0.11")
	message(FATAL_ERROR "You are using an old version of SWIG ( v${SWIG_VERSION} ). Please upgrade to v3.0.11 or higher.")
endif()

include_directories(${PYTHON_INCLUDE_DIRS})

# Indicate that wrapper is for C++ code. Required b/c use of classes.
set_source_files_properties(${MODELBUILDER_SWIG_INTERFACE} PROPERTIES CPLUSPLUS ON)

# Set output directory for wrapper (*_wrap.cxx)
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}")

# Create SWIG wrapper and the Python module
if(CMAKE_VERSION VERSION_LESS 3.8)
	# swig_add_module is deprecated in CMake version 3.8
	swig_add_module(CADmodeler python ${MODELBUILDER_SWIG_INTERFACE})
else(CMAKE_VERSION VERSION_LESS 3.8)
	swig_add_library(CADmodeler LANGUAGE python SOURCES ${MODELBUILDER_SWIG_INTERFACE})
endif(CMAKE_VERSION VERSION_LESS 3.8)

# Link SWIG module with ModelBuilder and the Python libraries
swig_link_libraries(CADmodeler ModelBuilder ${PYTHON_LIBRARIES})
target_include_directories(${SWIG_MODULE_CADmodeler_REAL_NAME} PUBLIC ${PROJECT_BINARY_DIR})

# Python debug builds require "_d" suffix to load the module

IF(WIN32)
  # Add suffix to debug builds
  set_target_properties(ModelBuilder PROPERTIES DEBUG_POSTFIX "_d")

  set_target_properties(${SWIG_MODULE_CADmodeler_REAL_NAME} PROPERTIES DEBUG_POSTFIX  "_d")

ENDIF()

# Set required C++ standard for the _CADsupport target
set_property(TARGET ${SWIG_MODULE_CADmodeler_REAL_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${SWIG_MODULE_CADmodeler_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

# Set Python example files
UNSET(MODELBUILDER_PYTHON_EXAMPLE_FILES)
SET(DATA_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/examples")
FILE(GLOB MODELBUILDER_PYTHON_EXAMPLE_FILES "${DATA_FILE_DIR}/*.py")

# Install Python example files to the specified directory
foreach(f ${MODELBUILDER_PYTHON_EXAMPLE_FILES})
	install(FILES ${f} DESTINATION ${APP_INSTALL_DIR}/examples)
endforeach()

# Install Python module (.pyd file) and required libraries
install(
	TARGETS ${SWIG_MODULE_CADmodeler_REAL_NAME} ModelBuilder
	DESTINATION ${APP_INSTALL_DIR}/delamo
)

# Install Python module (.py file)
install(
	FILES "${PROJECT_BINARY_DIR}/CADmodeler.py"
	DESTINATION ${APP_INSTALL_DIR}/delamo
)

# Set additional SWIG headers
set(MODELBUILDER_SWIG_INTERFACE_EXTRAS
    src/swig/CADmodeler_renames.i
	src/swig/CADmodeler_typemaps.i
	src/swig/CADmodeler_extends.i
)

# Add a Visual Studio filter for the NURBS SWIG interface target
source_group("SWIG Headers" FILES ${MODELBUILDER_SWIG_INTERFACE_EXTRAS})
set_source_files_properties(${MODELBUILDER_SWIG_INTERFACE_EXTRAS} PROPERTIES CPLUSPLUS ON)
add_custom_target(${SWIG_MODULE_CADmodeler_REAL_NAME}_Extras SOURCES ${MODELBUILDER_SWIG_INTERFACE_EXTRAS})



# ----------------------------------------
#
# PTH FILE GENERATION
#

# Set extra Python-related variables that are needed to install the build module
if(WIN32)
    set(PYTHON_SITE_PACKAGES "${PYTHON_INCLUDE_DIR}/../Lib/site-packages" CACHE PATH "Directory containing site-packages")
else(WIN32)
    if(APPLE)
        set(PYTHON_SITE_PACKAGES "~/install/python27/site-packages" CACHE PATH "Directory containing site-packages")
    else(APPLE)
      # If we are here, we're on Linux
      # ask Python what its site-packages directory is
      execute_process ( COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE _AUTOGEN_PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(PYTHON_SITE_PACKAGES ${_AUTOGEN_PYTHON_SITE_PACKAGES} CACHE PATH "Directory containing site-packages")
    endif(APPLE)
endif(WIN32)

## Install python "delamo" package into destination
#install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/delamo DESTINATION ${APP_INSTALL_PREFIX}/de-la-mo )
FILE(GLOB DELAMO_DIRECTORY_FILES "${CMAKE_CURRENT_SOURCE_DIR}/delamo/*.py")
foreach(item ${DELAMO_DIRECTORY_FILES})
  # Filter out CADmodeler.py and CADsupport.py from source directory in case there are symbolic links there
  if(NOT ( ( ${item} MATCHES ".*CADsupport.py$") OR (${item} MATCHES ".*CADmodeler.py$") ))
    LIST(APPEND DELAMO_PACKAGE_FILES ${item})
  endif()
endforeach()

foreach(f ${DELAMO_PACKAGE_FILES})
  install(FILES ${f} DESTINATION ${APP_INSTALL_DIR}/delamo)
endforeach()


# Install python scripts into destination
FILE(GLOB SCRIPTS_DIRECTORY_FILES "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*")
foreach(item ${SCRIPTS_DIRECTORY_FILES})
  # Filter out files with extensions, hidden files, lock files, and emacs backups
  if(NOT ( ( ${item} MATCHES ".*[.].*") OR (${item} MATCHES ".*~$") OR (${item} MATCHES ".*LOCK.*") OR (${item} MATCHES ".*LCK.*") )) # hidden files captured automatically by the files with extensions
    LIST(APPEND SCRIPTS_INSTALLABLE_FILES ${item})
  endif()
endforeach()

foreach(f ${SCRIPTS_INSTALLABLE_FILES})
  install(PROGRAMS ${f} DESTINATION ${APP_INSTALL_DIR}/scripts)
  if ((NOT WIN32) AND INSTALL_SCRIPTS_INTO_INSTALL_PREFIX_BIN_DIRECTORY)
    install(PROGRAMS ${f} DESTINATION ${APP_INSTALL_PREFIX}/bin)
  endif()
endforeach()



# Create a variable to enable/disable installing .pth file into site-packages
set(INSTALL_INTO_PYTHON_SITE_PACKAGES ON CACHE BOOL "Install into the Python site-packages so that scripts run without setting PYTHONPATH")

if(NOT WIN32)
  set(INSTALL_SCRIPTS_INTO_INSTALL_PREFIX_BIN_DIRECTORY ON CACHE BOOL "Install scripts into the bin/ directory under your selected prefix")
endif()


# Create a variable to enable/disable building documentation
set(BUILD_DOCUMENTATION OFF CACHE BOOL "Build documentation (requires Sphinx and installed copy of De-la-mo)")


set(FAKE delamo.pth)
add_custom_command(OUTPUT delamo.pth COMMAND ${CMAKE_COMMAND} -E echo ${APP_INSTALL_DIR} > "${CMAKE_CURRENT_BINARY_DIR}/delamo.pth" )

# NOTE: Must do 'make generate_pth' prior to 'make install'
add_custom_target(generate_pth DEPENDS delamo.pth)
# Cannot make INSTALL dependent on 'generate_pth', so we make the SWIG module dependent.
add_dependencies(${SWIG_MODULE_CADmodeler_REAL_NAME} generate_pth)

if (INSTALL_INTO_PYTHON_SITE_PACKAGES)
  # Install .pth file into site-packages directory so that Python can find our scripts
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/delamo.pth" DESTINATION "${PYTHON_SITE_PACKAGES}")
endif()


# Don't put the generated delamo.pth file into repository, so clean it
set_property(GLOBAL PROPERTY ADDITIONAL_MAKE_CLEAN_FILES delamo.pth)





# ------------------------- #
#
# DATA INPUT FILES
#

# Create a variable to store input files
UNSET(MODELBUILDER_INPUT_FILES)
SET(DATA_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/examples/data")
FILE(GLOB MODELBUILDER_INPUT_FILES "${DATA_FILE_DIR}/*.txt" "${DATA_FILE_DIR}/*.csv")

# Copy input files to project binary dir to run project from Visual Studio
foreach(f ${MODELBUILDER_INPUT_FILES})
    GET_FILENAME_COMPONENT(NAME_TEMP ${f} NAME)
	configure_file(${f} ${CMAKE_CURRENT_BINARY_DIR}/${NAME_TEMP} COPYONLY)
endforeach()

# Install input files to the specified directory
foreach(f ${MODELBUILDER_INPUT_FILES})
	install(FILES ${f} DESTINATION ${APP_INSTALL_DIR}/examples/data)
endforeach()





# ------------------------- #
#
# CAD LIBRARY DEPENDENCIES
#


if (WIN32)
  install(
    FILES ${MODELBUILDER_LIBRARY_REDIST_DEBUG}
    DESTINATION ${APP_INSTALL_DIR}/delamo
    CONFIGURATIONS Debug)
else()
  install(
    FILES ${MODELBUILDER_LIBRARY_REDIST_RELEASE}
    DESTINATION ${APP_INSTALL_DIR}/delamo
    CONFIGURATIONS Debug)
endif()

install(
	FILES ${MODELBUILDER_LIBRARY_REDIST_RELEASE}
	DESTINATION ${APP_INSTALL_DIR}/delamo
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
)

message(STATUS "MODELBUILDER_LIBRARY_REDIST_RELEASE: ${MODELBUILDER_LIBRARY_REDIST_RELEASE}")




# ------------------------- #
#
# Executables for API testing.
#

# Create a variable to enable/disable testcase apps installation
set(MODELBUILDER_INSTALL_APPS OFF CACHE BOOL "Adds testcase apps to the INSTALL target")

# Read app source files inside the test case source directory
file(GLOB TESTCASE_APPS "${CMAKE_CURRENT_LIST_DIR}/src/testcases/app_*.cpp")

# Common includes for all apps
set(TESTCASE_SOURCE_COMMON
	src/testcases/testcase_includes.h
	src/testcases/testcase_common.cpp
)

# Set up the link libraries
if(NOT WIN32)
    set(MODELBUILDER_LINK_LIBS ${LINK_LIBS} "stdc++")
	set(TESTCASE_LIBS ${LINK_LIBS} "stdc++")
endif()
set(TESTCASE_LIBS ${TESTCASE_LIBS} ModelBuilder)

# Print an informative message before starting TESTCASE_APPS loop
message(STATUS "Loaded test case applications:")

# Loop through each app to compile-link-install
foreach(app_ext_path ${TESTCASE_APPS})
	# Get filename without the directory and the extension
	get_filename_component(app ${app_ext_path} NAME_WE)

	# Print test case name
	message(STATUS ${app})

	# Generate executable file
	add_executable(${app} ${TESTCASE_SOURCE_COMMON} src/testcases/${app}.cpp)

	# Link executable with the libraries
	target_link_libraries(${app} ${TESTCASE_LIBS})

	# Add suffix to debug builds
	set_target_properties(${app} PROPERTIES DEBUG_POSTFIX "_d")

	# Set required C++ standard for this target
	set_property(TARGET ${app} PROPERTY CXX_STANDARD 11)
	set_property(TARGET ${app} PROPERTY CXX_STANDARD_REQUIRED ON)

	if(MODELBUILDER_INSTALL_APPS)
		# Install the app
		install(TARGETS ${app} DESTINATION ${APP_INSTALL_DIR}/bin)
	endif()
endforeach()

# Install required libs for the apps
if(MODELBUILDER_INSTALL_APPS)
	# Install ModelBuilder
	install(
		TARGETS ModelBuilder
		DESTINATION ${APP_INSTALL_DIR}/bin
	)

        # Install ACIS shared libraries
	if (WIN32)
 	  install(
		FILES ${MODELBUILDER_LIBRARY_REDIST_DEBUG}
		DESTINATION ${APP_INSTALL_DIR}/bin
		CONFIGURATIONS Debug)
	else()
 	  install(
		FILES ${MODELBUILDER_LIBRARY_REDIST_RELEASE}
		DESTINATION ${APP_INSTALL_DIR}/bin
		CONFIGURATIONS Debug)
	endif()

	install(
		FILES ${MODELBUILDER_LIBRARY_REDIST_RELEASE}
		DESTINATION ${APP_INSTALL_DIR}/bin
		CONFIGURATIONS Release RelWithDebInfo MinSizeRel
	)
endif()

# Documentation
if( BUILD_DOCUMENTATION AND Sphinx_FOUND )
  configure_file("${CMAKE_SOURCE_DIR}/doc/conf.py.in" "${CMAKE_BINARY_DIR}/conf.py")

  add_custom_target(copy_doc_source ALL
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/doc/index.rst" "${CMAKE_BINARY_DIR}/index.rst"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/doc/delamo.rst" "${CMAKE_BINARY_DIR}/delamo.rst"
    #COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/doc/modules.rst" "${CMAKE_BINARY_DIR}/modules.rst"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/doc/delamo_static" "${CMAKE_BINARY_DIR}/delamo_static"
    #COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/doc/delamo_templates" "${CMAKE_BINARY_DIR}/delamo_templates"
    COMMENT "Copying documentation source"
    )
  Sphinx_add_targets(build ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR} ${CMAKE_BINARY_DIR} copy_doc_source )
  if(${SPHINX_HTML_OUTPUT})
    install(DIRECTORY ${CMAKE_BINARY_DIR}/html
      DESTINATION ${APP_INSTALL_DIR}/doc
      COMPONENT Doc
      PATTERN "${CMAKE_BINARY_DIR}/html/*"
      )
  endif()
  
  if(${SPHINX_LATEX_OUTPUT})
    find_package(LATEX)
    
    if(NOT PDFLATEX_COMPILER)
      message("pdflatex must be found or set in configuration options for LaTeX PDF output")
    else()
      # Run latex twice
      add_custom_command(build_latex
	POST_BUILD
	COMMAND ${PDFLATEX_COMPILER} ${CMAKE_BINARY_DIR}/latex/index.tex -output-directory ${CMAKE_BINARY_DIR}/latex
	COMMAND ${MAKEINDEX_COMPILER} index.idx
	COMMAND ${PDFLATEX_COMPILER} ${CMAKE_BINARY_DIR}/latex/index.tex -output-directory ${CMAKE_BINARY_DIR}/latex
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/latex
	COMMENT "Building PDF Documentation"
	)
      install(FILES ${CMAKE_BINARY_DIR}/index.pdf
	DESTINATION ${APP_INSTALL_DIR}/doc
	COMPONENT Doc
	)
    endif()
  endif()
else()
  if( BUILD_DOCUMENTATION AND NOT Sphinx_FOUND )
    
    MESSAGE(WARNING "Sphinx not found -- Not building documentation!")
  endif()
endif()

### DON'T TOUCH ABOVE THIS LINE - END ###
