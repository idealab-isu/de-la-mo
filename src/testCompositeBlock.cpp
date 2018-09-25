#include "Layer.h"
#include "ACISModelBuilder.h"
#include <iomanip>

using namespace delamo;

// Function prototypes
void pause();
void WriteFAL(std::string fileName, FaceAdjacency* fal, int fal_size);

// Test cases
void testcase_simple(const char* unlock_str, NURBS<double>& mold_in);
void testcase_shell_solid(const char* unlock_str);
void testcase_simple_loop(const char* unlock_str, NURBS<double>& mold_in);
void testcase_stiffener(const char* unlock_str, NURBS<double>& mold_in);
void testcase_cohesive(const char* unlock_str, NURBS<double>& mold_in);
void testcase_nasa(const char* unlock_str, NURBS<double>& mold_in);


int main(int argc, char** argv) {

	// Read ACIS license file
	char* unlock_str = nullptr;
	if (!read_license_file("spa_unlock_7806.txt", unlock_str))
	{
		std::cout << "ERROR: Unable to read license file!" << std::endl;
		pause();
		exit(EXIT_FAILURE);
	}

	// Prepare mold as a NURBS surface
	NURBS<double> mold;
	int degree_u = 3;
	int degree_v = 3;
	mold.degree_u(degree_u);
	mold.degree_v(degree_v);

	//std::string cpfile = "CP_Planar1.txt";
	//std::string cpfile = "CP_Planar2.txt";
	//std::string cpfile = "CP_Curved1.txt";
	std::string cpfile = "CP_Curved5.txt";

	mold.read_ctrlpts(cpfile.c_str());

	delamo::List<double> knot_vector_u = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };
	delamo::List<double> knot_vector_v = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };
	//delamo::List<double> knot_vector_u = { 0, 0, 0, 0, 0.5, 1, 1, 1, 1 };
	//delamo::List<double> knot_vector_v = { 0, 0, 0, 0, 0.33, 0.67, 1, 1, 1, 1 };

	mold.knotvector_u(&knot_vector_u[0], (int)knot_vector_u.size());
	mold.knotvector_v(&knot_vector_v[0], (int)knot_vector_v.size());

	//testcase_simple(unlock_str, mold);
	//testcase_simple_loop(unlock_str, mold);
	//testcase_stiffener(unlock_str, mold);
	//testcase_cohesive(unlock_str, mold);
	//testcase_nasa(unlock_str, mold);
	testcase_shell_solid(unlock_str);

	// Clear license key from the memory
	if (unlock_str != nullptr)
	{
		delete[] unlock_str;
		unlock_str = nullptr;
	}

	// Pause execution to see console output
	pause();

	// Return success
	return EXIT_SUCCESS;
}

// Pauses the command line messages
void pause()
{
	std::string dummy;
	std::cout << "Press ENTER to continue..." << std::endl;
	std::getline(std::cin, dummy);
}

// Debug FAL using a text file
void WriteFAL(std::string fileName, FaceAdjacency* fal, int fal_size)
{
	std::ofstream outFile;
	outFile.open(fileName, std::ios::out | std::ios::app);

	for (int i = 0; i < fal_size; i++)
	{
		outFile << std::setprecision(3) << fal[i].point1.x() << " " << fal[i].point1.y() << " " << fal[i].point1.z() << " | ";
		outFile << std::setprecision(3) << fal[i].point2.x() << " " << fal[i].point2.y() << " " << fal[i].point2.z() << " | ";
		outFile << std::setprecision(3) << fal[i].vector1.x() << " " << fal[i].vector1.y() << " " << fal[i].vector1.z() << " | ";
		outFile << std::setprecision(3) << fal[i].vector1.x() << " " << fal[i].vector2.y() << " " << fal[i].vector2.z() << " | ";
		outFile << fal[i].name1 << " ";
		outFile << fal[i].name2 << " ";

		std::string bctype_str;
		switch (fal[i].bcType)
		{
		case DelaminationType::NODELAM: bctype_str = "No Delamination"; break;
		case DelaminationType::NOMODEL: bctype_str = "No Model"; break;
		case DelaminationType::CONTACT: bctype_str = "Contact"; break;
		case DelaminationType::COHESIVE: bctype_str = "Cohesive"; break;
		case DelaminationType::TIE: bctype_str = "Tie"; break;
		  //case DelaminationType::CONTACT_OR_NOMODEL: bctype_str = "Contact or No Model"; break;
		default: bctype_str = "Undefined Type";
		}

		outFile << bctype_str << std::endl;
	}

	// Add separator
	outFile << std::endl;

	outFile.close();
}

