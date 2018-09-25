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

	std::string cpfile = "CP_Planar2.txt";
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
	double thickness = 0.25;

	// Define file names
	std::string cad_file = "DeLaMo_TC_NASA.sat";
	std::string fal_debug_file = "Debug_FAL_NASA.txt";

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// Set offset distance
	acis->offset_distance(0.8);

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

	// Number of layers
	const int layers_len = 8;

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

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam12-1.csv", &layers[1], Direction::ORIG, "nasa-delam12-1.csv");
	acis->parpos_csv_to_pos("nasa/delam12-2.csv", &layers[1], Direction::ORIG, "nasa-delam12-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list12;
	delam_list12.add("nasa-delam12-1.csv"); // FIX: This profile throws an exception when offset distance >= 0.4
	delam_list12.add("nasa-delam12-2.csv");
	acis->adjacent_layers(&layers[0], &layers[1], delam_list12, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none, list12, list12_size);
	std::cout << "Bonded layer 0 and 1" << std::endl;

	// Create 3rd layer
	acis->create_layer(&layers[1], Direction::OFFSET, thickness, &layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam23-1.csv", &layers[2], Direction::ORIG, "nasa-delam23-1.csv");
	acis->parpos_csv_to_pos("nasa/delam23-2.csv", &layers[2], Direction::ORIG, "nasa-delam23-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list23;
	delam_list23.add("nasa-delam23-1.csv");
	delam_list23.add("nasa-delam23-2.csv");
	acis->adjacent_layers(&layers[1], &layers[2], delam_list23, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none,  list23, list23_size);
	std::cout << "Bonded layer 1 and 2" << std::endl;

	// Create 4th layer
	acis->create_layer(&layers[2], Direction::OFFSET, thickness, &layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(90);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam34-1.csv", &layers[3], Direction::ORIG, "nasa-delam34-1.csv");
	acis->parpos_csv_to_pos("nasa/delam34-2.csv", &layers[3], Direction::ORIG, "nasa-delam34-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list34;
	delam_list34.add("nasa-delam34-1.csv"); // FIX: This shape throws an exception when offset distance >= 0.5
	delam_list34.add("nasa-delam34-2.csv");
	acis->adjacent_layers(&layers[2], &layers[3], delam_list34, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none,  list34, list34_size);
	std::cout << "Bonded layer 2 and 3" << std::endl;

	// Create 5th layer
	acis->create_layer(&layers[3], Direction::OFFSET, thickness, &layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[3], &layers[4],  BCStatus::is_contact,   list45, list45_size);
	std::cout << "Bonded layer 3 and 4" << std::endl;

	// Create 6th layer
	acis->create_layer(&layers[4], Direction::OFFSET, thickness, &layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam56-1.csv", &layers[5], Direction::ORIG, "nasa-delam56-1.csv");
	acis->parpos_csv_to_pos("nasa/delam56-2.csv", &layers[5], Direction::ORIG, "nasa-delam56-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list56;
	delam_list56.add("nasa-delam56-1.csv"); // FIX: This profile throws an exception when offset distance = 1.0
	delam_list56.add("nasa-delam56-2.csv");
	acis->adjacent_layers(&layers[4], &layers[5], delam_list56, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none,  list56, list56_size);
	std::cout << "Bonded layer 4 and 5" << std::endl;

	// Create 7th layer
	acis->create_layer(&layers[5], Direction::OFFSET, thickness, &layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	// Convert 2D parametric positions to 3D positions
	acis->parpos_csv_to_pos("nasa/delam67-1.csv", &layers[6], Direction::ORIG, "nasa-delam67-1.csv");
	acis->parpos_csv_to_pos("nasa/delam67-2.csv", &layers[6], Direction::ORIG, "nasa-delam67-2.csv");

	// Imprint multiple delaminations with FAL
	delamo::List< std::string > delam_list67;
	delam_list67.add("nasa-delam67-1.csv");
	delam_list67.add("nasa-delam67-2.csv");
	acis->adjacent_layers(&layers[5], &layers[6], delam_list67, BCStatus::is_cohesive, BCStatus::is_contact, BCStatus::is_none,  list67, list67_size);
	std::cout << "Bonded layer 5 and 6" << std::endl;

	// No delamination imprinting
	//acis->adjacent_layers(layers[5], layers[6], BCStatus::is_contact, list67, list67_size);

	// Create 8th layer
	acis->create_layer(&layers[6], Direction::OFFSET, thickness, &layers[7]);
	layers[7].name("Lamina_8");
	layers[7].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[6], &layers[7], BCStatus::is_contact,  list78, list78_size);
	std::cout << "Bonded layer 6 and 7" << std::endl;

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

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer *> layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

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
