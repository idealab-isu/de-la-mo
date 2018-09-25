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
 * File: CADsupport_typemaps.i
 * Module: CADsupport
 *
 * "typemap" directives
 */

 // Include SWIG typemap support
 %include "typemaps.i"

/*
 * "in" and "freearg" typemaps
 *
 * These typemaps convert inputs from the target language to the C/C++
 */

%typemap(in) (double* IN_ARRAY, int DIM)
{
	if (!PyList_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a list");
		return NULL;
	}

	// Referring the second input, DIM: dimension
	$2 = ($2_ltype) PyList_Size($input);

	// Referring the first input, IN_ARRAY: input pointer array
	$1 = ($1_ltype) malloc(sizeof($*1_ltype) * $2);

	for (int i = 0; i < $2; i++)
	{
		// Create a temporary double
		$*1_ltype val_temp;

		// Get the i-th item from the input list
		PyObject* val = PyList_GetItem($input, i);

		// Check the list item is a floating point number
		if (!PyFloat_Check(val))
		{
			// Check as cast integers to doubles
			if (!PyNumber_Check(val))
			{
				// The list item is something else
				free($1);
				PyErr_SetString(PyExc_ValueError, "List items must be floating point numbers");
				return NULL;
			}
			else
			{
				// The list item is integer and we can cast it to double
				%#if PY_MAJOR_VERSION == 2
				val_temp = ($*1_ltype)PyInt_AsLong(val);
				%#else
				val_temp = ($*1_ltype)PyLong_AsDouble(val);
				%#endif
			}
		}
		else
		{
			// The list item is a floating point number
			val_temp = PyFloat_AsDouble(val);
		}

		// Set the IN_ARRAY element
		$1[i] = val_temp;
	}
}

%typemap(freearg) (double* IN_ARRAY, int DIM)
{
	if($1)
	{
		free($1);
	}
}