void testcase_stiffener(const char* unlock_str, NURBS<double>& mold_in)
{
	/**
	 * REQUIRED VARIABLES
	 */

	// Define thickness
	double thickness = 0.199;

	// Define file names
	std::string cad_file = "DeLaMo_Ex_CPP.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	 * START CAD MODEL BUILDER
	 */

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	//acis->debug_mode(true);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	/**
	 * OFFSETTING STRATEGY
	 */

	// It is possible to use: sweeping, thickening or booleans
	//acis->layer_gen_strategy(LayerGenStrategy::USE_SWEEPING);

	/**
	 * FACE ADJACENCY LISTS (FALs)
	 */

	// Instantiate FALs
	FaceAdjacency* list01 = nullptr;
	int list01_size;
	FaceAdjacency* list12 = nullptr;
	int list12_size;
	FaceAdjacency* list23 = nullptr;
	int list23_size;
	FaceAdjacency* list34 = nullptr;
	int list34_size;
	FaceAdjacency* list45 = nullptr;
	int list45_size;
	FaceAdjacency* list56 = nullptr;
	int list56_size;
	FaceAdjacency* list67 = nullptr;
	int list67_size;
	FaceAdjacency* list78 = nullptr;
	int list78_size;
	FaceAdjacency* list89 = nullptr;
	int list89_size;

	/**
	 * ALLOCATE MEMORY FOR LAYERS
	 */

	// Number of layers
	const int layers_len = 10;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	 * GENERATE LAYERS
	 */

	// Create 1st layer from the NURBS surface
	acis->create_layer(mold_in, Direction::OFFSET, thickness, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(layers[0], Direction::OFFSET, thickness, layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Testing FAL
	acis->adjacent_layers(layers[0], layers[1], BCStatus::is_contact, list01, list01_size);

	// Testing single delamination without FAL
	//acis->adjacent_layers(layers[0], layers[1], "Delamination1.csv");

	// Testing single delamination with FAL
	//acis->adjacent_layers(layers[0], layers[1], "Delamination1_3D.csv", BCStatus::is_contact, list01, list01_size);
	//acis->adjacent_layers(layers[0], layers[1], "Delamination1ShiftedX.csv", BCStatus::is_contact, list01, list01_size);

	// Testing multiple delaminations with FAL
	//delamo::List< std::string > delam_list;
	//delam_list.add("delam12-1.csv");
	//delam_list.add("delam12-2.csv");
	//acis->adjacent_layers(layers[0], layers[1], delam_list, BCStatus::is_contact, list01, list01_size);

	// Create 3rd layer
	acis->create_layer(layers[1], Direction::OFFSET, thickness, layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[1], layers[2], BCStatus::is_contact, list12, list12_size);

	// Create 4th layer
	acis->create_layer(layers[2], Direction::OFFSET, thickness, layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(-90);

	// Split 4th layer
	acis->split_layer(layers[3], "SplitLine.csv");

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[2], layers[3], BCStatus::is_contact, list23, list23_size);

	// Create 5th layer
	acis->create_layer(layers[3], Direction::OFFSET, thickness, layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[3], layers[4], BCStatus::is_contact, list34, list34_size);

	// Create 6th layer
	acis->create_layer(layers[4], Direction::OFFSET, thickness, layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[4], layers[5], BCStatus::is_contact, list45, list45_size);

	// Create stiffener
	Layer stiffener;
	acis->create_hat_stiffener(layers[5], stiffener, "HatStiffener.csv");
	acis->adjacent_layers(layers[5], stiffener);

	// Create 7th layer
	acis->create_layer(layers[5], Direction::OFFSET, thickness, layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	// Add delamination under the stiffener
	acis->adjacent_layers(layers[5], layers[6], "Delamination1_3D.csv", BCStatus::is_contact, list56, list56_size);

	// Create 8th layer
	acis->create_layer(layers[6], Direction::OFFSET, thickness, layers[7]);
	layers[7].name("Lamina_7");
	layers[7].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[6], layers[7], BCStatus::is_contact, list67, list67_size);

	// Create 9th layer
	acis->create_layer(layers[7], Direction::OFFSET, thickness, layers[8]);
	layers[8].name("Lamina_9");
	layers[8].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[7], layers[8], BCStatus::is_contact, list78, list78_size);

	// Create 10th layer
	acis->create_layer(layers[8], Direction::OFFSET, thickness, layers[9]);
	layers[9].name("Lamina_10");
	layers[9].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[8], layers[9], BCStatus::is_contact, list89, list89_size);

	//// Testing bodynames getter functionality
	//char** namelist;
	//int namelist_size;
	//layers[6].bodynames(namelist, namelist_size);
	//for (int i = 0; i < namelist_size; i++)
	//	delete[] namelist[i];
	//delete[] namelist;

	//// Testing copy functionality
	//Layer layer8;
	//layers[7].copy(layer8);

	/**
	 * DEBUGGING FALs
	 */

	// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Write FAL info to the text file
	WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	WriteFAL(fal_debug_file.c_str(), list34, list34_size);
	WriteFAL(fal_debug_file.c_str(), list45, list45_size);
	WriteFAL(fal_debug_file.c_str(), list56, list56_size);
	WriteFAL(fal_debug_file.c_str(), list67, list67_size);
	WriteFAL(fal_debug_file.c_str(), list78, list78_size);
	WriteFAL(fal_debug_file.c_str(), list89, list89_size);

	/**
	 * SAVE CAD FILE
	 */

	// Testing Layer Manager functionality with pointer arrays
	//char** bodynames;
	//int num_bodynames;
	//acis->save(cad_file.c_str(), bodynames, num_bodynames);
	//for (int i = 0; i < num_bodynames; i++)
	//	std::cout << bodynames[i] << std::endl;
	//for (int i = 0; i < num_bodynames; i++)
	//	delete[] bodynames[i];
	//delete[] bodynames;

	// Testing Layer Manager functionality with Container
	//delamo::List<std::string> bodynames_ct;
	//acis->save(cad_file.c_str(), bodynames_ct);
	//for (auto bn : bodynames_ct)
	//	std::cout << bn << std::endl;

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	/**
	 * GET TOLERANCE VALUE
	 */

	//// Get the tolerance / delta value of the ModelBuilder
	//double mb_tol = acis->tolerance();

	/**
	 * BOUNDARY CONDITIONS
	 */

	//// Find closest face to the input point
	//delamo::TPoint3<double> point_in(-30.0, -24.0, 0.1);
	//delamo::TPoint3<double> point_out;
	//delamo::TPoint3<double> normal_out;
	//char* name_out;
	//acis->find_closest_face_to_point(layers[1], point_in, point_out, normal_out, name_out);


	/**
	 * STOP CAD MODEL BUILDER
	 */

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	 * DEALLOCATE MEMORY
	 */

	// Free FAL memory
	delete[] list01;
	delete[] list12;
	delete[] list23;
	delete[] list34;
	delete[] list45;
	delete[] list56;
	delete[] list67;
	delete[] list78;
	delete[] list89;

	//// Free BC data
	//free((char*)name_out);

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}

