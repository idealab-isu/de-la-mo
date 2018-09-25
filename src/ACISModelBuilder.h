#ifndef ACISMODELBUILDER_H
#define ACISMODELBUILDER_H

#include "ModelBuilder.h"
#include "ACIS.h"
#include "PNFind_UVseek.h"
#include "PNFind_BBox.h"


struct LSDistanceRel { LayerSurface* ls; double dist; };

// ModelBuilder sub-class for ACIS solid modeling kernel
class MODELBUILDER_EXPORT ACISModelBuilder : public ModelBuilder
{
public:

	/*
	 * \brief Default constructor
	 */
	ACISModelBuilder() : ModelBuilder() {};

	/**
	 * \brief ACIS constructor
	 * \param unlock_str License key for ACIS
	 */
	explicit ACISModelBuilder(const char* unlock_str) : ModelBuilder(unlock_str) {};

	/**
	 * \brief Default destructor
	 */
	~ACISModelBuilder() {};

	/**
	 * \brief Starts the ACIS modeler
	 */
	void start();

	/**
	 * \brief Stops the ACIS modeler
	 */
	void stop();

	/**
	 * \brief Creates a new layer from a NURBS surface
	 *
	 * \param[in] nurbs_surface_in input NURBS surface to be used as a mold for the new layer
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out new layer generated from the input NURBS surface
	 */
	void create_layer(delamo::NURBS<double> *nurbs_surface_in, Direction ldir, double thickness, Layer *layer_out);

	/**
	 * \brief Creates a new layer from a NURBS surface in the OFFSET direction
	 * \param[in] nurbs_surface_in input NURBS surface to be used as a mold for the new layer
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out new layer generated from the input NURBS surface
	 */
	void create_layer(delamo::NURBS<double> *nurbs_surface_in, double thickness, Layer *layer_out);

	/**
	 * \brief Creates a new layer using previously generated layers as a mold
	 *
	 * \param[in] layer_in input layer to be used as a mold
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out the new layer
	 */
	void create_layer(Layer *layer_in, Direction ldir, double thickness, Layer *layer_out);

	/**
	 * \brief Creates a new layer from a LayerMold object
	 * \param[in] mold_in input mold
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness layer thickness
	 * \param[out] layer_out the new layer
	 */
	void create_layer(LayerMold *mold_in, Direction ldir, double thickness, Layer *layer_out);

	/**
	 * \brief Imprints the the adjacent faces of the input layers to each other
	 *
	 * \param layer_orig layer on the ORIG side
	 * \param layer_offset layer on the OFFSET side
	 */
	void adjacent_layers(Layer *layer_orig, Layer *layer_offset);

