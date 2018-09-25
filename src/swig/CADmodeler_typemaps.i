/* 
 * File: CADmodeler_typemaps.i
 * Module: CADmodeler
 *
 * "typemap" directives
 */

 // Enable SWIG typemap support
%include "typemaps.i"

/**
 * Swig "in" & "argout" typemap combination for the function ACISModelBuilder::find_closest_points()
 * 
 * Usage:
 *	>> point_list, normal_list, name_list = acis.find_closest_points([layer_list], [x, y, z]);
 */
%typemap(in) (delamo::TPoint3<double> point_in, delamo::TPoint3<double>*& point_list, delamo::TPoint3<double>*& normal_list, char**& name_list, int& list_size)
{
	// The input to the function is going to be a list which corresponds to "point_in"
	// No need to touch the other input parameters, as they will be handled inside the C++ method
	if (!PyList_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a list which corresponds to (x, y, z) coordinates!");
		return NULL;
	}

	// Traverse through the input list to obtain the contents
	double p[3];
	for (int i = 0; i < 3; i++)
	{
		PyObject* val = PyList_GetItem($input, i);
		if (PyLong_Check(val))
		{
			p[i] = (double)PyLong_AsSsize_t(val);
		}
		else if (PyFloat_Check(val))
		{
			p[i] = PyFloat_AsDouble(val);
		}
	}
	// Convert the input list into the C++ object, delamo::TPoint3<double>
	$1.set_x(p[0]);
	$1.set_y(p[1]);
	$1.set_z(p[2]);

	// Allocation of memory with zero bits is necessary because malloc doesn't call constructors
	// SWIG converts "**&" to "***" and therefore, it is required to add an additional instantiation and deletion step
	$2 = ($2_ltype) calloc(1, sizeof(delamo::TPoint3<double>*));
	$3 = ($3_ltype) calloc(1, sizeof(delamo::TPoint3<double>*));
	$4 = ($4_ltype) calloc(1, sizeof(char**));
	$5 = ($5_ltype) calloc(1, sizeof(int));
}

%typemap(argout) (delamo::TPoint3<double> point_in, delamo::TPoint3<double>*& point_list, delamo::TPoint3<double>*& normal_list, char**& name_list, int& list_size)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// The C++ method will fill point_list, normal_list, name_list, list_size automatically and will not touch to point_in
	// We will be returning a a list of 3 lists, which these lists have the size of list_size
	
	// Initialize the return list
	$result = PyList_New(3);

	// Initialize the point and the normal list
	PyObject* plist = PyList_New(*$5);
	PyObject* nlist = PyList_New(*$5);
	PyObject* names = PyList_New(*$5);

	// Traverse through both lists to convert Geometry3 into a 3-element list
	for (int i = 0; i < *$5; i++)
	{
		// Convert the point into a Python list
		PyObject* point_pyobj = PyList_New(3);
		PyList_SetItem(point_pyobj, 0, PyFloat_FromDouble($2[0][i].x()));
		PyList_SetItem(point_pyobj, 1, PyFloat_FromDouble($2[0][i].y()));
		PyList_SetItem(point_pyobj, 2, PyFloat_FromDouble($2[0][i].z()));

		// Add the point to the Python point list
		PyList_SetItem(plist, i, point_pyobj);
		
		// Convert the normal into a Python list
		PyObject* normal_pyobj = PyList_New(3);
		PyList_SetItem(normal_pyobj, 0, PyFloat_FromDouble($3[0][i].x()));
		PyList_SetItem(normal_pyobj, 1, PyFloat_FromDouble($3[0][i].y()));
		PyList_SetItem(normal_pyobj, 2, PyFloat_FromDouble($3[0][i].z()));

		// Add the normal to the Python point list
		PyList_SetItem(nlist, i, normal_pyobj);

		// Add the layer body name to the Python list
		%#if PY_MAJOR_VERSION == 2
		PyList_SetItem(names, i, PyString_FromString($4[0][i]));
		%#else
		PyList_SetItem(names, i, PyUnicode_FromString($4[0][i]));
		%#endif
	}

	// We have propagated the Python side of the point, normal and name list. Now add them to the return list.
	PyList_SetItem($result, 0, plist);
	PyList_SetItem($result, 1, nlist);
	PyList_SetItem($result, 2, names);

	// Deallocate memory which was allocated by "new"
	delete[] $2[0];
	delete[] $3[0];
	// Delete char**& array
	for (int i = 0; i < *$5; i++)
	{
		// C-style strings can only be deleted by free
		free((char*)$4[0][i]);
	}
	delete[] $4[0];

	// Deallocate memory which was allocated by "calloc"
	free($2);
	free($3);
	free($4);
	free($5);
}