void testcase_simple(const char* unlock_str, NURBS<double>& mold_in)
{
	/**
	* REQUIRED VARIABLES
	*/

	// Define thickness
	double thickness = 0.199;

	// Define file names
	std::string cad_file = "DeLaMo_Ex_CPP.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	* START CAD MODEL BUILDER
	*/

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// It is possible to use: sweeping, thickening or booleans
	//acis->layer_gen_strategy(LayerGenStrategy::USE_SWEEPING);

	/**
	* FACE ADJACENCY LISTS (FALs)
	*/

	// Instantiate FALs
	FaceAdjacency* list01 = nullptr;
	int list01_size;
	FaceAdjacency* list12 = nullptr;
	int list12_size;
	FaceAdjacency* list23 = nullptr;
	int list23_size;
	FaceAdjacency* list34 = nullptr;
	int list34_size;
	FaceAdjacency* list45 = nullptr;
	int list45_size;
	FaceAdjacency* list56 = nullptr;
	int list56_size;
	FaceAdjacency* list67 = nullptr;
	int list67_size;

	/**
	* ALLOCATE MEMORY FOR LAYERS
	*/

	// Number of layers
	const int layers_len = 8;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	* GENERATE LAYERS
	*/

	//// Read a ACIS file and use the loaded entities as molds
	delamo::List<LayerMold> lmlist;
	////acis->load_molds("MoldTestSpline.SAT", lmlist);
	acis->load_molds("CurvedMold3.SAT", lmlist);
	acis->create_layer(lmlist[0], Direction::OFFSET, thickness, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	//// Create 1st layer from the NURBS surface
	//acis->create_layer(mold_in, Direction::OFFSET, thickness, layers[0]);
	//layers[0].name("Lamina_1");
	//layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(layers[0], Direction::OFFSET, thickness, layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Testing single delamination with FAL
	acis->adjacent_layers(layers[0], layers[1], BCStatus::is_contact, list01, list01_size);

	// Create 3rd layer
	acis->create_layer(layers[1], Direction::OFFSET, thickness, layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[1], layers[2], BCStatus::is_contact, list12, list12_size);

	// Create 4th layer
	acis->create_layer(layers[2], Direction::OFFSET, thickness, layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[2], layers[3], BCStatus::is_contact, list23, list23_size);

	// Create 5th layer
	acis->create_layer(layers[3], Direction::OFFSET, thickness, layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	// Testing single delamination with FAL
	acis->adjacent_layers(layers[3], layers[4], BCStatus::is_contact, list34, list34_size);

	// Create 6th layer
	acis->create_layer(layers[4], Direction::OFFSET, thickness, layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[4], layers[5], BCStatus::is_contact, list45, list45_size);

	// Create 7th layer
	acis->create_layer(layers[5], Direction::OFFSET, thickness, layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	acis->parpos_csv_to_pos("Delamination1.csv", layers[6], Direction::ORIG, "Delamination1_3D.csv");

	// Add delamination under the stiffener
	//acis->adjacent_layers(layers[5], layers[6], "Delamination1_3D.csv", BCStatus::is_contact, list56, list56_size);
	acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	// Create 8th layer
	acis->create_layer(layers[6], Direction::OFFSET, thickness, layers[7]);
	layers[7].name("Lamina_8");
	layers[7].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[6], layers[7], BCStatus::is_contact, list67, list67_size);

	/**
	* DEBUGGING FALs
	*/

	// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Write FAL info to the text file
	WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	WriteFAL(fal_debug_file.c_str(), list34, list34_size);
	WriteFAL(fal_debug_file.c_str(), list45, list45_size);
	WriteFAL(fal_debug_file.c_str(), list56, list56_size);
	WriteFAL(fal_debug_file.c_str(), list67, list67_size);

	/**
	* SAVE CAD FILE
	*/
	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	/**
	* STOP CAD MODEL BUILDER
	*/

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	* DEALLOCATE MEMORY
	*/

	// Free FAL memory
	delete[] list01;
	delete[] list12;
	delete[] list23;
	delete[] list34;
	delete[] list45;
	delete[] list56;
	delete[] list67;

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}

void testcase_simple_loop(const char* unlock_str, NURBS<double>& mold_in)
{
	/**
	* REQUIRED VARIABLES
	*/

	// Define thickness
	double thickness = 4.21/16;

	// Define file names
	std::string cad_file = "DeLaMo_Ex_CPP.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	* START CAD MODEL BUILDER
	*/

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// It is possible to use: sweeping, thickening or booleans
	//acis->layer_gen_strategy(LayerGenStrategy::USE_SWEEPING);

	/**
	* FACE ADJACENCY LISTS (FALs)
	*/

	// Instantiate FALs
	FaceAdjacency* list = nullptr;
	int list_size;

	/**
	* ALLOCATE MEMORY FOR LAYERS
	*/

	// Number of layers
	const int layers_len = 16;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	* GENERATE LAYERS
	*/

	int layup_angles[16] = { 90, 45, 0, -45, -90, -45, 0, 45, 90, 45, 0, -45, -90, -45, 0, 45 };

	// Create 1st layer from the NURBS surface
	acis->create_layer(mold_in, Direction::OFFSET, thickness, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	for (int layer_num = 1; layer_num < layers_len; layer_num++)
	{
		// Create layer
		std::string layer_name = "Lamina_" + std::to_string(layer_num + 1);
		acis->create_layer(layers[layer_num-1], Direction::OFFSET, thickness, layers[layer_num]);
		layers[layer_num].name(layer_name.c_str());
		layers[layer_num].layup(layup_angles[layer_num]);

		// Bond layer
		acis->adjacent_layers(layers[layer_num - 1], layers[layer_num], BCStatus::is_contact, list, list_size);

		delete[] list;

	}


	/**
	* DEBUGGING FALs
	*/

	// Truncate the text file
	//std::ofstream outFile;
	//outFile.open(fal_debug_file.c_str(), std::ios::out);
	//outFile.close();

	//// Write FAL info to the text file
	//WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	//WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	//WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	//WriteFAL(fal_debug_file.c_str(), list34, list34_size);
	//WriteFAL(fal_debug_file.c_str(), list45, list45_size);
	//WriteFAL(fal_debug_file.c_str(), list56, list56_size);
	//WriteFAL(fal_debug_file.c_str(), list67, list67_size);

	/**
	* SAVE CAD FILE
	*/
	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	/**
	* STOP CAD MODEL BUILDER
	*/

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	* DEALLOCATE MEMORY
	*/

	// Free FAL memory
	//delete[] list;

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}

void testcase_shell_solid(const char* unlock_str)
{
	/**
	* REQUIRED VARIABLES
	*/

	// Define thickness
	double thickness = 0.199;

	// Define file names
	std::string cad_file = "Flat_Solid_Model.sat";
	std::string shell_file = "Flat_Solid_Model.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	* START CAD MODEL BUILDER
	*/

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// It is possible to use: sweeping, thickening or booleans
	//acis->layer_gen_strategy(LayerGenStrategy::USE_SWEEPING);

	/**
	* FACE ADJACENCY LISTS (FALs)
	*/

	// Instantiate FALs
	FaceAdjacency* list01 = nullptr;
	int list01_size;
	FaceAdjacency* list12 = nullptr;
	int list12_size;
	FaceAdjacency* list23 = nullptr;
	int list23_size;
	FaceAdjacency* list34 = nullptr;
	int list34_size;
	FaceAdjacency* list45 = nullptr;
	int list45_size;
	FaceAdjacency* list56 = nullptr;
	int list56_size;
	FaceAdjacency* list67 = nullptr;
	int list67_size;

	/**
	* ALLOCATE MEMORY FOR LAYERS
	*/

	// Number of layers
	const int layers_len = 8;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	* GENERATE LAYERS
	*/

	// Read a ACIS file and use the loaded entities as molds
	delamo::List<LayerMold> lmlist;
	acis->load_molds("FlatMold.SAT", lmlist);

	// Read a ACIS file for the shell and get the midpoints of edges in the internal loop
	delamo::List<delamo::TPoint3<double>> pointList;
	delamo::List<delamo::TPoint3<double>> normalList;
	acis->load_shell_model("FlatShell.SAT", pointList, normalList);

	// Create 1st layer from the SAT Mold
	acis->create_layer(lmlist[0], Direction::OFFSET, thickness, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	//// Create 1st layer from the NURBS surface
	//acis->create_layer(mold_in, Direction::OFFSET, thickness, layers[0]);
	//layers[0].name("Lamina_1");
	//layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(layers[0], Direction::OFFSET, thickness, layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[0], layers[1], BCStatus::is_contact, list01, list01_size);

	// Test shell solid coupling with 2 layers


	// Load the shell model



	//// Create 3rd layer
	//acis->create_layer(layers[1], Direction::OFFSET, thickness, layers[2]);
	//layers[2].name("Lamina_3");
	//layers[2].layup(0);

	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[1], layers[2], BCStatus::is_contact, list12, list12_size);

	//// Create 4th layer
	//acis->create_layer(layers[2], Direction::OFFSET, thickness, layers[3]);
	//layers[3].name("Lamina_4");
	//layers[3].layup(-90);

	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[2], layers[3], BCStatus::is_contact, list23, list23_size);

	//// Create 5th layer
	//acis->create_layer(layers[3], Direction::OFFSET, thickness, layers[4]);
	//layers[4].name("Lamina_5");
	//layers[4].layup(0);

	//// Convert parametric position to 3D position
	//// acis->parpos_csv_to_pos("Delamination1.csv", layers[4], Direction::ORIG, "Delamination1_3D.csv");

	//// Testing single delamination with FAL
	//// acis->adjacent_layers(layers[3], layers[4], "Delamination1_3D.csv", BCStatus::is_contact, list34, list34_size);
	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[3], layers[4], BCStatus::is_contact, list34, list34_size);

	//// Create 6th layer
	//acis->create_layer(layers[4], Direction::OFFSET, thickness, layers[5]);
	//layers[5].name("Lamina_6");
	//layers[5].layup(90);

	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list45, list45_size);

	//// Create 7th layer
	//acis->create_layer(layers[5], Direction::OFFSET, thickness, layers[6]);
	//layers[6].name("Lamina_7");
	//layers[6].layup(0);

	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	//// Create 8th layer
	//acis->create_layer(layers[6], Direction::OFFSET, thickness, layers[7]);
	//layers[7].name("Lamina_8");
	//layers[7].layup(-90);

	//// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[6], layers[7], BCStatus::is_contact, list67, list67_size);

	/**
	* DEBUGGING FALs
	*/

	//// Truncate the text file
	//std::ofstream outFile;
	//outFile.open(fal_debug_file.c_str(), std::ios::out);
	//outFile.close();

	//// Write FAL info to the text file
	//WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	//WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	//WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	//WriteFAL(fal_debug_file.c_str(), list34, list34_size);
	//WriteFAL(fal_debug_file.c_str(), list45, list45_size);
	//WriteFAL(fal_debug_file.c_str(), list56, list56_size);
	//WriteFAL(fal_debug_file.c_str(), list67, list67_size);

	/**
	* SAVE CAD FILE
	*/
	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	/**
	* STOP CAD MODEL BUILDER
	*/

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	* DEALLOCATE MEMORY
	*/

	// Free FAL memory
	delete[] list01;
	delete[] list12;
	delete[] list23;
	delete[] list34;
	delete[] list45;
	delete[] list56;
	delete[] list67;

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}

