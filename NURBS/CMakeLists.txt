cmake_minimum_required(VERSION 2.8)
project(NURBS_API CXX)



# ------------------------- #

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



# ------------------------- #

# Set installation location rules
IF(APPLE)    # Apple
  SET(NURBS_INSTALL_PREFIX "~/install" CACHE PATH "Prefix prepended to NURBS install directories.  Must be writable by your user!")
ELSEIF(UNIX) # Linux (not Apple due to use of if/else-if construct)
  SET(NURBS_INSTALL_PREFIX "/usr/local" CACHE PATH "Prefix prepended to NURBS install directories.  Must be writable by your user!")  
ENDIF()

IF (NOT DEFINED NURBS_INSTALL_DIR)
  IF(WIN32)
    SET(NURBS_INSTALL_DIR ${PROJECT_BINARY_DIR}/install) # CACHE PATH "Installation directory.  Must be writable by your user!")
  ELSE()
    SET(NURBS_INSTALL_DIR ${NURBS_INSTALL_PREFIX}/NURBS) # CACHE PATH "Installation directory.  Must be writable by your user!") 
  ENDIF()
ENDIF()

set(CMAKE_INSTALL_PREFIX "${NURBS_INSTALL_PREFIX}" CACHE PATH "Prefix prepended to install directories" FORCE)

set(NURBS_EXAMPLES_INSTALL_DIR "${NURBS_INSTALL_DIR}/examples" CACHE PATH "Installation location for NURBS examples")


# ------------------------- #

#
# NURBS Template libraries
#

set(NURBS_TEMPLATE_SOURCE
    src/PointVector.hxx
    src/ContainerList.hxx
	src/NURBS.hxx
)

add_custom_target(NURBS_API SOURCES ${NURBS_TEMPLATE_SOURCE})
include_directories(NURBS_API PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})



# ------------------------- #

#
# Setup SWIG Interface for NURBS
#

set(NURBS_PYTHON_PACKAGE_NAME "delamo" CACHE STRING "Python package name to install the SWIG-wrapped module")

set(NURBS_PYTHON_PACKAGE_PATH "${NURBS_PYTHON_PACKAGE_NAME}" CACHE STRING "Directory path for the SWIG-wrapped module")


# Fix a common problem related to cmath and Python
add_definitions(-D_hypot=hypot)

# Set SWIG interface face
set(NURBS_SWIG_INTERFACE 
	swig/ConfigSWIG.h
    swig/CADsupport.i
)
    
# Find SWIG Package
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# Find Python libraries
find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_DIRS})

# Indicate that wrapper is for C++ code.  Required b/c use of classes.
set_source_files_properties(${NURBS_SWIG_INTERFACE} PROPERTIES CPLUSPLUS ON)
     
# Set output directory for wrapper (*_wrap.cxx)
set(CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}")

# CMake v2.8.x require "swig" directory and cannot create it automatically
if(${CMAKE_MAJOR_VERSION} VERSION_LESS 3)
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/swig)
endif()

# Create SWIG wrapper
if(CMAKE_VERSION VERSION_LESS 3.8)
	# swig_add_module is deprecated in CMake version 3.8
	swig_add_module(CADsupport python ${NURBS_SWIG_INTERFACE})
else(CMAKE_VERSION VERSION_LESS 3.8)
	swig_add_library(CADsupport LANGUAGE python TYPE MODULE SOURCES ${NURBS_SWIG_INTERFACE})
endif(CMAKE_VERSION VERSION_LESS 3.8)

