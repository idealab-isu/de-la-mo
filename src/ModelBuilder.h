#ifndef MODELBUILDER_H
#define MODELBUILDER_H

#include "APIConfig.h"
#include "Layer.h"
#include "PNFind_ABS.h"
#include "mb_utilities.h"

// Export file is generated by CMake's GenerateExportHeader module
#include "modelbuilder_export.h"

// Demo of how to return a vector of vectors
// (remove me!) 
static inline std::shared_ptr<std::vector<std::shared_ptr<std::vector<delamo::TPoint3<double>>>>> VectorVectorDemo(void)
{
  std::shared_ptr<std::vector<std::shared_ptr<std::vector<delamo::TPoint3<double>>>>> retval = std::make_shared<std::vector<std::shared_ptr<std::vector<delamo::TPoint3<double>>>>>();

  std::shared_ptr<std::vector<delamo::TPoint3<double>>> first_sublist=std::make_shared<std::vector<delamo::TPoint3<double>>>();

  first_sublist->push_back(delamo::TPoint3<double>(1.0,2.0,3.0));
  first_sublist->push_back(delamo::TPoint3<double>(4.0,5.0,6.0));
  retval->push_back(first_sublist);
  
  std::shared_ptr<std::vector<delamo::TPoint3<double>>> second_sublist=std::make_shared<std::vector<delamo::TPoint3<double>>>();
  second_sublist->push_back(delamo::TPoint3<double>(7.0,8.0,9.0));
  retval->push_back(second_sublist);
    
  return retval;
}
/* test Python code for VectorVectorDemo:
import delamo.CADmodeler as cadmb
VV=cadmb.VectorVectorDemo()
len(VV[0])
len(VV[0][0])
VV[0][0][2]  # 3.0
*/

// Defines an abstract class for interacting with the solid modeling kernels/engines
class MODELBUILDER_EXPORT ModelBuilder
{
public:

	/**
	 * \brief Default constructor
	 */
	ModelBuilder();

	/**
	 * \brief Commercial Solid Modeling Kernel constructor
	 * \param license_key Solid Modeling Kernel license key as a string
	 */
	explicit ModelBuilder(const char* license_key);

	/**
	 * \brief Default destructor
	 */
	~ModelBuilder();

	/**
	 * \brief Wrapper function for ModelBuilder::start()
	 */
	void init(); 

	/**
	 * \brief Returns DELTA a.k.a. tolerance value used internally in the C++ API
	 * \return the delta value
	 */
	double tolerance();

	/**
	 * \brief Sets offset distance required to generate a no model zone
	 * \param val offset distance
	 */
	void offset_distance(double val);

	/**
	 * \brief Returns the offset distance
	 * \return offset distance
	 */
	double offset_distance();

	/**
	 * \brief Sets the license key (where necessary)
	 * \param key license key as a string
	 */
	void set_license_key(const char* key);

	/**
	 * \brief Virtual function for initializing the modeler engine
	 */
	virtual void start() = 0;

	/**
	 * \brief Virtual function for stopping the modeler engine
	 */
	virtual void stop() = 0;

	/**
	 * \brief Virtual function for creating a new layer from a NURBS surface
	 * \param[in] nurbs_surface_in input NURBS surface to be used as a mold for the new layer
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out new layer generated from the input NURBS surface
	 */
	virtual void create_layer(delamo::NURBS<double> *nurbs_surface_in, Direction ldir, double thickness, Layer *layer_out) = 0;

	/**
	 * \brief Virtual function for creating a new layer from a NURBS surface in the OFFSET direction
	 * \param[in] nurbs_surface_in input NURBS surface to be used as a mold for the new layer
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out new layer generated from the input NURBS surface
	 */
	virtual void create_layer(delamo::NURBS<double> *nurbs_surface_in, double thickness, Layer *layer_out) = 0;
	
	/**
	 * \brief Virtual function for creating a new layer from the original or offset surface of the provided layer
	 * \param[in] layer_in input layer to be used as a mold
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness the layer thickness
	 * \param[out] layer_out the new layer
	 */
	virtual void create_layer(Layer *layer_in, Direction ldir, double thickness, Layer *layer_out) = 0;

	/**
	 * \brief Virtual function for creating a new layer from a LayerMold object
	 * \param[in] mold_in input mold
	 * \param[in] ldir the layer direction in which the mold will be offset
	 * \param[in] thickness layer thickness
	 * \param[out] layer_out the new layer
	 */
	virtual void create_layer(LayerMold *mold_in, Direction ldir, double thickness, Layer *layer_out) = 0;

