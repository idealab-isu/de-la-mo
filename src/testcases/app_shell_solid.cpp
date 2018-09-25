#include "testcase_includes.h"


int main(int argc, char** argv)
{
	// Read ACIS license file
	char* unlock_str = nullptr;
	if (!read_license_file("license.dat", unlock_str))
	{
		std::cout << "ERROR: Unable to read license file!" << std::endl;
		pause();
		exit(EXIT_FAILURE);
	}

	// Prepare mold
	NURBS<double> mold;

	int degree_u = 3;
	int degree_v = 3;

	std::string cpfile = "CP_Curved4.txt";
	delamo::List<double> knot_vector_u = { 0, 0, 0, 0, 0.5, 1, 1, 1, 1 };
	delamo::List<double> knot_vector_v = { 0, 0, 0, 0, 0.33, 0.67, 1, 1, 1, 1 };

	// Check if we can read the control points file
	if (!mold.read_ctrlpts(cpfile.c_str()))
	{
		pause();
		return EXIT_FAILURE;
	}

	// Knot vectors
	mold.knotvector_u(&knot_vector_u[0], (int)knot_vector_u.size());
	mold.knotvector_v(&knot_vector_v[0], (int)knot_vector_v.size());

	// Degrees
	mold.degree_u(degree_u);
	mold.degree_v(degree_v);

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
	//delamo::List<LayerMold> lmlist;
	//acis->load_molds("FlatMold2.SAT", lmlist);

	delamo::List<LayerMold *> lmlist;
	acis->create_shell_cutout("FlatShellTool.SAT", lmlist);
	
	// Read a ACIS file for the shell and get the midpoints of edges in the internal loop
	delamo::List<delamo::TPoint3<double>> edgePointList;
	delamo::List<delamo::TPoint3<double>> edgeTangentList;
	delamo::List<delamo::TPoint3<double>> edgeNormalList;

	//acis->load_shell_model("FlatShell2.SAT", edgePointList, edgeTangentList, edgeNormalList);
	acis->load_shell_model(lmlist[1], edgePointList, edgeTangentList, edgeNormalList);

	// Save the shell model in a MBBody list to save later
	delamo::List<MBBody*> mbbody_list;
	mbbody_list.add(lmlist[1]);

	// Create 1st layer from the SAT Mold
	acis->create_layer(lmlist[0], Direction::OFFSET, thickness,&layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	//// Create 1st layer from the NURBS surface
	//acis->create_layer(mold_in, Direction::OFFSET, thickness, &layers[0]);
	//layers[0].name("Lamina_1");
	//layers[0].layup(0);
	
	// Translate edge points to the mid point of the current layer
	double layer1_mid_thickness = thickness / 2.0;
	delamo::List<delamo::TPoint3<double>> layer1EdgePointList;
	acis->translate_shell_edge_points(edgePointList, edgeNormalList, layer1_mid_thickness, layer1EdgePointList);

	// Find side faces for shell solid coupling
	delamo::List<delamo::TPoint3<double>> layer1SideFacePointList;
	delamo::List<delamo::TPoint3<double>> layer1SideFaceNormalList;
	acis->find_side_faces(&layers[0], layer1EdgePointList, layer1SideFacePointList, layer1SideFaceNormalList);

	// Create 2nd layer
	acis->create_layer(&layers[0], Direction::OFFSET, thickness, &layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Translate edge points to the mid point of the current layer
	double layer2_mid_thickness = layer1_mid_thickness + thickness;
	delamo::List<delamo::TPoint3<double>> layer2EdgePointList;
	acis->translate_shell_edge_points(edgePointList, edgeNormalList, layer2_mid_thickness, layer2EdgePointList);


	// Find side faces for shell solid coupling
	delamo::List<delamo::TPoint3<double>> layer2SideFacePointList;
	delamo::List<delamo::TPoint3<double>> layer2SideFaceNormalList;
	acis->find_side_faces(&layers[1], layer2EdgePointList, layer2SideFacePointList, layer2SideFaceNormalList);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[0], &layers[1], BCStatus::is_contact, list01, list01_size);

	//// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	//// Write FAL info to the text file
	WritePoints(fal_debug_file.c_str(), edgePointList);
	WritePoints(fal_debug_file.c_str(), layer2SideFacePointList);


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
	delamo::List< Layer *> layer_list(layers, layers_len);
	//acis->save(cad_file.c_str(), layer_list, body_names);
	acis->save(cad_file.c_str(), layer_list, mbbody_list, body_names);

	// Save first layer as STL file
	acis->save_layer_surface_stl("Layer1.STL", &layers[0], &layers[1]);

	// Delete layer list
	layer_list.clear();

	//std::cout << body_names.at(2) << std::endl;
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