# Link SWIG module with the Python libraries
swig_link_libraries(CADsupport ${PYTHON_LIBRARIES})
target_include_directories(${SWIG_MODULE_CADsupport_REAL_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/swig ${PROJECT_BINARY_DIR})

if(WIN32)
  # Python debug builds require "_d" suffix to load the module
  set_target_properties(${SWIG_MODULE_CADsupport_REAL_NAME} PROPERTIES DEBUG_POSTFIX  "_d")
endif()

# Set required C++ standard for the _CADsupport target
set_property(TARGET ${SWIG_MODULE_CADsupport_REAL_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${SWIG_MODULE_CADsupport_REAL_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)   
	
# Install Python module (.pyd file) and the required libraries
install(
	TARGETS ${SWIG_MODULE_CADsupport_REAL_NAME}
	DESTINATION ${NURBS_INSTALL_DIR}/${NURBS_PYTHON_PACKAGE_PATH}
)

# Install Python module (.py file)
install(
	FILES "${PROJECT_BINARY_DIR}/CADsupport.py"
	DESTINATION ${NURBS_INSTALL_DIR}/${NURBS_PYTHON_PACKAGE_PATH}
)

# Install required files for creating a Python package
set(NURBS_PYTHON_PACKAGE_FILES
	python/__init__.py
	python/Support.py
)

# Install Python package files to the install directory
foreach(f ${NURBS_PYTHON_PACKAGE_FILES})
	install(FILES ${f} DESTINATION ${NURBS_INSTALL_DIR}/${NURBS_PYTHON_PACKAGE_PATH})
endforeach()

# Install required files for creating a Python package
set(NURBS_PYTHON_EXAMPLES
	python/nurbs_example.py
)

# Install Python package files to the install directory
foreach(f ${NURBS_PYTHON_EXAMPLES})
	install(FILES ${f} DESTINATION ${NURBS_EXAMPLES_INSTALL_DIR})
endforeach()

# Set additional SWIG headers
set(NURBS_SWIG_INTERFACE_EXTRAS 
    swig/CADsupport_renames.i
	swig/CADsupport_typemaps.i
	swig/CADsupport_extends.i
)
# Add a Visual Studio filter for the NURBS SWIG interface target
source_group("SWIG Headers" FILES ${NURBS_SWIG_INTERFACE_EXTRAS})
set_source_files_properties(${NURBS_SWIG_INTERFACE_EXTRAS} PROPERTIES CPLUSPLUS ON)
add_custom_target(${SWIG_MODULE_CADsupport_REAL_NAME}_Extras SOURCES ${NURBS_SWIG_INTERFACE_EXTRAS})



# ------------------------- #

# Create a variable to store input files
unset(NURBS_INPUT_FILES)
set(NURBS_DATA_FILE_DIR "${CMAKE_CURRENT_LIST_DIR}/data")
file(GLOB NURBS_INPUT_FILES "${NURBS_DATA_FILE_DIR}/*.txt")

# Copy input files to project binary dir to run project from Visual Studio
foreach(f ${NURBS_INPUT_FILES})
    GET_FILENAME_COMPONENT(NAME_TEMP ${f} NAME)
	configure_file(${f} ${CMAKE_CURRENT_BINARY_DIR}/${NAME_TEMP} COPYONLY)
endforeach()

# Install input files to the specified directory
foreach(f ${NURBS_INPUT_FILES})
	install(FILES ${f} DESTINATION ${NURBS_EXAMPLES_INSTALL_DIR}/data)
endforeach()




# ------------------------- #

#
# Example Application for testing the NURBS library
#

# Create a variable to enable/disable app installation
set(NURBS_INSTALL_APPS OFF CACHE BOOL "Adds text executable to the INSTALL target")

# Set source files for this project
set(NURBS_APP_SOURCE
	test/testNURBS.cpp
)

# Start compiling
add_executable(app_nurbslib ${NURBS_APP_SOURCE} ${NURBS_TEMPLATE_SOURCE})

if(NOT WIN32)
	set(LINK_LIBS ${LINK_LIBS} "stdc++")
endif()

# Link executable with the libraries
target_link_libraries(app_nurbslib ${LINK_LIBS})

# Add debug suffix
set_target_properties(app_nurbslib PROPERTIES DEBUG_POSTFIX "_d")

# Set required C++ standard for this target
set_property(TARGET app_nurbslib PROPERTY CXX_STANDARD 11)
set_property(TARGET app_nurbslib PROPERTY CXX_STANDARD_REQUIRED ON)

if(NURBS_INSTALL_APPS)
	# Install the test executable
	install(
		TARGETS app_nurbslib
		DESTINATION ${NURBS_INSTALL_DIR}/bin
	)
endif()
