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

	// Prepare mold as a NURBS surface
	NURBS<double> mold;

	int degree_u = 3;
	int degree_v = 3;

	std::string cpfile = "CP_Planar1.txt";
	delamo::List<double> knot_vector_u = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };
	delamo::List<double> knot_vector_v = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };

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

	// Define thickness
	double thickness = 0.199;

	// Define file names
	std::string cad_file = "DeLaMo_TC_Stiffener.sat";
	std::string fal_debug_file = "Debug_FAL_Stiffener.txt";

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

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

	// Number of layers
	const int layers_len = 10;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	// Create 1st layer from the NURBS surface
	acis->create_layer(&mold, Direction::OFFSET, thickness, &layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(&layers[0], Direction::OFFSET, thickness, &layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Testing FAL
	acis->adjacent_layers(&layers[0], &layers[1], BCStatus::is_contact, list01, list01_size);

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
	acis->create_layer(&layers[1], Direction::OFFSET, thickness, &layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[1], &layers[2], BCStatus::is_contact, list12, list12_size);

	// Create 4th layer
	acis->create_layer(&layers[2], Direction::OFFSET, thickness, &layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(-90);

	// Split 4th layer
	acis->split_layer(&layers[3], "SplitLine.csv");

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[2], &layers[3], BCStatus::is_contact, list23, list23_size);

	// Create 5th layer
	acis->create_layer(&layers[3], Direction::OFFSET, thickness, &layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[3], &layers[4], BCStatus::is_contact, list34, list34_size);

	// Create 6th layer
	acis->create_layer(&layers[4], Direction::OFFSET, thickness, &layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[4], &layers[5], BCStatus::is_contact, list45, list45_size);

	// Create stiffener
	Layer stiffener;
	acis->create_hat_stiffener(&layers[5], &stiffener, "HatStiffener.csv");
	acis->adjacent_layers(&layers[5], &stiffener);

	// Create 7th layer
	acis->create_layer(&layers[5], Direction::OFFSET, thickness, &layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	// Imprint layers to each other with FAL
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list56, list56_size);

	// Add delamination under the stiffener
	acis->adjacent_layers(&layers[5], &layers[6], "Delamination1_3D.csv", BCStatus::is_contact, list56, list56_size);

	// Create 8th layer
	acis->create_layer(&layers[6], Direction::OFFSET, thickness, &layers[7]);
	layers[7].name("Lamina_7");
	layers[7].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[6], &layers[7], BCStatus::is_contact, list67, list67_size);

	// Create 9th layer
	acis->create_layer(&layers[7], Direction::OFFSET, thickness, &layers[8]);
	layers[8].name("Lamina_9");
	layers[8].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[7], &layers[8], BCStatus::is_contact, list78, list78_size);

	// Create 10th layer
	acis->create_layer(&layers[8], Direction::OFFSET, thickness, &layers[9]);
	layers[9].name("Lamina_10");
	layers[9].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[8], &layers[9], BCStatus::is_contact, list89, list89_size);

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

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer *> layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

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

	return EXIT_SUCCESS;
}