	/**
	 * \brief Imprints the the adjacent faces of the input layers to each other
	 * \param layer_orig layer on the ORIG side
	 * \param layer_offset layer on the OFFSET side
	 */
	virtual void adjacent_layers(Layer *layer_orig, Layer *layer_offset) = 0;

	/**
	 * \brief Imprints delamination profile to the input layers
	 * \param[in] layer_orig layer on the ORIG side
	 * \param[in] layer_offset layer on the OFFSET side
	 * \param[in] file_name CSV file containing the outer delamination profile
	 */
	virtual void adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name) = 0;

	/**
	 * \brief Imprints multiple delamination profiles to the input layers
	 * \param[in] layer_orig layer on the ORIG side
	 * \param[in] layer_offset layer on the OFFSET side
	 * \param[in] file_names a list of CSV files containing the outer delamination profile
	 */
	virtual void adjacent_layers(Layer *layer_orig, Layer *layer_offset, delamo::List< std::string > &file_names) = 0;

	/**
	 * \brief Imprints the the adjacent faces of the input layers to each other and generates a face adjacency list
	 * \param[in] layer_orig layer on the ORIG side
	 * \param[in] layer_offset layer on the OFFSET side
	 * \param[out] fal the face adjacency list
	 * \param[out] fal_size size of the face adjacency list
	 * \param[in] status sets the default boundary condition
	 */
	void adjacent_layers(Layer *layer_orig, Layer *layer_offset, BCStatus delam_region_status, FaceAdjacency*& fal, int& fal_size);

	/**
	 * \brief Imprints delamination profile to the input layers and generates a face adjacency list
	 * \param[in] layer_orig layer on the ORIG side
	 * \param[in] layer_offset layer on the OFFSET side
	 * \param[in] file_name CSV file containing the outer delamination profile
	 * \param[out] fal the face adjacency list
	 * \param[out] fal_size size of the face adjacency list
	 * \param[in] status sets the default boundary condition
	 */
	void adjacent_layers(Layer  *layer_orig, Layer  *layer_offset, const char* file_name, BCStatus delam_region_status, FaceAdjacency*& fal, int& fal_size);

	/**
	 * \brief Imprints multiple delamination profiles to the input layers and generates a face adjacency list
	 * \param[in] layer_orig layer on the ORIG side
	 * \param[in] layer_offset layer on the OFFSET side
	 * \param[in] file_names a list of CSV files containing the outer delamination profile
	 * \param[out] fal the face adjacency list
	 * \param[out] fal_size size of the face adjacency list
	 * \param[in] status sets the default boundary condition
	 */
	void adjacent_layers(Layer * layer_orig, Layer *layer_offset, delamo::List< std::string > file_names, BCStatus default_status, BCStatus delam_region_status, BCStatus delam_ring_status,FaceAdjacency*& fal, int& fal_size);

	/**
	 * \brief Saves the CAD model
	 * \param file_name file name to save
	 */
	void save(const char* file_name);

	/**
	 * \brief Saves the CAD model and extracts the saved layer body names
	 * \param[in] file_name name of the CAD model file
	 * \param[out] bodynames list of layer body names to be returned
	 */
	void save(const char* file_name, delamo::List< std::string >& bodynames);

	/**
	 * \brief Saves the CAD model and extracts the saved layer body names
	 * \param[in] file_name name of the CAD model file
	 * \param[out] bodynames list of layer body names to be returned
	 * \param[out] num_bodynames size of the list of layer body names to be returned
	 */
	void save(const char* file_name, char**& bodynames, int& num_bodynames);

	/**
	 * \brief Saves the CAD model and extracts the saved layer body names
	 * \param[in] file_name name of the CAD model file
	 * \param[in] layer_list list of layers to be saved
	 * \param[out] names_list list of layer body names to be returned
	 */
	void save(const char* file_name, delamo::List<Layer*> &layer_list, delamo::List< std::string >& names_list);

	/**
	* \brief Saves the CAD model and extracts the saved layer body names
	* \param[in] file_name name of the CAD model file
	* \param[in] layer_list list of layers to be saved
	* \param[in] mbbody_list list of additional shell bodies to be saved
	* \param[out] names_list list of layer body names to be returned
	*/
	void save(const char* file_name, delamo::List<Layer*>& layer_list, delamo::List<MBBody*>& mbbody_list, delamo::List< std::string >& names_list);

	/**
	 * \brief Loads the CAD model with sheet bodies and converts them into LayerMold objects
	 * \param[in] file_name name of the CAD model file
	 * \param[in] lm_list list of LayerMolds loaded from the file
	 */
	void load_molds(const char* file_name, delamo::List<LayerMold*>& lm_list);

	/**
	* \brief Loads the CAD model with sheet bodies and cut out the mold
	* \param[in] file_name name of the CAD model file
	* \param[in] lm_list list of LayerMolds loaded from the file and contains the mold and cutout shell
	*/
	void create_shell_cutout(const char* file_name, delamo::List<LayerMold*>& lm_list);


	/**
	* \brief Loads the CAD shell model with sheet body and finds mid-points of each edge of the cut-out region
	* \param[in] file_name name of the CAD model file
	* \param[out] point_list list of midpoints of the cut out region
	* \param[out] tangent_list list of tangents at midpoints of the cut out region
	* \param[out] normal_list list of surface normals at the midpoints of the cut out region
	*/
	void load_shell_model(const char* file_name, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list);

	/**
	* \brief Loads the CAD shell model with sheet body and finds mid-points of each edge of the cut-out region
	* \param[in] lm LayerMolds with the shell model
	* \param[out] point_list list of midpoints of the cut out region
	* \param[out] tangent_list list of tangents at midpoints of the cut out region
	* \param[out] normal_list list of surface normals at the midpoints of the cut out region
	*/
	void load_shell_model(LayerMold *lm, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list);

	/**
	 * \brief Finds the closest points and normal at this points for the input layers to use with SIMULIA Abaqus FEA
	 *
	 * The function initializes the point and normal list pointer array, but leaves the memory deallocation to the the user.
	 *
	 * \note
	 * This function has a typemap which converts the special objects to native Python lists. The function and  he return signature change to
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
	virtual void find_closest_points(Layer** layer_list, int layer_list_size, delamo::TPoint3<double> point_in, delamo::TPoint3<double>*& point_list, delamo::TPoint3<double>*& normal_list, char**& name_list, int& list_size) = 0;

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
	virtual void find_closest_face_to_point(Layer *layer_in, delamo::TPoint3<double> point_in, delamo::TPoint3<double>& point_out, delamo::TPoint3<double>& normal_out, char*& name_out) = 0;

	/**
	 * \brief Generates a hat stiffener
	 * \param[in] layer_orig layer on the original side
	 * \param[out] stiffener stiffener to be generated
	 * \param[in] file_name file containing the stiffener outline
	 * \param[in] radius blending radius
	 */
	virtual void create_hat_stiffener(Layer *layer_orig, Layer *stiffener, const char* file_name, double radius = 0) = 0;

	/**
	 * \brief Splits the layer using the points included in the input file
	 *
	 * The function generates a wire using the points in the input file and splits the input layer using the generated wire.
	 *
	 * \param layer_in layer to be split
	 * \param file_name file which contains the points for wire generation
	 */
	virtual void split_layer(Layer *layer_in, const char* file_name) = 0;

	/**
	 * \brief Converts input 2D parametric positions into 3D positions
	 *
	 * This function reads parametric positions from a CSV file, evaluates using the input layer's corresponding surface and outputs the evaluated 3D positions as a CSV file.
	 * \param[in] csv_in file name of the input CSV file containing the parametric positions
	 * \param[in] ref_layer reference layer for evaluation of parametric positions
	 * \param[in] ref_dir direction of the reference surface in the layer
	 * \param[in] csv_out file name of the output CSV file containing the evaluated 3D positions
	 */
	virtual void parpos_csv_to_pos(const char* csv_in, Layer *ref_layer, Direction ref_dir, const char* csv_out) = 0;

	/**
	* \brief Load the shell model from a file and extract the point list of the edges
	* \param file_name name of the file which contains the CAD model
	* \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	* \param point list of the midpoints of the edges of the cut out region
	*/
	virtual void load_shell_sat_model(const char* file_name, bool text_mode, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list) = 0;

	/**
	* \brief Load the shell model from a file and extract the point list of the edges
	* \param lm_list LayerMold with a cut out shell model
	* \param point list of the midpoints of the edges of the cut out region
	*/
	virtual void load_shell_sat_model(LayerMold *lm, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list) = 0;

	/**
	* \brief Translate the points on the edges of the shell model alomg the normals to the middle of ths side faces
	*
	* \param edge_point_list points on the edges of the shell model
	* \param edge_normal_list normals on the edges of the shell model
	* \param thickness is the distance to translate
	* \param layer_point_list is the translated point list
	*/
	virtual void translate_shell_edge_points(delamo::List<delamo::TPoint3<double>>& edge_point_list, delamo::List<delamo::TPoint3<double>>& edge_normal_list, double thickness, delamo::List<delamo::TPoint3<double>>& layer_point_list) = 0;

	/**
	* \brief Finds the side faces corresponding to the input points list on the given layer and outputs the points and normals
	*
	* \param layer_in input layer
	* \param edge_point_list points on the sides of the layer
	* \param point_out output points on the sides of the layer
	* \param normal_out output normal on the sides of the layer
	*/
	virtual void find_side_faces(Layer *layer_in, delamo::List<delamo::TPoint3<double>>& side_point_list, delamo::List<delamo::TPoint3<double>>& point_out, delamo::List<delamo::TPoint3<double>>& normal_out) = 0;

	/**
	* \brief Saves the layer as a text STL file
	* \param file_name name of the STL file to be saved
	* \param layer layer to be saved in the file
	*/
	virtual void save_layer_stl(const char* file_name, Layer *layer) = 0;

	/**
	* \brief Saves the boundary between layer1 and layer2 as a text STL file. Should be called AFTER adjacent_layers()/bond_layers()
	* \param file_name name of the STL file to be saved
	* \param layer1  First layer 
	* \param layer2  Second layer
	*/
	virtual void save_layer_surface_stl(const char* file_name, Layer *layer1, Layer *layer2)=0;

	/**
	 * \brief Controls CAD kernel's internal debugging facilities
	 *
	 * Not intented to be used in the production environment.
	 * \param[in] flag enable or disable internal debugging facilities
	 */
	virtual void debug_mode(bool flag) = 0;

