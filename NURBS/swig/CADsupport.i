// Copyright (c) 2018 by Adarsh Krishnamurthy et. al. and Iowa State University. 
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for non-profit use, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL IOWA STATE UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF IOWA STATE UNIVERSITY
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// IOWA STATE UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND IOWA STATE UNIVERSITY HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//
// Initial version April 20 2018 - Adarsh Krishnamurthy et. al.
//

/* 
 * File: CADsupport.i
 * Module: CADsupport
 */
%module(package="delamo") CADsupport

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
// C++ includes 
#include "../src/PointVector.hxx"
#include "../src/ContainerList.hxx"
#include "../src/NURBS.hxx"

// Set the namespace
using namespace delamo;

struct DoubleStruct
{
	double* ptr_array;
	int ptr_size;
};

struct Point3Struct
{
	delamo::TPoint3<double>* ptr_array;
	int ptr_size_u;
	int ptr_size_v;
};

struct Vector3Struct
{
	delamo::TVector3<double>* ptr_array;
	int ptr_size_u;
	int ptr_size_v;
};


%}

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

// Swig includes
%include "std_string.i"
%include "std_vector.i"
#ifdef _WIN32
%include "windows.i"
#endif

// Import "rename" directives
%import "CADsupport_renames.i"

// Import "typemap" directives
%import "CADsupport_typemaps.i"

/*
* Include files which would be parsed by Swig interpreter
*/

// Include libraries for Swing parsing
%include "../src/PointVector.hxx"
 //%include "../src/ContainerList.hxx"
namespace delamo
{
	template <typename T>
	class List
	{
	public:
		using size_type = unsigned int; /**< Default size type for the container class */
		using value_type = T; /**< Default value type for the container class */
		using iterator = T*; /**< Iterator type for the container class */
		using const_iterator = const T*; /**< Const Iterator type for the container class */
		List();
		explicit List(size_type s, T value);
		explicit List(T* ptr_elem, size_type ptr_elem_size);
		List(const List<T>& rhs);
		List(List<T>&& rhs);
		~List();
		List<T>& operator=(const List<T>& rhs);
		List<T>& operator=(List<T>&& rhs);
		T& operator[](size_type idx);
		const T& operator[](size_type idx) const;
		T* data();
		const T* data() const;
		size_type size();
		void clear();
		bool empty();
		void push_front(const T& elem);
		void push_front(List<T>& elem_list);
		void push_back(const T& elem);
		void push_back(List<T>& elem_list);
		void add(const T& elem);
		void append(const T& elem);
		T pop_front();
		T pop_back();
		T remove();
		T& at(size_type idx);
		bool reserve(size_type newalloc);
		size_type capacity() const;
		void resize(size_type newsize);
		iterator begin();
		const_iterator begin() const;
		iterator end();
		const_iterator end() const;
		iterator first();
		iterator last();
		void reverse();
		void reset_alloc();
	};
}

%include "../src/NURBS.hxx"

// Set the namespace
using namespace delamo;

// Import "extend" directives
%import "CADsupport_extends.i"

// Instantiate templates
%template(TPoint3d) delamo::TPoint3<double>; // was Point3
%template(TPoint3dList) delamo::List< TPoint3<double> >;  // Was Point3List
%template(NURBSd) delamo::NURBS<double>;  // Was "Surface"