	/**
	 * \brief Imprints delamination profile to the input layers
	 *
	 * \param layer_orig layer on the ORIG side
	 * \param layer_offset layer on the OFFSET side
	 * \param file_name CSV file containing the outer delamination profile
	 */
	void adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name);

	/**
	 * \brief Imprints multiple delamination profiles to the input layers
	 *
	 * \param layer_orig layer on the ORIG side
	 * \param layer_offset layer on the OFFSET side
	 * \param file_names a list of CSV files containing the outer delamination profile
	 */
	void adjacent_layers(Layer *layer_orig, Layer *layer_offset, delamo::List< std::string >& file_names);

	/**
	 * \brief Finds the closest points and normal at this points for the input layers to use with SIMULIA Abaqus FEA
	 *
	 * The function initializes the point and normal list pointer array, but leaves the memory deallocation to the the user.
	 *
	 * \note
	 * This function has a typemap which converts the special objects to native Python lists. The function and the return signature change to
	 *	>> point_list, normal_list, name_list = acis.find_closest_points([list of layers], [x, y, z]);
	 *
	 * \param[in] layer_list list of layers
	 * \param[in] layer_list_size size of the layer list
	 * \param[in] point_in reference point
	 * \param[out] point_list list of points
	 * \param[out] normal_list list of normals
	 * \param[out] name_list list of layer body names
	 * \param[out] list_size size of the point and normal lists
	 */
	void find_closest_points(Layer** layer_list, int layer_list_size, delamo::TPoint3<double> point_in, delamo::TPoint3<double>*& point_list, delamo::TPoint3<double>*& normal_list, char**& name_list, int& list_size);

	/**
	 * \brief Finds the closest point and normal at this point for the input layer to use with SIMULIA Abaqus FEA
	 *
	 * \note
	 * This function has a typemap which converts the special objects to native Python lists. The function and the return signature change to
	 *	>> point, normal, name = acis.find_closest_face_to_point(layer_in, [x, y, z]);
	 *
	 * \param[in] layer_in input layer
	 * \param[in] point_in reference point
	 * \param[out] point_out evaluated point
	 * \param[out] normal_out evaluated normal
	 * \param[out] name_out name of the layer body
	 */
	void find_closest_face_to_point(Layer *layer_in, delamo::TPoint3<double> point_in, delamo::TPoint3<double>& point_out, delamo::TPoint3<double>& normal_out, char*& name_out);

	/**
	 * \brief Splits the layer using the points included in the input file
	 *
	 * The function generates a wire using the points in the input file and splits the input layer using the generated wire.
	 *
	 * \param layer_in layer to be split
	 * \param file_name file which contains the points for wire generation
	 */
	void split_layer(Layer *layer_in, const char* file_name);

	/**
	 * \brief Generates a hat stiffener
	 * \param[in] layer_orig layer on the original side
	 * \param[out] stiffener stiffener to be generated
	 * \param[in] file_name file containing the stiffener outline
	 * \param[in] radius blending radius
	 */
	void create_hat_stiffener(Layer *layer_orig, Layer *stiffener, const char* file_name, double radius = 0);

	/**
	 * \brief Converts input 2D parametric positions into 3D positions
	 *
	 * This function reads parametric positions from a CSV file, evaluates using the input layer's corresponding surface and outputs the evaluated 3D positions as a CSV file.
	 * \param[in] csv_in file name of the input CSV file containing the parametric positions
	 * \param[in] ref_layer reference layer for evaluation of parametric positions
	 * \param[in] ref_dir direction of the reference surface in the layer
	 * \param[in] csv_out file name of the output CSV file containing the evaluated 3D positions
	 */
	void parpos_csv_to_pos(const char* csv_in, Layer *ref_layer, Direction ref_dir, const char* csv_out);

	/**
	* \brief Saves the layer as a text STL file
	* \param file_name name of the STL file to be saved
	* \param layer layer to be saved in the file
	*/
	void save_layer_stl(const char* file_name, Layer *layer);

	/**
	* \brief Saves the boundary between layer1 and layer2 as a text STL file. Should be called AFTER adjacent_layers()/bond_layers()
	* \param file_name name of the STL file to be saved
	* \param layer1  First layer 
	* \param layer2  Second layer
	*/
	void save_layer_surface_stl(const char* file_name, Layer *layer1, Layer *layer2);

	/**
	 * \brief Controls CAD kernel's internal debugging facilities
	 *
	 * Not intented to be used in the production environment.
	 * \param[in] flag enable or disable internal debugging facilities
	 */
	void debug_mode(bool flag);