/**
 * Swig "in" & "argout" typemap combination for the function ACISModelBuilder::find_closest_face_to_point()
 *
 * Example code:
 *	>> point, normal, name = acis.find_closest_face_to_point(layer_in, [x, y, z])
 */
%typemap(in) (delamo::TPoint3<double> point_in, delamo::TPoint3<double>& point_out, delamo::TPoint3<double>& normal_out, char*& name_out)
{
	// The input to the function is going to be a list which corresponds to "point_in"
	// No need to touch the other input parameters, as they will be handled inside the C++ method
	if (!PyList_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a list which corresponds to (x, y, z) coordinates!");
		return NULL;
	}

	// Traverse through the input list to obtain the contents
	double p[3];
	for (int i = 0; i < 3; i++)
	{
		PyObject* val = PyList_GetItem($input, i);
		if (PyLong_Check(val))
		{
			p[i] = (double)PyLong_AsSsize_t(val);
		}
		else if (PyFloat_Check(val))
		{
			p[i] = PyFloat_AsDouble(val);
		}
	}
	// Convert the input list into the C++ object, delamo::TPoint3<double>
	$1.set_x(p[0]);
	$1.set_y(p[1]);
	$1.set_z(p[2]);

	// Allocate memory for the output parameters (SWIG converts "&" to "*" automatically)
	$2 = ($2_ltype) calloc(1, sizeof(delamo::TPoint3<double>));
	$3 = ($3_ltype) calloc(1, sizeof(delamo::TPoint3<double>));
	$4 = ($4_ltype) calloc(1, sizeof(char*));
}

%typemap(argout) (delamo::TPoint3<double> point_in, delamo::TPoint3<double>& point_out, delamo::TPoint3<double>& normal_out, char*& name_out)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// The C++ method will fill point_out, normal_out and name_out automatically and will not touch to point_in
	// We will be returning a a list of 2 lists, which these lists have the size of 3 corresponding to the (x, y, z) coordinates, and 1 string

	// Initialize the return list
	$result = PyList_New(3);

	// Convert the point into a Python list
	PyObject* point_pyobj = PyList_New(3);
	PyList_SetItem(point_pyobj, 0, PyFloat_FromDouble($2[0].x()));
	PyList_SetItem(point_pyobj, 1, PyFloat_FromDouble($2[0].y()));
	PyList_SetItem(point_pyobj, 2, PyFloat_FromDouble($2[0].z()));

	// Convert the normal into a Python list
	PyObject* normal_pyobj = PyList_New(3);
	PyList_SetItem(normal_pyobj, 0, PyFloat_FromDouble($3[0].x()));
	PyList_SetItem(normal_pyobj, 1, PyFloat_FromDouble($3[0].y()));
	PyList_SetItem(normal_pyobj, 2, PyFloat_FromDouble($3[0].z()));

	// We have propagated the Python side of the point, normal and names list. Now add them to the return list.
	PyList_SetItem($result, 0, point_pyobj);
	PyList_SetItem($result, 1, normal_pyobj);
	
	%#if PY_MAJOR_VERSION == 2
		PyList_SetItem($result, 2, PyString_FromString($4[0]));
	%#else
		PyList_SetItem($result, 2, PyUnicode_FromString($4[0]));
	%#endif

	// Free the memory allocated by the "in" typemap
	free($2);
	free($3);
	free($4);
}