%typemap(in) (TPoint3<double>* IN_ARRAY, int DIM1, int DIM2)
{
	if (!PyList_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a list");
		return NULL;
	}

	// Referring the third input, length of the v-axis, DIM2
	$3 = ($3_ltype) PyList_Size($input);

	// We need to find the length of u-axis, DIM1
	PyObject* val_temp = PyList_GetItem($input, 1);
	if (!PyList_Check(val_temp))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a nested list");
		return NULL;
	}
	$2 = ($2_ltype) PyList_Size(val_temp);

	// Refering the first input parameter, IN_ARRAY: the pointer array
	$1 = ($1_ltype) malloc(sizeof($*1_ltype) * $2 * $3);

	int cntr = 0;
	for (int i = 0; i < $3; i++)
	{
		// Get the i-th item from the Python list
		PyObject* val_v = PyList_GetItem($input, i);

		// The i-th item should be a list of u-values
		if (!PyList_Check(val_v))
		{
			free($1);
			PyErr_SetString(PyExc_ValueError, "Expecting a nested list");
			return NULL;
		}

		for (int j = 0; j < $2; j++)
		{
			PyObject* val = PyList_GetItem(val_v, j);
			
			// All list items should be a Python list too
			if (!PyList_Check(val))
			{
				free($1);
				PyErr_SetString(PyExc_ValueError, "Expecting a nested list");
				return NULL;
			}
			
			// We use "double" data type here
			double val_temp_x, val_temp_y, val_temp_z;

			PyObject* val_x = PyList_GetItem(val, 0);
			if (!PyFloat_Check(val_x))
			{
				// Check as cast integers to doubles
				if (!PyNumber_Check(val_x))
				{
					// The list item is something else
					free($1);
					PyErr_SetString(PyExc_ValueError, "List items must be floating point numbers (x)");
					return NULL;
				}
				else
				{
					// The list item is integer and we can cast it to double
					%#if PY_MAJOR_VERSION == 2
					val_temp_x = (double)PyInt_AsLong(val_x);
					%#else
					val_temp_x = PyLong_AsDouble(val_x);
					%#endif
				}
			}
			else
			{
				// The list item is a floating point number
				val_temp_x = PyFloat_AsDouble(val_x);
			}
		
			PyObject* val_y = PyList_GetItem(val, 1);
			if (!PyFloat_Check(val_y))
			{
				// Check as cast integers to doubles
				if (!PyNumber_Check(val_y))
				{
					// The list item is something else
					free($1);
					PyErr_SetString(PyExc_ValueError, "List items must be floating point numbers (y)");
					return NULL;
				}
				else
				{
					// The list item is integer and we can cast it to double
					%#if PY_MAJOR_VERSION == 2
					val_temp_y = (double)PyInt_AsLong(val_y);
					%#else
					val_temp_y = PyLong_AsDouble(val_y);
					%#endif
				}
			}
			else
			{
				// The list item is a floating point number
				val_temp_y = PyFloat_AsDouble(val_y);
			}
		
			PyObject* val_z = PyList_GetItem(val, 2);
			if (!PyFloat_Check(val_z))
			{
				// Check as cast integers to doubles
				if (!PyNumber_Check(val_z))
				{
					// The list item is something else
					free($1);
					PyErr_SetString(PyExc_ValueError, "List items must be floating point numbers (z)");
					return NULL;
				}
				else
				{
					// The list item is integer and we can cast it to double
					%#if PY_MAJOR_VERSION == 2
					val_temp_z = (double)PyInt_AsLong(val_z);
					%#else
					val_temp_z = PyLong_AsDouble(val_z);
					%#endif
				}
			}
			else
			{
				// The list item is a floating point number
				val_temp_z = PyFloat_AsDouble(val_z);
			}
		
			// Create a temporary TPoint3<double> object
			$*1_ltype temp;
			temp.set_x(val_temp_x);
			temp.set_y(val_temp_y);
			temp.set_z(val_temp_z);

			// Add the temporary TPoint3<double> object to the input pointer array
			$1[cntr] = temp;
			cntr++;
		}
	}
}

%typemap(freearg) (Geometry3<double>* IN_ARRAY, int DIM1, int DIM2)
{
	if($1)
	{
		free($1);
	}
}

/* ##### * ##### */

/*
 * "in" and "argout" typemaps
 *
 * These typemaps convert output parameters in the function signature of C/C++ method tom the target language structures
 */

/* 
 * Swig "in" typemap for NURBS<double>::derivatives() method
 *
 * Function signature on the Python side: derivatives(double u, double v, int order)
 * where:
 *	u: u-coordinate
 *	v: v-coordinate
 *	order: derivative order, i.e. order = 2 means "calculate up to 2nd derivative of the surface"
 */
%typemap(in) (int d, TPoint3<double>**& SKL)
{
	bool check;
	%#if PY_MAJOR_VERSION == 2
	check = PyInt_Check($input);
	%#else
	check = PyLong_Check($input);
	%#endif

	if (!check)
	{
		PyErr_SetString(PyExc_ValueError, "Expecting an integer");
		return NULL;
	}

	// 3rd parameter coming from Python should be "order"
	%#if PY_MAJOR_VERSION == 2
	$1 = ($1_ltype)PyInt_AsLong($input);
	%#else
	$1 = ($1_ltype)PyLong_AsLong($input);
	%#endif

	// Allocate memory for the Python side for SKL variable and initialize the memory to zero
	// Allocation of memory with zero bits is necessary because malloc doesn't call constructors
	// SWIG converts "**&" to "***" and therefore, it is required to add an additional instantiation and deletion step
	$2 = ($2_ltype) calloc(1, sizeof(delamo::TPoint3<double>**));
}

