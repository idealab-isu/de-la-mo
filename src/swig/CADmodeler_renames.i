/* 
 * File: CADmodeler_renames.i
 * Module: CADmodeler
 *
 * "rename" directives
 */

// Ignoring assignment(=) and subscript([]) operators that can't be wrapped in Python
%rename("$ignore") *::operator=;
%rename("$ignore") *::operator[];

// Ignoring std::initializer_list constructor as instructed in http://www.swig.org/Doc3.0/CPlusPlus11.html#CPlusPlus11_initializer_lists
%rename("$ignore") delamo::List< std::string >::List(std::initializer_list< std::string >);
%rename("$ignore") delamo::List< FaceAdjacency *>::List(std::initializer_list< FaceAdjacency >);
%rename("$ignore") delamo::List< Layer *>::List(std::initializer_list< Layer >);
%rename("$ignore") delamo::List< LayerMold *>::List(std::initializer_list< LayerMold >);

// Ignore some ModelBuilder functions, as we don't need them on the Python side
%rename("$ignore") ACISModelBuilder::save(const char* file_name);
%rename("$ignore") ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset);
%rename("$ignore") ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name);
%rename("$ignore") ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, delamo::List< std::string >& file_names);
%rename("$ignore") ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name, delamo::TPoint3<double>& pt_inside);
%rename("$ignore") ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, delamo::List< std::string >& file_names, delamo::List< delamo::TPoint3<double> >& pts_inside);