/**
 * Swig "in" & "argout" typemap combination for the function ACISModelBuilder::save()
 * 
 * Example code:
 *	>> bodyname_list = acis.save("SaveFileName")
 */
%typemap(in) (const char* file_name, char**& bodynames, int& num_bodynames)
{
// @see http://stackoverflow.com/questions/36750580/generating-swig-code-based-on-python-version
%#if PY_MAJOR_VERSION == 2
	// Check if the input is correct
	if (!PyString_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a string containing the file name for save operation!");
		return NULL;
	}

	// Set file_name
	$1 = PyString_AsString($input);
%#else
	// Check if the input is correct
	if (!PyUnicode_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting a string containing the file name for save operation!");
		return NULL;
	}

	// Set file_name
	$1 = PyUnicode_AsUTF8($input);
%#endif

	// Allocate memory for char**& (SWIG converts it to char***)
	$2 = ($2_ltype) calloc(1, sizeof(char**));

	// Allocate memory for int*
	$3 = ($3_ltype) calloc(1, sizeof(int));
}

%typemap(argout) (const char* file_name, char**& bodynames, int& num_bodynames)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Create a list to be used as the output
	$result = PyList_New(*$3);

	// Traverse through bodynames list to fill the output list
	for (int i = 0; i < *$3; i++)
	{
		// Add the layer body name to the Python list
		%#if PY_MAJOR_VERSION == 2
			PyList_SetItem($result, i, PyString_FromString($2[0][i]));
		%#else
			PyList_SetItem($result, i, PyUnicode_FromString($2[0][i]));
		%#endif
	}

	// Delete char**& array which was instantiated by the save function
	for (int i = 0; i < *$3; i++)
	{
		// C-style strings can only be freed from the memory by C-function "free"
		free((char*)$2[0][i]);
	}
	// Deallocate memory allocated by "new"
	delete[] $2[0];

	// Free the memory allocated by the "in" typemap
	free($2);
	free($3);
}

/**
 * Swig "in" & "argout" typemap combination for the function ACISModelBuilder::adjacent_layers() with FAL outputs
 * 
 * Example code:
 *	>> fal12 = acis.adjacent_layers(layer1, layer2, delamo.CADwrap.DEFAULT_BC_CONTACT)
 */
%typemap(in) (BCStatus)
{
	%#if PY_MAJOR_VERSION == 2
	// Check if the input is correct
	if (!PyInt_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting an integer for BCStatus!");
		return NULL;
	}

	// The input should be assigned to the first variable
	$1 = ($1_ltype) PyInt_AsSsize_t($input);
	%#else
	// Check if the input is correct
	if (!PyLong_Check($input))
	{
		PyErr_SetString(PyExc_ValueError, "Expecting an integer for BCStatus!");
		return NULL;
	}

	// The input should be assigned to the first variable
	$1 = ($1_ltype) PyLong_AsSsize_t($input);
	%#endif
}

%typemap(in,numinputs=0) (FaceAdjacency*& fal, int& fal_size)
{

	// Initialize FaceAdjacency** pointer
	$1 = ($1_ltype) calloc(1, sizeof(FaceAdjacency*));

	// Initialize int* pointer
	$2 = ($2_ltype) calloc(1, sizeof(int));
}


