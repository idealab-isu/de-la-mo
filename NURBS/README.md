# Template Libraries for CAD and Modeling

## Introduction

This package contains the following template classes developed in C++ and their SWIG-wrapped versions for Python.

* __NURBS__ surface evaluation library
* 3D __Point__ and __Vector__ geometry classes
* __List__ container class (FIFO & LIFO)

## Requirements

* A C++11-compliant compiler (i.e. VS2013, VS2015, GCC or MinGW)
* SWIG v3.0.11+ (for Python module generation)
* CMake v2.8+
* Doxygen (for creating documentation)

## What is inside the package?

* ```src/PointVector.hxx```: _delamo::TPoint3_ & _delamo::TVector3_ template classes
* ```src/ContainerList.hxx```: _delamo::List_ container template class
* ```src/NURBS.hxx```: _delamo::NURBS_ template class
* ```swig/*.i```: SWIG interfaces for template classes' _double_ type name
* ```python/*.py```: Python support files and examples
* ```test/*.cpp```: C++ examples

## Compile instructors

* Use __CMake-GUI__ to create the project files
* Check for ```NURBS_*``` options in CMake-GUI for compile and build options

## Running instructions on Linux (or derivatives)

You can simply set the ```PYTHON_PATH``` to include the package directory (i.e. the directory containing \*.py files and \*.pyd files).

If somehow ```PYTHON_PATH``` method doesn't work, please follow the instructions below:

* If you are installing to a custom directory, i.e. ```/home/user/install```, you might need to execute some additional commands to run this library
* First, you need to add the new libraries to the shared library cache by using this command: ```export LD_LIBRARY_PATH="~/install/CAD:$LD_LIBRARY_PATH"```
* Now, you can simply run the example Python script: ```python -i nurbs_example.py```
* Please note that, for computers used by multiple users (i.e. servers), you should install the libraries to a common location, e.g. ```/opt```