/*
 * Swig "argout" typemap for NURBS<double>::derivatives() method
 *
 * This typemap allows us to get the value of SKL as the function return value.
 *
 * Ex1: Find the 2nd derivative of the surface w.r.t to v axis at u = 0.1 and v = 0.4
 *	>>> skl = surf.derivatives(0.1, 0.4, 2)
 *	>>> skl[0][2]
 *
 * Ex2: Find the 1st derivative of the surface w.r.t to u and v axis at u = 0.2 and v = 0.9
 *	>>> skl = surf.derivatives(0.2, 0.9, 1)
 *	>>> skl[1][1]
 */
%typemap(argout) (int d, TPoint3<double>**& SKL)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Create the first level Python list
	$result = PyList_New($1 + 1);

	// Create the second level Python list
	for (int i = 0; i < $1 + 1; i++)
	{
		PyObject* temp_list = PyList_New($1 + 1);
		for (int j = 0; j < $1 + 1; j++)
		{
			PyObject* coord_list = PyList_New(3);
			PyList_SetItem(coord_list, 0, PyFloat_FromDouble($2[0][i][j].x()));
			PyList_SetItem(coord_list, 1, PyFloat_FromDouble($2[0][i][j].y()));
			PyList_SetItem(coord_list, 2, PyFloat_FromDouble($2[0][i][j].z()));
			PyList_SetItem(temp_list, j, coord_list);
		}
		PyList_SetItem($result, i, temp_list);
	}

	// Deallocate SKL pointer which was allocated by "new"
	for (int i = 0; i <= $1; i++)
	{
		delete[] $2[0][i];
	}
	delete $2[0];

	// Deallocate SWIG pointer which was allocated by "calloc"
	free($2);
}

%typemap(in) (double v_value, TPoint3<double>& out_value)
{
	if (!PyFloat_Check($input))
	{
		if (!PyNumber_Check($input))
		{
			PyErr_SetString(PyExc_ValueError, "Expecting a number");
			return NULL;
		}
		else
		{
			// Initialize "v_value" by converting Python int to double
			%#if PY_MAJOR_VERSION == 2
			$1 = ($1_ltype)PyInt_AsLong($input);
			%#else
			$1 = ($1_ltype)PyLong_AsDouble($input);
			%#endif
		}
	}
	else
	{
		// Initialize "v_value" directly from Python double
		$1 = PyFloat_AsDouble($input);
	}

	// Allocate memory
	$2 = ($2_ltype) calloc(1, sizeof(delamo::TPoint3<double>));
}

%typemap(argout) (double v_value, TPoint3<double>& out_value)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Prepare the output Python list
	$result = PyList_New(3);

	// Add x, y, z coordinates to the output Python list
	PyList_SetItem($result, 0, PyFloat_FromDouble($2->x()));
	PyList_SetItem($result, 1, PyFloat_FromDouble($2->y()));
	PyList_SetItem($result, 2, PyFloat_FromDouble($2->z()));

	// Deallocate memory
	free($2);
}

%typemap(in) (double v_value, TVector3<double>& out_value)
{
	if (!PyFloat_Check($input))
	{
		if (!PyNumber_Check($input))
		{
			PyErr_SetString(PyExc_ValueError, "Expecting a number");
			return NULL;
		}
		else
		{
			// Initialize "v_value" by converting Python int to double
			%#if PY_MAJOR_VERSION == 2
			$1 = ($1_ltype)PyInt_AsLong($input);
			%#else
			$1 = ($1_ltype)PyLong_AsDouble($input);
			%#endif
		}
	}
	else
	{
		// Initialize "v_value" directly from Python double
		$1 = PyFloat_AsDouble($input);
	}

	// Allocate memory
	$2 = ($2_ltype) calloc(1, sizeof(delamo::TVector3<double>));
}

%typemap(argout) (double v_value, TVector3<double>& out_value)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Prepare the output Python list
	$result = PyList_New(3);

	// Add x, y, z coordinates to the output Python list
	PyList_SetItem($result, 0, PyFloat_FromDouble($2->x()));
	PyList_SetItem($result, 1, PyFloat_FromDouble($2->y()));
	PyList_SetItem($result, 2, PyFloat_FromDouble($2->z()));

	// Deallocate memory
	free($2);
}