void testcase_cohesive(const char* unlock_str, NURBS<double>& mold_in)
{
	/**
	* REQUIRED VARIABLES
	*/

	// Define thickness
	double thickness = 0.2;
	double thickness1 = 0.18;
	double thickness2 = 0.04;

	// Define file names
	std::string cad_file = "DeLaMo_Ex_CPP.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	* START CAD MODEL BUILDER
	*/

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	/**
	* OFFSETTING STRATEGY
	*/

	// It is possible to use: sweeping, thickening or booleans
	//acis->layer_gen_strategy(LayerGenStrategy::USE_SWEEPING);

	/**
	* FACE ADJACENCY LISTS (FALs)
	*/

	// Instantiate FALs
	FaceAdjacency* list01 = nullptr;
	int list01_size;
	FaceAdjacency* list12 = nullptr;
	int list12_size;
	FaceAdjacency* list23 = nullptr;
	int list23_size;
	FaceAdjacency* list34 = nullptr;
	int list34_size;

	/**
	* ALLOCATE MEMORY FOR LAYERS
	*/

	// Number of layers
	const int layers_len = 5;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	* GENERATE LAYERS
	*/

	// Create 1st layer from the NURBS surface
	acis->create_layer(mold_in, Direction::OFFSET, thickness1, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(layers[0], Direction::OFFSET, thickness2, layers[1]);
	layers[1].name("Lamina_12_Delam");
	layers[1].layup(90);

	// Create 3rd layer
	acis->create_layer(layers[1], Direction::OFFSET, thickness1, layers[2]);
	layers[2].name("Lamina_2");
	layers[2].layup(0);

	// Split 2th layer
	//acis->split_layer(layers[1], "SplitLine.csv");
	acis->split_layer(layers[1], "Delamination4.csv");

	// Remove the inner delaminated region
	layers[1].remove(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[0], layers[1], BCStatus::is_contact, list01, list01_size);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[1], layers[2], BCStatus::is_contact, list12, list12_size);

	// Create 4th layer
	acis->create_layer(layers[2], Direction::OFFSET, thickness, layers[3]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[2], layers[3], BCStatus::is_contact, list23, list23_size);

	// Create 5th layer
	acis->create_layer(layers[3], Direction::OFFSET, thickness, layers[4]);
	layers[4].name("Lamina_4");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[3], layers[4], BCStatus::is_contact, list34, list34_size);


	/**
	* DEBUGGING FALs
	*/

	// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Write FAL info to the text file
	WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	WriteFAL(fal_debug_file.c_str(), list34, list34_size);

	/**
	* SAVE CAD FILE
	*/

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();


	/**
	* STOP CAD MODEL BUILDER
	*/

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	* DEALLOCATE MEMORY
	*/

	// Free FAL memory
	delete[] list01;
	delete[] list12;
	delete[] list23;
	delete[] list34;

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}

