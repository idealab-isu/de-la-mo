/*
* File: CADmodeler.i
* Module: CADmodeler
*/
%module(package="delamo") CADmodeler

/*
 * Suppress some warning messages that Swig produces on ignoring operators, etc.
 *
 * Warnings are defined in swigwarn.h file.  
 *	Warning 509: WARN_LANG_OVERLOAD_SHADOW
 *	Warning 454: WARN_TYPEMAP_SWIGTYPELEAK
 *	Warning 454: WARN_CPP11_ALIAS_DECLARATION => Related to Swig's C++11 standard support 
 */
#pragma SWIG nowarn=509,454,341

/*
 * C/C++ code or definitions required for the wrapper file (*_wrap.cxx) to compile it properly
 *
 * This section will be included in the wrapper file directly.
 * Use for:
 *	- Defining extra variables, directives, macros to use in the wrapper file
 *	- Including library header files
 *	- Adding extra functionality to use for the target language (i.e. Python)
 *
 * This section can be thought as a C/C++ file (i.e. .c, .cpp) with Swig headers included.
 * Therefore, you can use SWIG-defined variables, preprocessor directives and other functionality.
 */
%{
// On Windows, we require declspec functions, but on Linux, we don't
#ifdef _WIN32
#include "modelbuilder_export.h"
#else
#define MODELBUILDER_EXPORT
#endif

// Define the solid modeling kernel to use (3D ACIS Modeler)
#ifndef ACISOBJ
#define ACISOBJ
#endif

// Required includes
#include "PointVector.hxx"
#include "ContainerList.hxx"
#include "NURBS.hxx"

using namespace delamo;

// Common includes
#include "src/MBBody.h"
#include "src/Layer.h"
#include "src/LayerBody.h"
#include "src/LayerSurface.h"
#include "src/LayerMold.h"
#include "src/ModelBuilder.h"
#include "src/ACISModelBuilder.h"
%}

/*
 * Start SWIG processing and definitions
 */

//%ignore delamo::List<std::string>(std::string,unsigned int);
//%ignore delamo::List< std::string >( std::string ,delamo::List< std::string >::size_type);
//%ignore StringList(std::string ,delamo::List< std::string >::size_type);
//%ignore StringList(std::string,unsigned int);

// SWIG exception handling
%include "exception.i"
%exception{
	try {
		$action
	}
	catch (const std::exception& e) {
		SWIG_exception(SWIG_RuntimeError, e.what());
	}
}

// Import NURBS library
%import "../../NURBS/swig/CADsupport.i"

// Swig includes
%include "std_string.i"
%include "std_vector.i"
#ifdef _WIN32
%include "windows.i"
#endif

// Import "rename" directives
%import "CADmodeler_renames.i"

// Import "typemap" directives
%import "CADmodeler_typemaps.i"

/*
 * Include files which will be parsed by Swig interpreter
 */
#ifdef _WIN32
%include "modelbuilder_export.h"
#else
#define MODELBUILDER_EXPORT
#endif


%include "../APIConfig.h"
%include "../MBBody.h"
%include "../Layer.h"
%include "../LayerBody.h"
%include "../LayerSurface.h"
%include "../LayerMold.h"
%include "../ModelBuilder.h"
%include "../ACISModelBuilder.h"
 //%nclude "ContainerList.hxx"



// Import "extend" directives
%import "CADmodeler_extends.i"

// Example of STL vector of vectors with shared pointers
%include "std_container.i"

// For some reason this code isn't making it in from pycontainer.swg in swig 3.0.12 (?) so it is included here as a workaround
%{
#include <iostream>

#if PY_VERSION_HEX >= 0x03020000
# define SWIGPY_SLICE_ARG(obj) ((PyObject*) (obj))
#else
# define SWIGPY_SLICE_ARG(obj) ((PySliceObject*) (obj))
#endif
%}


%include "std_vector.i"
%include "std_shared_ptr.i"

%shared_ptr(std::vector<>);
%shared_ptr(std::vector<delamo::TPoint3<double>>);
%shared_ptr(std::vector<std::shared_ptr<std::vector<delamo::TPoint3<double>>>>);

%ignore std::vector::pop(); // Work around apparent Swig 3.0.12 bug

%template(VecTPoint3d) std::vector<delamo::TPoint3<double>>;
%template(VecVecTPoint3d) std::vector<std::shared_ptr<std::vector<delamo::TPoint3<double>>>>;

  

// Instantiate templates
%template(StringList) delamo::List<std::string>;
%template(FAL) delamo::List<FaceAdjacency*>;
%template(LayerList) delamo::List<Layer*>;
%template(MBBodyList) delamo::List<MBBody*>;
%template(LayerMoldList) delamo::List<LayerMold*>;
 
