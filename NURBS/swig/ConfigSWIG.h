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

#ifndef CONFIGSWIG_H
#define CONFIGSWIG_H

// CPP includes
#include <vector>
#include <map>

/*
* Convenience macros for declaring SWIG collections, with correspondingly named typedefs
* @see: https://www.softwariness.com/articles/api-design-for-swig/
* @example:
*   SWIG_DECLARE_VECTOR(mystring, StringVector);
*   SWIG_DECLARE_MAP(mystring, myint, StringToIntMap);
*/
#ifdef SWIG
#define SWIG_DECLARE_VECTOR(MemberType, TypeName) namespace std { %template(TypeName) vector<MemberType>; } typedef std::vector<MemberType> TypeName;
#define SWIG_DECLARE_MAP(KeyType, ValueType, TypeName) namespace std { %template(TypeName) map<KeyType, ValueType>; } typedef std::map<KeyType, ValueType> TypeName;
#else
#define SWIG_DECLARE_VECTOR(MemberType, TypeName) typedef std::vector<MemberType> TypeName;
#define SWIG_DECLARE_MAP(KeyType, ValueType, TypeName) typedef std::map<KeyType, ValueType> TypeName;
#endif

#endif // !CONFIGSWIG_H