void testcase_nasa(const char* unlock_str, NURBS<double>& mold_in)
{
	/**
	 * REQUIRED VARIABLES
	 */

	// Define thickness
	double thickness = 0.25;

	// Define file names
	std::string cad_file = "DeLaMo_Ex_NASA_CPP.sat";
	std::string fal_debug_file = "Debug_FAL.txt";

	/**
	 * START CAD MODEL BUILDER
	 */

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// Set offset distance (for some reason only negative distances work with the NASA delam profiles)
	acis->offset_distance(-0.5);

	/**
	 * FACE ADJACENCY LISTS (FALs)
	 */

	// Instantiate FALs
	FaceAdjacency* list12 = nullptr;
	int list12_size;
	FaceAdjacency* list23 = nullptr;
	int list23_size;
	FaceAdjacency* list34 = nullptr;
	int list34_size;
	FaceAdjacency* list45 = nullptr;
	int list45_size;
	FaceAdjacency* list56 = nullptr;
	int list56_size;
	FaceAdjacency* list67 = nullptr;
	int list67_size;
	FaceAdjacency* list78 = nullptr;
	int list78_size;

	/**
	 * ALLOCATE MEMORY FOR LAYERS
	 */

	// Number of layers
	const int layers_len = 8;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	 * GENERATE LAYERS
	 */

	// Create 1st layer from the NURBS surface
	acis->create_layer(mold_in, Direction::OFFSET, thickness, layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(layers[0], Direction::OFFSET, thickness, layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam12-1.csv", layers[1], Direction::ORIG, "nasa-delam12-1.csv");
	acis->parpos_csv_to_pos("nasa/delam12-2.csv", layers[1], Direction::ORIG, "nasa-delam12-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list12;
	//delam_list12.add("nasa-delam12-1.csv"); // FIX: This profile throws an exception when offset distance >= 0.4
	delam_list12.add("nasa-delam12-2.csv");
	acis->adjacent_layers(layers[0], layers[1], delam_list12, BCStatus::is_contact, list12, list12_size);

	// Create 3rd layer
	acis->create_layer(layers[1], Direction::OFFSET, thickness, layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam23-1.csv", layers[2], Direction::ORIG, "nasa-delam23-1.csv");
	acis->parpos_csv_to_pos("nasa/delam23-2.csv", layers[2], Direction::ORIG, "nasa-delam23-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list23;
	delam_list23.add("nasa-delam23-1.csv");
	delam_list23.add("nasa-delam23-2.csv");
	acis->adjacent_layers(layers[1], layers[2], delam_list23, BCStatus::is_contact, list23, list23_size);

	// Create 4th layer
	acis->create_layer(layers[2], Direction::OFFSET, thickness, layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(90);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam34-1.csv", layers[3], Direction::ORIG, "nasa-delam34-1.csv");
	acis->parpos_csv_to_pos("nasa/delam34-2.csv", layers[3], Direction::ORIG, "nasa-delam34-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list34;
	//delam_list34.add("nasa-delam34-1.csv"); // FIX: This shape throws an exception when offset distance >= 0.5
	delam_list34.add("nasa-delam34-2.csv");
	acis->adjacent_layers(layers[2], layers[3], delam_list34, BCStatus::is_contact, list34, list34_size);

	// Create 5th layer
	acis->create_layer(layers[3], Direction::OFFSET, thickness, layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[3], layers[4], BCStatus::is_contact, list45, list45_size);

	// Create 6th layer
	acis->create_layer(layers[4], Direction::OFFSET, thickness, layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam56-1.csv", layers[5], Direction::ORIG, "nasa-delam56-1.csv");
	acis->parpos_csv_to_pos("nasa/delam56-2.csv", layers[5], Direction::ORIG, "nasa-delam56-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list56;
	//delam_list56.add("nasa-delam56-1.csv"); // FIX: This profile throws an exception when offset distance = 1.0
	delam_list56.add("nasa-delam56-2.csv");
	acis->adjacent_layers(layers[4], layers[5], delam_list56, BCStatus::is_contact, list56, list56_size);

	// Create 7th layer
	acis->create_layer(layers[5], Direction::OFFSET, thickness, layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam67-1.csv", layers[6], Direction::ORIG, "nasa-delam67-1.csv");
	acis->parpos_csv_to_pos("nasa/delam67-2.csv", layers[6], Direction::ORIG, "nasa-delam67-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list67;
	delam_list67.add("nasa-delam67-1.csv");
	delam_list67.add("nasa-delam67-2.csv");
	acis->adjacent_layers(layers[5], layers[6], delam_list67, BCStatus::is_contact, list67, list67_size);

	// No delamination imprinting
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list67, list67_size);

	// Create 8th layer
	acis->create_layer(layers[6], Direction::OFFSET, thickness, layers[7]);
	layers[7].name("Lamina_8");
	layers[7].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(layers[6], layers[7], BCStatus::is_contact, list78, list78_size);

	//// Testing bodynames getter functionality
	//char** namelist;
	//int namelist_size;
	//layers[6].bodynames(namelist, namelist_size);
	//for (int i = 0; i < namelist_size; i++)
	//	delete[] namelist[i];
	//delete[] namelist;

	//// Testing copy functionality
	//Layer layer8;
	//layers[7].copy(layer8);

	/**
	 * DEBUGGING FALs
	 */

	// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Write FAL info to the text file
	WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	WriteFAL(fal_debug_file.c_str(), list34, list34_size);
	WriteFAL(fal_debug_file.c_str(), list45, list45_size);
	WriteFAL(fal_debug_file.c_str(), list56, list56_size);
	WriteFAL(fal_debug_file.c_str(), list67, list67_size);
	WriteFAL(fal_debug_file.c_str(), list78, list78_size);

	/**
	 * SAVE CAD FILE
	 */

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer > layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	/**
	 * STOP CAD MODEL BUILDER
	 */

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

	/**
	 * DEALLOCATE MEMORY
	 */

	// Free FAL memory
	delete[] list12;
	delete[] list23;
	delete[] list34;
	delete[] list45;
	delete[] list56;
	delete[] list67;
	delete[] list78;

	// Delete layers
	delete[] layers;

	// Delete ModelBuilder object
	delete acis;
}
