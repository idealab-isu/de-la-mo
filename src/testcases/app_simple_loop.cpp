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

	// Number of layers
	const int layers_len = 16;

	// Define thickness
	double thickness = 4.21 / layers_len;

	// Define file names
	std::string cad_file = "DeLaMo_TC_SimpleLoop.sat";
	std::string fal_debug_file = "Debug_FAL_SimpleLoop.txt";

	// Use ACIS as the solid modeling kernel
	ModelBuilder* acis = new ACISModelBuilder(unlock_str);

	// Start 3D ACIS Modeler engine and initialize kernel operations
	acis->start();

	// Truncate FAL debug file
	std::ofstream outFile;
	outFile.open(fal_debug_file.c_str(), std::ios::out);
	outFile.close();

	// Instantiate FALs
	FaceAdjacency* list = nullptr;
	int list_size;

	// Instantiate the Layer pointer array
	Layer* layers = new Layer[layers_len];

	int layup_angles[16] = { 90, 45, 0, -45, -90, -45, 0, 45, 90, 45, 0, -45, -90, -45, 0, 45 };

	// Create 1st layer from the NURBS surface
	acis->create_layer(&mold, Direction::OFFSET, thickness, &layers[0]);
	layers[0].name("Lamina_1");
	layers[0].layup(0);

	for (int layer_num = 1; layer_num < layers_len; layer_num++)
	{
		// Create layer
		std::string layer_name = "Lamina_" + std::to_string(layer_num + 1);
		acis->create_layer(&layers[layer_num - 1], Direction::OFFSET, thickness, &layers[layer_num]);
		layers[layer_num].name(layer_name.c_str());
		layers[layer_num].layup(layup_angles[layer_num]);

		// Bond layer
		acis->adjacent_layers(&layers[layer_num - 1], &layers[layer_num], BCStatus::is_contact, list, list_size);

		// Write FAL debug info to the file
		WriteFAL(fal_debug_file.c_str(), list, list_size);

		delete[] list;
	}

	// Testing save functionality (function takes a list of Layers and returns LayerBody names of the saved layers)
	delamo::List< std::string > body_names;
	delamo::List< Layer *> layer_list(layers, layers_len);
	acis->save(cad_file.c_str(), layer_list, body_names);
	layer_list.clear();

	// Stop 3D ACIS Modeler engine and free allocated memory
	acis->stop();

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
