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
	//std::string cpfile = "CP_Curved1.txt";
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

	/**
	* REQUIRED VARIABLES
	*/

	// Define thickness
	double thickness = 0.2;
	double thickness1 = 0.18;
	double thickness2 = 0.04;

	// Define file names
	std::string cad_file = "DeLaMo_TC_Cohesive.sat";
	std::string fal_debug_file = "Debug_FAL_Cohesive.txt";

	/**
	* START CAD MODEL BUILDER
	*/

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

	// Number of layers
	const int layers_len = 5;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	/**
	* GENERATE LAYERS
	*/

	// Create 1st layer from the NURBS surface
	acis->create_layer(&mold, Direction::OFFSET, thickness1, &layers[0]);
	layers[0].name("Layer_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(&layers[0], Direction::OFFSET, thickness2, &layers[1]);
	layers[1].name("Layer_12_Cohesive");
	layers[1].layup(90);

	// Create 3rd layer
	acis->create_layer(&layers[1], Direction::OFFSET, thickness1, &layers[2]);
	layers[2].name("Layer_2");
	layers[2].layup(0);

	// Split 2th layer
	//acis->split_layer(layers[1], "SplitLine.csv");
	acis->split_layer(&layers[1], "nasa-delam12-1.csv");

	delamo::List< std::string > delam_list12;
	delam_list12.add("nasa-delam12-1.csv");

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[0], &layers[1], delam_list12, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none,list01, list01_size);

	// Remove the inner delaminated region
	layers[1].remove(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[1], &layers[2], BCStatus::is_contact, list12, list12_size);

	// Create 4th layer
	acis->create_layer(&layers[2], Direction::OFFSET, thickness, &layers[3]);
	layers[2].name("Layer_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[2], &layers[3], BCStatus::is_contact, list23, list23_size);

	// Create 5th layer
	acis->create_layer(&layers[3], Direction::OFFSET, thickness, &layers[4]);
	layers[4].name("Layer_4");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[3], &layers[4], BCStatus::is_contact, list34, list34_size);

	// Truncate the text file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Write FAL info to the text file
	WriteFAL(fal_debug_file.c_str(), list01, list01_size);
	WriteFAL(fal_debug_file.c_str(), list12, list12_size);
	WriteFAL(fal_debug_file.c_str(), list23, list23_size);
	WriteFAL(fal_debug_file.c_str(), list34, list34_size);

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
