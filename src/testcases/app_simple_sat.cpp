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

	// Define thickness
	double thickness = 0.199;

	// Define file names
	std::string cad_file = "DeLaMo_TC_SimpleSAT.sat";
	std::string fal_debug_file = "Debug_FAL_SimpleSAT.txt";

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

	// Number of layers
	const int layers_len = 8;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	// Read a ACIS file and use the loaded entities as molds
	delamo::List<LayerMold*> lmlist;
	acis->load_molds("CurvedMold1.SAT", lmlist);

	// Create 1st layer
	acis->create_layer(lmlist[0], Direction::OFFSET, thickness, &layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	// Create 2nd layer
	acis->create_layer(&layers[0], Direction::OFFSET, thickness,&layers[1]);
	layers[1].name("Lamina_2");
	layers[1].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[0], &layers[1], BCStatus::is_contact, list01, list01_size);
	std::cout << "Bonded layer 1 and 2" << std::endl;

	// Create 3rd layer
	acis->create_layer(&layers[1], Direction::OFFSET, thickness, &layers[2]);
	layers[2].name("Lamina_3");
	layers[2].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[1], &layers[2], BCStatus::is_contact, list12, list12_size);
	std::cout << "Bonded layer 2 and 3" << std::endl;

	// Create 4th layer
	acis->create_layer(&layers[2], Direction::OFFSET, thickness, &layers[3]);
	layers[3].name("Lamina_4");
	layers[3].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[2], &layers[3], BCStatus::is_contact, list23, list23_size);
	std::cout << "Bonded layer 3 and 4" << std::endl;

	// Create 5th layer
	acis->create_layer(&layers[3], Direction::OFFSET, thickness, &layers[4]);
	layers[4].name("Lamina_5");
	layers[4].layup(0);

	//acis->parpos_csv_to_pos("Delamination1.csv", &layers[4], Direction::ORIG, "Delamination1_3D.csv");

	// Imprint layers to each other with Delamination
	acis->adjacent_layers(&layers[3], &layers[4], "Delamination1_3D.csv", BCStatus::is_contact, list34, list34_size);

	//acis->adjacent_layers(&layers[3], &layers[4], BCStatus::is_contact, list34, list34_size);
	std::cout << "Bonded layer 4 and 5" << std::endl;

	// Create 6th layer
	acis->create_layer(&layers[4], Direction::OFFSET, thickness, &layers[5]);
	layers[5].name("Lamina_6");
	layers[5].layup(90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[4], &layers[5], BCStatus::is_contact, list45, list45_size);
	std::cout << "Bonded layer 5 and 6" << std::endl;

	// Create 7th layer
	acis->create_layer(&layers[5], Direction::OFFSET, thickness, &layers[6]);
	layers[6].name("Lamina_7");
	layers[6].layup(0);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[5], &layers[6], BCStatus::is_contact, list56, list56_size);
	std::cout << "Bonded layer 7 and 8" << std::endl;

	// Create 8th layer
	acis->create_layer(&layers[6], Direction::OFFSET, thickness, &layers[7]);
	layers[7].name("Lamina_8");
	layers[7].layup(-90);

	// Imprint layers to each other with FAL
	acis->adjacent_layers(&layers[6], &layers[7], BCStatus::is_contact, list67, list67_size);

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

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer * > layer_list(layers, layers_len);
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