#if MODELBUILDER_DEBUG_LEVEL == MODELBUILDER_DEBUG_DEBUG
	void crash_on_exception(bool coe_status);
	bool crash_on_exception();
#endif

protected:
	Layer* _pInitialLayer; /**< Stores the first layer during the pre-bonding stage (adjacent layers) */
	char* _pUnlockStr; /**< Stores Solid Modeling Kernel license key */
	PNFind_ABS* _pPtNmAlgo; /**< Stores a pointer to the point-normal find algorithm class */
	bool _mDebugMode;
	int _mLayerID;

	int next_layer_id();

	/**
	 * \brief Generates the CAD model file
	 * \param file_name name of the file which contains the CAD model
	 * \param layer_list list of the layers to be saved in the file
	 */
	virtual void save_cad_model(const char* file_name, delamo::List<Layer*>& layer_list) = 0;

	/**
	* \brief Generates the CAD model file
	* \param file_name name of the file which contains the CAD model
	* \param layer_list list of the layers to be saved in the file
	* \param mbbody_list list of additional shell bodies to be saved in the file
	*/
	virtual void save_cad_model(const char* file_name, delamo::List<Layer*>& layer_list, delamo::List<MBBody*>& mbbody_list) = 0;


	/**
	 * \brief Load the CAD model from a file
	 * \param file_name name of the file which contains the CAD model
	 * \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	 * \param lm_mold list of the LayerMolds loaded from the file
	 */
	virtual void load_cad_model(const char* file_name, bool text_mode, delamo::List<LayerMold*>& lm_mold) = 0;

	/**
	* \brief Cuts out the mold from a shell sat file
	* \param file_name name of the file which contains the CAD model. The tool is colored red.
	* \param text_mode defines whether the loaded file is binary file or not (FALSE means binary)
	* \param lm_list list of the layer molds loaded from the file along with cut out shell
	*/
	virtual void create_shell_cutout_sat(const char* file_name, bool text_mode, delamo::List<LayerMold*>& lm_mold) = 0;

	/**
	 * \brief Generates the Face Adjacency List (FAL) for the input layers
	 * \param[in] layer_orig layer (original direction)
	 * \param[in] layer_offset layer (offset direction)
	 * \param[in] status Default boundary condition
	 * \param[out] fal output FAL pointer array
	 * \param[out] fal_size  size of the output FAL pointer array
	 */
	void generate_adjacency_list(Layer *layer_orig, Layer *layer_offset, BCStatus default_status, BCStatus delam_region_status, BCStatus delam_ring_status, FaceAdjacency*& fal, int& fal_size);

	/**
	 * \brief Collects all of the adjacent layers into a container
	 * \param layer_list_out an array containing the layers to be saved
	 */
	void prepare_layers(delamo::List<Layer*>& layer_list_out);

	/**
	 * \brief Virtual function for checking whether the modeler engine is started or not.
	 */
	virtual void is_builder_started() = 0;

	/**
	 * \brief Wrapper for error handling.
	 *
	 * This function is a wrapper for "exit(EXIT_FAILURE)" to allow easy debugging when using Visual Studio.
	 * If MODELBUILDER_DEBUG_LEVEL is set to MODELBUILDER_DEBUG_DEBUG, the program will pause before executing "exit()".
	 */
	void error_handler();

private:
	bool _bCrashOnException; /**< Flag to set Python mode which converts exception throws to assert(0) */
	double _mDelta; /**< Defines the tolerance value */
	double _mOffsetDistance; /**< Defines the delamination offset distance */
};

#endif // !MODELBUILDER_H