%typemap(argout) (FaceAdjacency*& fal, int& fal_size)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Convert the FAL to native python list
	$result = PyList_New(*$2);

	for (int i = 0; i < *$2; i++)
	{
		// Create a new dictionary
		PyObject* item_dict = PyDict_New();
		
		// Copy the names to the dictionary
		%#if PY_MAJOR_VERSION == 2
			PyDict_SetItemString(item_dict, "name1", PyString_FromString($1[0][i].name1));
			PyDict_SetItemString(item_dict, "name2", PyString_FromString($1[0][i].name2));
		%#else
			PyDict_SetItemString(item_dict, "name1", PyUnicode_FromString($1[0][i].name1));
			PyDict_SetItemString(item_dict, "name2", PyUnicode_FromString($1[0][i].name2));
		%#endif

		// Copy the normal and point of the first layer to the dictionary
		PyObject* point1 = PyList_New(3);
		PyList_SetItem(point1, 0, PyFloat_FromDouble($1[0][i].point1.x()));
		PyList_SetItem(point1, 1, PyFloat_FromDouble($1[0][i].point1.y()));
		PyList_SetItem(point1, 2, PyFloat_FromDouble($1[0][i].point1.z()));
		PyDict_SetItemString(item_dict, "point1", point1);
		PyObject* normal1 = PyList_New(3);
		PyList_SetItem(normal1, 0, PyFloat_FromDouble($1[0][i].vector1.x()));
		PyList_SetItem(normal1, 1, PyFloat_FromDouble($1[0][i].vector1.y()));
		PyList_SetItem(normal1, 2, PyFloat_FromDouble($1[0][i].vector1.z()));
		PyDict_SetItemString(item_dict, "normal1", normal1);

		// Copy the normal and point of the second layer to the dictionary
		PyObject* point2 = PyList_New(3);
		PyList_SetItem(point2, 0, PyFloat_FromDouble($1[0][i].point2.x()));
		PyList_SetItem(point2, 1, PyFloat_FromDouble($1[0][i].point2.y()));
		PyList_SetItem(point2, 2, PyFloat_FromDouble($1[0][i].point2.z()));
		PyDict_SetItemString(item_dict, "point2", point2);
		PyObject* normal2 = PyList_New(3);
		PyList_SetItem(normal2, 0, PyFloat_FromDouble($1[0][i].vector2.x()));
		PyList_SetItem(normal2, 1, PyFloat_FromDouble($1[0][i].vector2.y()));
		PyList_SetItem(normal2, 2, PyFloat_FromDouble($1[0][i].vector2.z()));
		PyDict_SetItemString(item_dict, "normal2", normal2);

		// Copy bcType to the dictionary
		%#if PY_MAJOR_VERSION == 2
			PyDict_SetItemString(item_dict, "bcType", PyInt_FromSsize_t($1[0][i].bcType));
		%#else
			PyDict_SetItemString(item_dict, "bcType", PyLong_FromSsize_t($1[0][i].bcType));
		%#endif

		// Add the dictionary to the return list
		PyList_SetItem($result, i, item_dict);
	}

	// Free the memory allocated in the C++ method
	delete[] $1[0];
	// Free the memory allocated in the typemap
	free($1);
}

/**
 * Swig "in" & "argout" typemap combination for the function Layer::bodynames()
 * 
 * Example code:
 *	>> names_list = layer.bodynames()
 */
%typemap(in,numinputs=0,noblock=1) (char**& namelistout, int& namelistout_size)
{
	// Allocate memory for char**& (SWIG converts it to char***)
	$1 = ($1_ltype) calloc(1, sizeof(char**));

	// Allocate memory for int*
	$2 = ($2_ltype) calloc(1, sizeof(int));
}

%typemap(argout) (char**& namelistout, int& namelistout_size)
{
	// Blow away any previous result
	Py_XDECREF($result);

	// Create a list to be used as the output
	$result = PyList_New(*$2);

	// Traverse through bodynames list to fill the output list
	for (int i = 0; i < *$2; i++)
	{
		// Add the layer body name to the Python list
		%#if PY_MAJOR_VERSION == 2
			PyList_SetItem($result, i, PyString_FromString($1[0][i]));
		%#else
			PyList_SetItem($result, i, PyUnicode_FromString($1[0][i]));
		%#endif
	}

	// Delete char**& array which was instantiated by the save function
	for (int i = 0; i < *$2; i++)
	{
		// C-style strings can only be freed from the memory by C-function "free"
		free((char*)$1[0][i]);
	}
	// Deallocate memory allocated by "new"
	delete[] $1[0];

	// Free the memory allocated by the "in" typemap
	free($1);
	free($2);
}
