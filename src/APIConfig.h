#ifndef APICONFIG_H
#define APICONFIG_H

// Define a debug level for the output messages
#define MODELBUILDER_DEBUG_NONE		0
#define MODELBUILDER_DEBUG_INFO		1
#define MODELBUILDER_DEBUG_WARN		2
#define MODELBUILDER_DEBUG_ERROR	3
#define MODELBUILDER_DEBUG_DEBUG	4

// Set verbosity of the output messages 
#ifndef MODELBUILDER_DEBUG_LEVEL
#define MODELBUILDER_DEBUG_LEVEL MODELBUILDER_DEBUG_NONE
#endif // !NURBS_DEBUG

// CPP includes
#ifndef SWIG // Hide system headers from SWIG
#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <cstring>
#include <memory>
#include <limits>
#include <cassert>
#include <cctype>
#include <algorithm>
#include <iomanip>
#endif

// NURBS API
#include "NURBS.hxx"
#include "PointVector.hxx"
#include "ContainerList.hxx"

// ACIS includes
#ifdef ACISOBJ
#include "ACIS.h"
#endif

// Convenience macros for declaring SWIG collections, with correspondingly named typedefs
// @see: https://www.softwariness.com/articles/api-design-for-swig/
#ifdef SWIG
#define SWIG_DECLARE_VECTOR(MemberType, TypeName) namespace std { %template(TypeName) vector<MemberType>; } typedef std::vector<MemberType> TypeName;
#define SWIG_DECLARE_MAP(KeyType, ValueType, TypeName) namespace std { %template(TypeName) map<KeyType, ValueType>; } typedef std::map<KeyType, ValueType> TypeName;
#else
#define SWIG_DECLARE_VECTOR(MemberType, TypeName) typedef std::vector<MemberType> TypeName;
#define SWIG_DECLARE_MAP(KeyType, ValueType, TypeName) typedef std::map<KeyType, ValueType> TypeName;
#endif

//SWIG_DECLARE_VECTOR(mystring, StringVector);
//SWIG_DECLARE_MAP(mystring, myint, StringToIntMap);

// Check for C++11
#ifndef CXX11
#if __cplusplus > 199711L
#define CXX11
#else 
#undef CXX11
#endif // __cplusplus > 199711L
#endif // CXX11 

// Layer types
enum LayerType
{ 
	LAMINA, 
	STIFFENER 
};

// Layer direction
enum Direction
{
	NODIR,
	ORIG,
	OFFSET,
	SIDE
};

// Delamination zones
enum DelaminationType // Actually used in two cases: nominal region type generated by adjacent_layers, mapped per desired BCstatus value to requested boundary condition value
{
	NODELAM, // Required value for the side faces
	NOMODEL, 
	COHESIVE, 
	CONTACT, 
	TIE,
	//CONTACT_OR_NOMODEL
};

enum BCStatus  // Possible requests from the Python side
{
	is_contact, // For delaminated composites
	is_cohesive, // For composites
	is_tie, // For solid blocks
	is_none, // for nothing specified
	is_cohesive_layer, // for cohesive layer -- handled on Python side; should never be seen here. 
};

// Face Adjacency List (FAL) for transferring boundary condition information
struct FaceAdjacency
{
	// Ctor
	FaceAdjacency()
	{
		name1 = nullptr;
		name2 = nullptr;
	}

	FaceAdjacency(const FaceAdjacency& rhs)
	{
		std::string rhsname;
		name1 = nullptr;
		name2 = nullptr;

		if (rhs.name1 != nullptr)
		{
			rhsname = std::string(rhs.name1);
			name1 = new char[rhsname.size() + 1];
			std::copy(rhsname.c_str(), rhsname.c_str() + rhsname.size(), name1);
			name1[rhsname.size()] = '\0';
			rhsname.clear();
		}
		else
		{
			name1 = nullptr;
		}

		if (rhs.name2 != nullptr)
		{
			rhsname = std::string(rhs.name2);
			name2 = new char[rhsname.size() + 1];
			std::copy(rhsname.c_str(), rhsname.c_str() + rhsname.size(), name2);
			name2[rhsname.size()] = '\0';
			rhsname.clear();
		}
		else
		{
			name2 = nullptr;
		}

		point1 = rhs.point1;
		vector1 = rhs.vector1;
		point2 = rhs.point2;
		vector2 = rhs.vector2;
		bcType = rhs.bcType;
	}

	// Dtor
	~FaceAdjacency()
	{
		if (name1 != nullptr)
		{
			delete[] name1;
			name1 = nullptr;
		}

		if (name2 != nullptr)
		{
			delete[] name2;
			name2 = nullptr;
		}
	}

	FaceAdjacency& operator=(const FaceAdjacency& rhs)
	{
		// Check for self assignment
		if (this != &rhs)
		{
			std::string rhsname;
			if (name1 != nullptr)
			{
				delete[] name1;
				name1 = nullptr;
			}
			if (rhs.name1 != nullptr)
			{
				rhsname = std::string(rhs.name1);
				name1 = new char[rhsname.size() + 1];
				std::copy(rhsname.c_str(), rhsname.c_str() + rhsname.size(), name1);
				name1[rhsname.size()] = '\0';
				rhsname.clear();
			}
			else
			{
				name1 = nullptr;
			}

			if (name2 != nullptr)
			{
				delete[] name2;
				name2 = nullptr;
			}
			if (rhs.name2 != nullptr)
			{
				rhsname = std::string(rhs.name2);
				name2 = new char[rhsname.size() + 1];
				std::copy(rhsname.c_str(), rhsname.c_str() + rhsname.size(), name2);
				name2[rhsname.size()] = '\0';
				rhsname.clear();
			}
			else
			{
				name2 = nullptr;
			}

			point1 = rhs.point1;
			vector1 = rhs.vector1;
			point2 = rhs.point2;
			vector2 = rhs.vector2;
			bcType = rhs.bcType;
		}
		return *this;
	}

	char* name1; /**< Name of the Layer 1 */
	delamo::TPoint3<double> point1; /**< Reference point of the Layer 1 */
	delamo::TPoint3<double> vector1; /**< Reference normal of the Layer 1 */
	char* name2; /**< Name of the Layer 2 */
	delamo::TPoint3<double> point2; /**< Reference point of the Layer 2 */
	delamo::TPoint3<double> vector2; /**< Reference normal of the Layer 2 */
	DelaminationType bcType; /**< Type of the boundary condition between Layer 1 and Layer 2 */
};

#endif // !APICONFIG_H
