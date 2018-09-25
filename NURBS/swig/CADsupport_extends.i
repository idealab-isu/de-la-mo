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
 * File: CADsupport_extends.i
 * Module: CADsupport
 *
 * "extend" directives
 */

// Extending TPoint3<double> class
%extend delamo::TPoint3<double>
{
	void __setitem__(unsigned int idx, double val) throw (std::out_of_range)
	{
		if (idx >= 3)
			throw std::out_of_range("in TPoint3::__setitem__()");
		(*self)[idx]=val;
	}

	double __getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx >= 3)
			throw std::out_of_range("in TPoint3::__getitem__()");
		return (*self)[idx];
	}
	
	// Implement Python @property decorator using Python's "property" class
	// @see: https://docs.python.org/2/library/functions.html#property
	// @note: Check CADsupport_renames.i file to find getters and setters
	%pythoncode %{
		__swig_getmethods__["x"] = _getX
		__swig_setmethods__["x"] = _setX
		__swig_getmethods__["y"] = _getY
		__swig_setmethods__["y"] = _setY
		__swig_getmethods__["z"] = _getZ
		__swig_setmethods__["z"] = _setZ
		if _newclass: 
			x = property(_getX, _setX)
			y = property(_getY, _setY)
			z = property(_getZ, _setZ)
    %}
};

// Extending TVector3<double> class
%extend delamo::TVector3<double>
{	
	// Implement Python @property decorator using Python's "property" class
	// @see: https://docs.python.org/2/library/functions.html#property
	// @note: Check CADsupport_renames.i file to find getters and setters
	%pythoncode %{
		__swig_getmethods__["x"] = _getX
		__swig_getmethods__["y"] = _getY
		__swig_getmethods__["z"] = _getZ
		if _newclass: 
			x = property(_getX)
			y = property(_getY)
			z = property(_getZ)
    %}
};

// Extending NURBS<double> class
%extend delamo::NURBS<double>
{
	// A custom getter for SWIG to return a DoubleStruct struct which is defined in CADsupport.i file
	DoubleStruct _get_knotvector_u()
	{
		DoubleStruct ret;
		ret.ptr_array = self->knotvector_u();
		ret.ptr_size = self->knotvector_u_len();
		return ret;
	}

	// A custom getter for SWIG to return a DoubleStruct struct which is defined in CADsupport.i file
	DoubleStruct _get_knotvector_v()
	{
		DoubleStruct ret;
		ret.ptr_array = self->knotvector_v();
		ret.ptr_size = self->knotvector_v_len();
		return ret;
	}

	// A custom getter for SWIG to return a DoubleStruct struct which is defined in CADsupport.i file
	DoubleStruct _get_weights()
	{
		DoubleStruct ret;
		ret.ptr_array = self->weights();
		ret.ptr_size = self->ctrlpts_len();
		return ret;
	}

	// A custom getter for SWIG to return a Geometry3Struct struct which is defined in CADsupport.i file
	Point3Struct _get_ctrlpts()
	{
		Point3Struct ret;
		ret.ptr_array = self->ctrlpts();
		ret.ptr_size_u = self->ctrlpts_u_len();
		ret.ptr_size_v = self->ctrlpts_v_len();
		return ret;
	}

	// A custom getter for SWIG to return a Geometry3Struct struct which is defined in CADsupport.i file
	Point3Struct _get_surfpts()
	{
		Point3Struct ret;
		ret.ptr_array = self->surfpts();
		ret.ptr_size_u = self->surfpts_u_len();
		ret.ptr_size_v = self->surfpts_v_len();
		return ret;
	}

	// Implement Python @property decorator using Python's "property" class
	// @see: https://docs.python.org/2/library/functions.html#property
	// @note: Check CADsupport_renames.i file to find getters and setters
	%pythoncode %{
		__swig_getmethods__["degree_u"] = _get_degree_u
		__swig_setmethods__["degree_u"] = _set_degree_u
		__swig_getmethods__["degree_v"] = _get_degree_v
		__swig_setmethods__["degree_v"] = _set_degree_v
		__swig_getmethods__["knotvector_u"] = _get_knotvector_u
		__swig_setmethods__["knotvector_u"] = _set_knotvector_u
		__swig_getmethods__["knotvector_v"] = _get_knotvector_v
		__swig_setmethods__["knotvector_v"] = _set_knotvector_v
		__swig_getmethods__["weights"] = _get_weights
		__swig_setmethods__["weights"] = _set_weights
		__swig_getmethods__["ctrlpts"] = _get_ctrlpts
		__swig_setmethods__["ctrlpts"] = _set_ctrlpts
		if _newclass: 
			degree_u = property(_get_degree_u, _set_degree_u)
			degree_v = property(_get_degree_v, _set_degree_v)
			knotvector_u = property(_get_knotvector_u, _set_knotvector_u)
			knotvector_v = property(_get_knotvector_v, _set_knotvector_v)
			weights = property(_get_weights, _set_weights)
			ctrlpts = property(_get_ctrlpts, _set_ctrlpts)
			surfpts = property(_get_surfpts);
    %}
};


%extend delamo::List< TPoint3< double > >
{
	TPoint3<double> __getitem__(unsigned int idx) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in TPoint3<double>List::__getitem__()");
		return (*self)[idx];
	}

	void __setitem__(unsigned int idx, TPoint3<double> val) throw (std::out_of_range)
	{
		if (idx>=self->size())
			throw std::out_of_range("in TPoint3<double>List::__setitem__()");
		(*self)[idx]=val;
	}

	unsigned int __len__()
	{
		return self->size();
	}
}
