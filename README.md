# DE-LA-MO

The De-la-mo website can be found at http://thermal.cnde.iastate.edu/de-la-mo.xhtml

PLEASE NOTE: THIS IS THE OBSOLETE DE-LA-MO V1 REPOSITORY. You probably want de-la-mo v2, which no longer requires the Acis solid modeler, nor swig, nor a compiler.

On-line documentation and installation instructions are located at
http://thermal.cnde.iastate.edu/de-la-mo/index.html

De-La-Mo is Copyright (C) 2016-2018 Iowa State University
Research Foundation, Inc. It is published under the
Apache 2.0 license. See the LICENSE.txt file for details


## Repository Organization
* `doc/': Contains documentation source code
* `scripts/': Contains Python helper scripts for building De-la-mo models
* `delamo/`: Contains the python package _delamo_
* `examples/`: Contains example python scripts which demonstrate how to use _delamo_
* `examples/data/`: Contains data files used in the example scripts
* `src/`: Contains C++ source code for the ModelBuilder tool. This tool is accessed from python using a SWIG-generated interface
* `src/swig/`: Contains SWIG source code for the ModelBuilder tool
* `src/testcases/`: Contains C++ source code for the ModelBuilder test cases
* `CMake/`: Contains CMake helper scripts

## Quickstart Build Instructions
1. Acquire requirements listed at http://thermal.cnde.iastate.edu/de-la-mo.xhtml
2. Run CMake on the CMakeLists.txt in the repository root directory. It
is generally the best practice to build into a new directory. 
3. make && make install

## Installation Structure

- `scripts/` Contains Python helper scripts for building De-la-mo models

- `delamo/`
    Contains the python package _delamo_ which is ready for import by the python scripts.

    This directory also contains the SWIG-wrapped ModelBuilder library and extra
    shared libraries required for functionality (e.g., SpaACIS.dll on Win32).

- `examples/`
    Contains copies of the python scripts from `<repository root>/examples/` directory.
	
- `examples/data/`
	Files from `<repository root>/examples/data/` are copied here.


## How to Run De-la-mo

Each of the Python scripts in the `examples/' directory can be run directly
from a Python interpreter. This will create a subdirectory with the
`_output' suffix containing the generated CAD model (`.sat' file) and
generated Python script for ABAQUS (`.py' file). Run the generated `.py' file
inside ABAQUS/CAE to perform the desired analysis. 

The 'delamo_process' utility (in the `scripts/' directory) can be used as a
helper to assist in running De-la-mo scripts. It is especially helpful
for multi-step processes such as automated damage insertion.