/* ##### * ##### */

/*
 * "out" typemaps
 *
 * These typemaps convert outputs of the C/C++ methods (lhs) to the target language elements, i.e list, dicts, etc.
 */

// TPoint3<double> return values
%typemap(out) TPoint3<double>
{
	// Prepare the output Python list
	$result = PyList_New(3);

	// Add x, y, z coordinates to the output Python list
	PyList_SetItem($result, 0, PyFloat_FromDouble($1.x()));
	PyList_SetItem($result, 1, PyFloat_FromDouble($1.y()));
	PyList_SetItem($result, 2, PyFloat_FromDouble($1.z()));
}

// TVector3<double> return values
%typemap(out) TVector3<double>
{
	// Prepare the output Python list
	$result = PyList_New(3);

	// Add x, y, z coordinates to the output Python list
	PyList_SetItem($result, 0, PyFloat_FromDouble($1.x()));
	PyList_SetItem($result, 1, PyFloat_FromDouble($1.y()));
	PyList_SetItem($result, 2, PyFloat_FromDouble($1.z()));
}


// DoubleStruct return values
%typemap(out) DoubleStruct
{
	// Prepare the Python list
	$result = PyList_New($1.ptr_size);
	for (int i = 0; i < $1.ptr_size; i++)
	{
		PyList_SetItem($result, i, PyFloat_FromDouble($1.ptr_array[i]));
	}
}

// Point3Struct return values
%typemap(out) Point3Struct
{
	// Prepare the Python list
	$result = PyList_New($1.ptr_size_v);
	int cntr = 0;
	for (int i = 0; i < $1.ptr_size_v; i++)
	{
		PyObject* temp_v = PyList_New($1.ptr_size_u);
		for (int j = 0; j < $1.ptr_size_u; j++)
		{
			PyObject* temp_u = PyList_New(3);
			PyList_SetItem(temp_u, 0, PyFloat_FromDouble($1.ptr_array[cntr].x()));
			PyList_SetItem(temp_u, 1, PyFloat_FromDouble($1.ptr_array[cntr].y()));
			PyList_SetItem(temp_u, 2, PyFloat_FromDouble($1.ptr_array[cntr].z()));
			PyList_SetItem(temp_v, j, temp_u);
			cntr++;
		}
		PyList_SetItem($result, i, temp_v);
	}
}

// Vector3Struct return values
%typemap(out) Vector3Struct
{
	// Prepare the Python list
	$result = PyList_New($1.ptr_size_v);
	int cntr = 0;
	for (int i = 0; i < $1.ptr_size_v; i++)
	{
		PyObject* temp_v = PyList_New($1.ptr_size_u);
		for (int j = 0; j < $1.ptr_size_u; j++)
		{
			PyObject* temp_u = PyList_New(3);
			PyList_SetItem(temp_u, 0, PyFloat_FromDouble($1.ptr_array[cntr].x()));
			PyList_SetItem(temp_u, 1, PyFloat_FromDouble($1.ptr_array[cntr].y()));
			PyList_SetItem(temp_u, 2, PyFloat_FromDouble($1.ptr_array[cntr].z()));
			PyList_SetItem(temp_v, j, temp_u);
			cntr++;
		}
		PyList_SetItem($result, i, temp_v);
	}
}

/* ##### * ##### */

/*
 * "apply" directives
 *
 * These are required for applying typemap patterns to various function signature patterns
 */

%apply (double* IN_ARRAY, int DIM)
{
	(double* knotvector, int num_knotvector),
	(double* weights, int num_weights)
};

%apply (TPoint3<double>* IN_ARRAY, int DIM1, int DIM2)
{
	(TPoint3<double>* ctrlpts, int ctrlpts_u_len, int ctrlpts_v_len)
};