protected:

	/**
	 * \brief Converts a NURBS surface to a face
	 * \param nurbs_surface input NURBS surface
	 * \param face output face
	 */
	void convert_nurbs_to_face(delamo::NURBS<double> *nurbs_surface, FACE*& face);

	/**
	 * \brief Converts a face to a NURBS surface
	 * \param face input face
	 * \param surface output NURBS surface
	 */
	void convert_face_to_nurbs(FACE* face, delamo::NURBS<double> *surface);

	/**
	 * \brief Converts a NURBS surface to a sheet body
	 * \param nurbs_surface input NURBS surface
	 * \param sheet_body output sheet body
	 */
	void convert_nurbs_to_sheet(delamo::NURBS<double> *nurbs_surface, BODY*& sheet_body);

	/**
	 * \brief Converts a face to a sheet body
	 * \param face input face
	 * \param sheet_body output sheet body
	 */
	void convert_face_to_sheet(FACE* face, BODY*& sheet_body);

	/**
	 * \brief Internal function for processing layers during generation
	 * \param layer_in input layer
	 * \param sheet_body_in mold body
	 * \param ldir layer offsetting direction
	 * \param from_stiffener flag to set if this layer is generated from a stiffened layer
	 */
	void process_layer(Layer *layer_in, BODY* sheet_body_in, Direction ldir, bool from_stiffener = false);

	/**
	 * \brief Finds LayerSurface directions (e.g. offset, orig, side) and updates the input LayerSurface object
	 *
	 * This function uses the angle between the reference normal and the eval normal to determine face directions.
	 * Due to its nature, it might fail to find correct directions when the side faces are perpendicular to the faces at the offset and original sides.
	 * Recommended to be used in simple use cases (such as; finding face directions of stiffener objects)
	 * \param ls_in input LayerSurface
	 * \param eval_normal surface evauated normal
	 * \param reference_normal surface reference normal
	 */
	void find_initial_ls_direction_simple(LayerSurface *ls_in, SPAunit_vector& eval_normal, SPAunit_vector& reference_normal);

	/**
	 * \brief Finds LayerSurface directions (e.g. offset, orig, side) and updates the input Layer object
	 *
	 * This function is the advanced version of the face direction finding functions. It uses initial sheet bodies (which are the molds at the original side) as reference.
	 * After determining the layer surfaces at the original side, it translates the reference points to the offset direction to determine layer surfaces at the offset direction.
	 * All other remaning layer surfaces should be on the sides of the input layer.
	 * \param layer_in input layer
	 * \param mold_list list of sheet bodies that may be used to generate the input layer
	 */
	void find_initial_ls_direction(Layer *layer_in, ENTITY_LIST& mold_list);

	/**
	 * \brief Internal function for processing layers for delamination imprinting
	 * \param layer_orig layer on the original side
	 * \param layer_offset layer on the offset side
	 * \param delampts points defining the delamination outline
	 * \param delampts_size number of points stored in the "delampts" pointer
	 */
	void process_delamination(Layer *layer_orig, Layer *layer_offset, delamo::TPoint3<double>* delampts, int delampts_size);

	/**
	 * \brief Imprints delamination and updates LayerSurface object
	 * \param[in] layersurface_in input LayerSurface
	 * \param[in] surf_direction surface direction (offset or original)
	 * \param[in] delampts delamination edge points
	 * \param[in] num_delampts number of delamination edge points
	 * \param[out] lsc_new lisr of newly generated LayerSurface objects
	 */
	void imprint_delamination(LayerSurface* layersurface_in, Direction surf_direction, BODY* outer_profile, BODY* inner_profile, BODY* inner_profile_pp, delamo::List<LayerSurface *>& lsc_new);

	/* Creates the inner delamination outline (contact zone) as a face in parametric space for boundary condition evaluations
	 *
	 * IMPORTANT: This function is a part of boundary condition evaluation evaluation functionality and creates the output face in parametric space.
	 *
	 * /param[in] delam_wirebody wire body of the inner delamination profile in parametric space
	 * /param[out] delam_face_inner wire covered face representing the inner delamination profile in parametric space
	 */
	void generate_delam_ref_face(BODY* delam_wirebody, FACE*& delam_face_inner);

	/**
	 * \brief Finds boundary conditions on the delaminated FACE
	 * \param[in] face_in face to be evaluated
	 * \param[in] transf_in face transformation (should be the same as body transformation)
	 * \param[in] ref_point arbitrary reference point for testing
	 * \param[in] ref_cohesive_zone cohesive zone reference
	 * \param[in] ref_contact_zone contact zone reference
	 * \param[out] delam_type delamination type
	 */
	void find_delam_bc(FACE* face_in, SPAtransf& transf_in, SPAposition& ref_point, SPAposition& ref_cohesive_zone, FACE* ref_contact_zone, DelaminationType& delam_type);

	/**
	* \brief Generates inner and outer delamination profiles
	* \param[in] ref_face reference face for generating profiles
	* \param[in] delampts list of delamination edge points
	* \param[in] delampts_size size of the delamination edge points list
	* \param[out] outer_profile outer delamination profile in 3D space
	* \param[out] inner_profile inner delamination profile in 3D space
	* \param[out] outer_profile_pp outer delamination profile in parametric space
	* \param[out] inner_profile_pp inner delamination profile in parametric space
	*/
	void generate_delamination_profiles(FACE* ref_face, delamo::TPoint3<double>* delampts, int delampts_size, BODY*& outer_profile, BODY*& inner_profile, BODY*& outer_profile_pp, BODY*& inner_profile_pp);

	/**
	 * \brief Internal function for processing split layers
	 * \param[in/out] layer_in input layer
	 * \param[out] tool_body sheet body to be used to chop/split the layer
	 */
	void process_split(Layer *layer_in, BODY* tool_body);

	/**
	 * \brief Generates molds on the original and offset direction of the input layer
	 * \param layer_in input layer
	 */
	void generate_mold(Layer *layer_in);

	/**
	 * \brief Generates mold for the stiffener
	 * \param lamina Layer on the original direction
	 * \param stiffener Stiffener on the offset direction
	 */
	void generate_stiffened_mold(Layer *lamina, Layer *stiffener);

	/**
	 * \brief Creates a solid body using thickening operation
	 * \param sheet_body input mold
	 * \param thickness layer thickness
	 * \return generated layer
	 */
	BODY* thicken_block(BODY* sheet_body, double thickness);

	/**
	 * \brief Updates surface pairs in the input layers
	 * \param layer1 input layer 1
	 * \param layer2 input layer 2
	 */
	void update_surface_pairs(Layer *layer1, Layer *layer2);

	/**
	 * \brief Checks whether ACIS is running or not
	 */
	void is_builder_started();


	/**
	 * \brief Generates the CAD model file
	 *
	 * \param file_name name of the file which contains the CAD model
	 * \param layer_list list of the layers to be saved in the file
	 */
	void save_cad_model(const char* file_name, delamo::List<Layer *>& layer_list);


	/**
	* \brief Generates the CAD model file
	*
	* \param file_name name of the file which contains the CAD model
	* \param layer_list list of the layers to be saved in the file
	* \param mbbody_list list of additional shell bodies to be saved in the file
	*/
	void save_cad_model(const char* file_name, delamo::List<Layer*>& layer_list, delamo::List<MBBody*>& mbbody_list);

	/**
	 * \brief Loads the CAD model from a file
	 *
	 * \param file_name name of the file which contains the CAD model
	 * \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	 * \param lm_list list of the layer molds loaded from the file
	 */
	void load_cad_model(const char* file_name, bool text_mode, delamo::List<LayerMold*>& lm_list);

	/**
	* \brief Cuts out the mold from a shell sat file
	*
	* \param file_name name of the file which contains the CAD model. The tool is colored red.
	* \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	* \param lm_list list of the layer molds loaded from the file along with cut out shell
	*/
	void create_shell_cutout_sat(const char* file_name, bool text_mode, delamo::List<LayerMold*>& lm_mold);

	/**
	* \brief Load the shell model from a file and extract the point list of the edges
	*
	* \param file_name name of the file which contains the CAD model
	* \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	* \param point list of the midpoints of the edges of the cut out region
	*/
	void load_shell_sat_model(const char* file_name, bool text_mode, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list);

	/**
	* \brief Load the shell model from a layer mold list
	*
	* \param lm layer mold of a shell model with cutout
	* \param point list of the midpoints of the edges of the cut out region
	*/
	void load_shell_sat_model(LayerMold *lm, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list);

	/**
	* \brief Translate the points on the edges of the shell model alomg the normals to the middle of ths side faces
	*
	* \param edge_point_list points on the edges of the shell model
	* \param edge_normal_list normals on the edges of the shell model
	* \param thickness is the distance to translate
	* \param layer_point_list is the translated point list
	*/
	void translate_shell_edge_points(delamo::List<delamo::TPoint3<double>>& edge_point_list, delamo::List<delamo::TPoint3<double>>& edge_normal_list, double thickness, delamo::List<delamo::TPoint3<double>>& layer_point_list);

	/**
	* \brief Finds the side faces corresponding to the input points list on the given layer and outputs the points and normals
	*
	* \param layer_in input layer
	* \param edge_point_list points on the sides of the layer
	* \param point_out output points on the sides of the layer
	* \param normal_out output normal on the sides of the layer
	*/
	void find_side_faces(Layer *layer_in, delamo::List<delamo::TPoint3<double>>& side_point_list, delamo::List<delamo::TPoint3<double>>& point_out, delamo::List<delamo::TPoint3<double>>& normal_out);

	/**
	 * \brief Extracts the transformation data from ACIS BODY objects
	 * It is required to find FACE transformations since ACIS FACE objects do not have any transformation option.
	 * According to the ACIS documentation, all FACE objects belonging to a BODY object should have the same transformation as the BODY.
	 *
	 * \param[in] body_in input BODY object
	 * \param[out] transf_out transformation data of the input BODY object
	 */
	void get_body_transf(BODY* body_in, SPAtransf& transf_out);

	/**
	 * \brief Finds the closest side face w.r.t. the given point
	 * \param layer_in input layer
	 * \param point_in input reference point
	 * \return reference to the LayerSurface object containing the closest side face
	 */
	LayerSurface* find_closest_side(Layer *layer_in, delamo::TPoint3<double>& point_in);

private:
	const double _u_pos = 0.05; /**< Initial surface point evaluation location in the parametric domain (u-direction) */
	const double _v_pos = 0.05; /**< Initial surface point evaluation location in the parametric domain (v-direction) */
	double _blending_radius = 0.0; /**< Radius for creating blended faces during stitching (fillet radius for stiffened layers) */

	/**
	 * \brief Checks ACIS API outcome and logs it if there are any errors
	 *
	 * \param api_retval ACIS API call return value
	 * \param file_name file name containing the function which calls the ACIS API
	 * \param line_number line number showing the location of the ACIS API call inside the file
	 * \param function_name name of the function containing the ACIS API call
	 */
	void _check_outcome(const outcome& api_retval, const char* file_name, int line_number, const char* function_name);


	/**
	* \brief Extracts the triangles from a meshed face
	* \param face pointer to a face that is already meshed
	* \param triVerts list of triangle vertices
	* \return number of triangles in the face mesh
	*/
	int GetTrianglesFromFacetedFace(FACE* face, std::vector<SPAposition>* triVerts);

};

#endif /* ACISMODELBUILDER_H */
