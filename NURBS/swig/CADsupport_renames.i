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
 * File: CADsupport_renames.i
 * Module: CADsupport
 *
 * "rename" directives
 */

// Ignoring assignment(=) and subscript([]) operators that can't be wrapped in Python
%rename("$ignore") *::operator=;
%rename("$ignore") *::operator[];

// Ignoring std::initializer_list constructor as instructed in http://www.swig.org/Doc3.0/CPlusPlus11.html#CPlusPlus11_initializer_lists
%rename("$ignore") delamo::List< delamo::TPoint3<double> >::List(std::initializer_list< delamo::TPoint3<double> >);

// Some operators have custom signatures, so they need to be redefined
%rename(add_g3g3) operator+(TPoint3<double>, const TPoint3<double>&);
%rename(sub_g3g3) operator-(TPoint3<double>, const TPoint3<double>&);
%rename(mul_g3g3) operator*(TPoint3<double>, const TPoint3<double>&);
%rename(div_g3g3) operator/(TPoint3<double>, const TPoint3<double>&);
%rename(add_g3d) operator+(TPoint3<double>, const double&);
%rename(sub_g3d) operator-(TPoint3<double>, const double&);
%rename(mul_g3d) operator*(TPoint3<double>, const double&);
%rename(div_g3d) operator/(TPoint3<double>, const double&);
%rename(eq_g3g3) operator==(const TPoint3<double>&, const TPoint3<double>&);
%rename(ieq_g3g3) operator!=(const TPoint3<double>&, const TPoint3<double>&);

// Ignoring std::initializer_list constructor as instructed in http://www.swig.org/Doc3.0/CPlusPlus11.html#CPlusPlus11_initializer_lists
%rename("$ignore") delamo::List< delamo::TVector3<double> >::List(std::initializer_list< delamo::TVector3<double> >);

// Some operators have custom signatures, so they need to be redefined
%rename(add_g3g3) operator+(TVector3<double>, const TVector3<double>&);
%rename(sub_g3g3) operator-(TVector3<double>, const TVector3<double>&);
%rename(mul_g3g3) operator*(TVector3<double>, const TVector3<double>&);
%rename(div_g3g3) operator/(TVector3<double>, const TVector3<double>&);
%rename(add_g3d) operator+(TVector3<double>, const double&);
%rename(sub_g3d) operator-(TVector3<double>, const double&);
%rename(mul_g3d) operator*(TVector3<double>, const double&);
%rename(div_g3d) operator/(TVector3<double>, const double&);

// Hide the C++ getters-setters of the geometry classes from the Python interpreter
%rename(_getX) *::x();
%rename(_getXconst) *::x() const;
%rename(_setX) *::x(double);
%rename(_setX2) *::set_x(double);
%rename(_getY) *::y();
%rename(_getYconst) *::y() const;
%rename(_setY) *::y(double);
%rename(_setY2) *::set_y(double);
%rename(_getZ) *::z();
%rename(_getZconst) *::z() const;
%rename(_setZ) *::z(double);
%rename(_setZ2) *::set_z(double);
%rename(_getW) *::w();
%rename(_getWconst) *::w() const;
%rename(_setW) *::w(double);
%rename(_setW2) *::set_w(double);

// These are required for typemaps to work
%rename(_set_knotvector_u) delamo::NURBS<double>::knotvector_u(double*, int);
%rename(_set_knotvector_v) delamo::NURBS<double>::knotvector_v(double*, int);
%rename(_set_weights) delamo::NURBS<double>::weights(double*, int);
%rename(_set_ctrlpts) delamo::NURBS<double>::ctrlpts(TPoint3<double>*, int, int);

// Required for Python property decorator
%rename(_get_degree_u) delamo::NURBS<double>::degree_u();
%rename(_set_degree_u) delamo::NURBS<double>::degree_u(int);
%rename(_get_degree_v) delamo::NURBS<double>::degree_v();
%rename(_set_degree_v) delamo::NURBS<double>::degree_v(int);

// Ignore some methods in the NURBS class
%rename("$ignore", fullname=1) delamo::NURBS<double>::knotvector_u();
%rename("$ignore", fullname=1) delamo::NURBS<double>::knotvector_u(List<double>);
%rename("$ignore", fullname=1) delamo::NURBS<double>::knotvector_v();
%rename("$ignore", fullname=1) delamo::NURBS<double>::knotvector_v(List<double>);
%rename("$ignore", fullname=1) delamo::NURBS<double>::weights();
%rename("$ignore", fullname=1) delamo::NURBS<double>::weights(List<double>);
%rename("$ignore", fullname=1) delamo::NURBS<double>::ctrlpts();
%rename("$ignore", fullname=1) delamo::NURBS<double>::ctrlpts(List< TPoint3<double> >);
