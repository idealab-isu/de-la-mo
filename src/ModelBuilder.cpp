#include "ModelBuilder.h"


ModelBuilder::ModelBuilder()
{
	this->_bCrashOnException = false;
	this->_mDelta = 1e-5;
	this->_pInitialLayer = nullptr;
	this->_pUnlockStr = nullptr;
	this->_pPtNmAlgo = nullptr;
	this->_mLayerID = 0;
	this->offset_distance(1.0);
	this->_mDebugMode = false;
}

ModelBuilder::ModelBuilder(const char* license_key) : ModelBuilder()
{
	// Set license key
	this->set_license_key(license_key);
}

ModelBuilder::~ModelBuilder()
{
	this->_pInitialLayer = nullptr;
	if (this->_pUnlockStr != nullptr)
	{
		delete[] this->_pUnlockStr;
		this->_pUnlockStr = nullptr;
	}
	if (this->_pPtNmAlgo != nullptr)
	{
		delete this->_pPtNmAlgo;
		this->_pPtNmAlgo = nullptr;
	}
}

void ModelBuilder::init() 
{
	// Wrapper function for ModelBuilder::start()
	this->start();
}

double ModelBuilder::tolerance()
{
	return this->_mDelta;
}

void ModelBuilder::error_handler()
{
	// Don't exit directly if debugging
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
	{
		std::string dummy;
		std::cout << "Press ENTER to continue. . ." << std::endl;
		std::getline(std::cin, dummy);
	}

	// If COE mode is activated, only assert zero. Otherwise, throw an exception (runtime error)
	if (this->_bCrashOnException)
		assert(0);
	else
		throw std::runtime_error("CAD Model Builder: Operation failed!");
}

void ModelBuilder::prepare_layers(delamo::List<Layer*>& layer_list_out)
{
	// Add initial layer to the out layer list
	layer_list_out.add(this->_pInitialLayer);

	// Then, check the orig side bonded layers
	Layer* orig_side = this->_pInitialLayer->bond_pair(Direction::ORIG);
	while (orig_side)
	{
		// Add the layer to the out list
		layer_list_out.add(orig_side);
		// Update the variable
		orig_side = orig_side->bond_pair(Direction::ORIG);
	}

	// Now check the offset side bonded layers
	Layer* offset_side = this->_pInitialLayer->bond_pair(Direction::OFFSET);
	while (offset_side)
	{
		// Add the layer to the out list
		layer_list_out.add(offset_side);
		// Update the variable
		offset_side = offset_side->bond_pair(Direction::OFFSET);
	}
}

void ModelBuilder::set_license_key(const char* key)
{
	// Check for empty license key input
	if (key == nullptr)
	{
		std::cout << "ERROR: The license key is empty!" << std::endl;
		this->error_handler();
	}

	// Clear internal ptr before setting it
	if (this->_pUnlockStr != nullptr)
	{
		delete[] this->_pUnlockStr;
		this->_pUnlockStr = nullptr;
	}

	// Convert const char* to a std::string
	std::string keystr(key);
	
	// Copy input key to the class variable
	this->_pUnlockStr = new char[keystr.size()+1];
	std::copy(keystr.c_str(), keystr.c_str() + keystr.size(), this->_pUnlockStr);
	this->_pUnlockStr[keystr.size()] = '\0';
}

void ModelBuilder::generate_adjacency_list(Layer *layer_orig, Layer *layer_offset, BCStatus default_status, BCStatus delam_region_status, BCStatus delam_ring_status, FaceAdjacency*& fal, int& fal_size)
{
	// A temporary container to store FaceAdjacency
	delamo::List<FaceAdjacency> fal_container;

	// Original Layer - LayerBody loop
	for (auto orig_lb : *layer_orig)
	{
		// Original Layer - LayerSurface loop
		for (auto orig_ls : *orig_lb)
		{
			if (Direction::OFFSET == orig_ls->direction())
			{
				// Find orig layer surface's pair
				LayerSurface* offset_ls = orig_ls->pair();
				// If orig layer surface has a pair..
				if (offset_ls != nullptr)
				{
					// Get a pointer to the offset layer body
					LayerBody* offset_lb = offset_ls->owner();

					// Ensure that we have a LAMINA type
					if (LayerType::LAMINA == offset_lb->owner()->type())
					{
						// Create a temporary FAL object
						FaceAdjacency elem;

						// Process orig layer surface
						elem.point1 = orig_ls->point_coords();
						elem.vector1 = orig_ls->normal_coords();
						std::string name1 = std::string(orig_lb->name())/* + "S" + std::to_string(orig_ls.id())*/;
						elem.name1 = new char[name1.size() + 1];
						std::copy(name1.c_str(), name1.c_str() + name1.size(), elem.name1);
						elem.name1[name1.size()] = '\0';
						
						// Process offset layer surface
						elem.point2 = offset_ls->point_coords();
						elem.vector2 = offset_ls->normal_coords();
						std::string name2 = std::string(offset_lb->name())/* + "S" + std::to_string(offset_ls->id())*/;
						elem.name2 = new char[name2.size() + 1];
						std::copy(name2.c_str(), name2.c_str() + name2.size(), elem.name2);
						elem.name2[name2.size()] = '\0';

						// Set delamination type
						/*switch (status)
						{
						case is_contact:
							elem.bcType = orig_ls->delam_type();
							break;
						case is_cohesive:
							elem.bcType = DelaminationType::COHESIVE;
							break;
						case is_tie:
							elem.bcType = DelaminationType::TIE;
							break;
						default:
							if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
								std::cout << "ERROR: Default BC is not set!" << std::endl;
							this->error_handler();
						
							}*/
						/* perform the mapping between what we found geometrically 
						   (orig_ls->delam_type()) and the actual boundary condition, 
						   based on the user's requests */
						if (orig_ls->delam_type()==DelaminationType::COHESIVE) {
						  /* This is the surrounding (non-delaminated zone */
						  if (default_status==is_cohesive) {
						    elem.bcType = DelaminationType::COHESIVE;				      
						  } else if (default_status==is_tie) {
						    elem.bcType = DelaminationType::TIE;
						  } else {
						    std::cout << "Error: Invalid value for default_status\n";
						    this->error_handler();
						  }
						} else if (orig_ls->delam_type()==DelaminationType::NODELAM) {
						  // This is for the side faces... just pass it out
						  elem.bcType = DelaminationType::NODELAM;
						} else if (orig_ls->delam_type()==DelaminationType::NOMODEL) {
						  /* This is the narrow ring just inside the delamination boundary */
						  if (delam_ring_status == is_contact) {
						    elem.bcType = DelaminationType::CONTACT;
						  } else if (delam_ring_status==is_none) {
						    elem.bcType = DelaminationType::NOMODEL;
						  } else if (delam_ring_status==is_cohesive) {
						    elem.bcType = DelaminationType::COHESIVE;
						  } else if (delam_ring_status==is_tie) {
						    elem.bcType = DelaminationType::TIE;
						  } else {
						    std::cout << "Error: Invalid value for delam_ring_status\n";
						    this->error_handler();
						  }
						} else if (orig_ls->delam_type()==DelaminationType::CONTACT) {
						  /* This is the main region of the delamination */
						  if (delam_region_status == is_contact) {
						    elem.bcType = DelaminationType::CONTACT;
						  } else if (delam_region_status==is_none) {
						    elem.bcType = DelaminationType::NOMODEL;
						  } else if (delam_region_status==is_cohesive) {
						    elem.bcType = DelaminationType::COHESIVE;
						  } else if (delam_region_status==is_tie) {
						    elem.bcType = DelaminationType::TIE;
						  } else {
						    std::cout << "Error: Invalid value for delam_region_status\n";
						    this->error_handler();
						  }
						} else {
						  // Shouldn't be possible to generate TIE
						    std::cout << "Error: Invalid value from delam_type()\n";
						    this->error_handler();						  
						}
						
						// Exclude stiffener-paired surfaces
						bool spair_flag_orig = orig_ls->is_stiffener_paired();
						bool spair_flag_offset = offset_ls->is_stiffener_paired();

						// Push it to the vector
						if (spair_flag_orig || spair_flag_offset)
							continue;
						else
							fal_container.push_back(elem);
					}
				}
			}
		}
	}

	// Copy the FAL data to the output variables
	fal_size = fal_container.size();
	fal = new FaceAdjacency[fal_size];
	int counter = 0;
	for (auto fa_elem : fal_container)
	{
		fal[counter] = fa_elem;
		counter++;
	}
}

void ModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, BCStatus delam_region_status, FaceAdjacency*& fal, int& fal_size)
{
	// Imprint layers to each other
	this->adjacent_layers(layer_orig, layer_offset);

	// Generate face adjacency list
	this->generate_adjacency_list(layer_orig, layer_offset, is_cohesive, delam_region_status, is_none, fal, fal_size);
}

void ModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name, BCStatus delam_region_status, FaceAdjacency*& fal, int& fal_size)
{
	// Imprint delamination outline between the input layers
	this->adjacent_layers(layer_orig, layer_offset, file_name);

	// Generate face adjacency list
	this->generate_adjacency_list(layer_orig, layer_offset, is_cohesive, delam_region_status, is_none, fal, fal_size);
}

void ModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, delamo::List<std::string> file_names, BCStatus default_status, BCStatus delam_region_status, BCStatus delam_ring_status,FaceAdjacency*& fal, int& fal_size)
{
	// Imprint multiple delamination outlines between the input layers
	this->adjacent_layers(layer_orig, layer_offset, file_names);

	// Generate face adjacency list
	this->generate_adjacency_list(layer_orig, layer_offset, default_status, delam_region_status, delam_ring_status, fal, fal_size);
}

void ModelBuilder::save(const char* file_name)
{
	// Check for empty file name
	if (file_name == nullptr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: File name cannot be empty!" << std::endl;
		this->error_handler();
	}

	// Prepare the layers list
	delamo::List<Layer*> layer_list;
	this->prepare_layers(layer_list);

	// Save the CAD model
	this->save_cad_model(file_name, layer_list);

}

void ModelBuilder::save(const char* file_name, delamo::List< std::string >& bodynames)
{
	// Check for empty file name
	if (file_name == nullptr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: File name cannot be empty!" << std::endl;
		this->error_handler();
	}

	// Prepare the layers list
	delamo::List<Layer*> layer_list;
	this->prepare_layers(layer_list);

	// Save the CAD model
	this->save_cad_model(file_name, layer_list);

	// Get the layer body names
	for (auto lyr : layer_list)
	{
		LayerBody** bodylist = lyr->list();
		int bodysize = lyr->size();
		for (int i = 0; i < bodysize; i++)
		{
			bodynames.push_back(std::string(bodylist[i]->name()));
		}
	}
}

void ModelBuilder::save(const char* file_name, char**& bodynames, int& num_bodynames)
{
	// Check for empty file name
	if (file_name == nullptr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: File name cannot be empty!" << std::endl;
		this->error_handler();
	}

	// Prepare the layers list
	delamo::List<Layer*> layer_list;
	this->prepare_layers(layer_list);

	// Save the CAD model
	this->save_cad_model(file_name, layer_list);

	// Set the size of the char array
	num_bodynames = layer_list.size();

	// Initialize the char array
	bodynames = new char*[num_bodynames];

	// Get the layer body names
	int cnt = 0;
	for (auto lyr : layer_list)
	{
		LayerBody** bodylist = lyr->list();
		int bodysize = lyr->size();
		for (int i = 0; i < bodysize; i++)
		{
			bodynames[cnt] = strdup(bodylist[i]->name());
			cnt++;
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
				std::cout << "Added " << bodynames[cnt] << " to the save list." << std::endl;
		}
	}
}

void ModelBuilder::save(const char* file_name, delamo::List<Layer*>& layer_list, delamo::List< std::string >& names_list)
{
	// Fill up bodynames container with the saved layer body names
	for (auto lyr : layer_list)
	{
		LayerBody** bodylist = lyr->list();
		int bodysize = lyr->size();
		for (int i = 0; i < bodysize; i++)
		{
			names_list.add(std::string(bodylist[i]->name()));
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
				std::cout << "Added " << bodylist[i]->name() << " to the save list." << std::endl;
		}
	}

	// Save CAD Model
	this->save_cad_model(file_name, layer_list);
}

void ModelBuilder::save(const char* file_name, delamo::List<Layer*>& layer_list, delamo::List<MBBody*>& mbbody_list, delamo::List< std::string >& names_list)
{
	// Fill up bodynames container with the saved layer body names
	for (auto lyr : layer_list)
	{
		LayerBody** bodylist = lyr->list();
		int bodysize = lyr->size();
		for (int i = 0; i < bodysize; i++)
		{
			names_list.add(std::string(bodylist[i]->name()));
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
				std::cout << "Added " << bodylist[i]->name() << " to the save list." << std::endl;
		}
 	}

	int bodyNum = 0;
	for (auto body : mbbody_list)
	{
		if (body->name() != nullptr)
			names_list.add(std::string(body->name()));
		else
			names_list.add(std::string("ShellBody") + std::to_string(bodyNum));
		bodyNum++;
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
			std::cout << "Added " << names_list.at(names_list.size()-1) << " to the save list." << std::endl;
	}

	// Save CAD Model
	this->save_cad_model(file_name, layer_list, mbbody_list);
}

void ModelBuilder::load_molds(const char* file_name, delamo::List<LayerMold*>& lm_list)
{
	// Currently, we only use SAT files
	bool text_mode = true;

	// Load CAD model from the file
	this->load_cad_model(file_name, text_mode, lm_list);
}

void ModelBuilder::create_shell_cutout(const char* file_name, delamo::List<LayerMold*>& lm_list)
{
	// Currently, we only use SAT files
	bool text_mode = true;

	// Load CAD model from the file
	this->create_shell_cutout_sat(file_name, text_mode, lm_list);
}

void ModelBuilder::load_shell_model(const char* file_name, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list)
{
	// Currently, we only use SAT files
	bool text_mode = true;

	// Load CAD model from the file
	this->load_shell_sat_model(file_name, text_mode, point_list, tangent_list, normal_list);
}

void ModelBuilder::load_shell_model(LayerMold *lm, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list)
{
	// Currently, we only use SAT files
	bool text_mode = true;

	// Load CAD model from the file
	this->load_shell_sat_model(lm, point_list, tangent_list, normal_list);
}

void ModelBuilder::offset_distance(double val)
{
	this->_mOffsetDistance = val;
}

double ModelBuilder::offset_distance()
{
	return this->_mOffsetDistance;
}

int ModelBuilder::next_layer_id()
{
	return this->_mLayerID++;
}

#if MODELBUILDER_DEBUG_LEVEL == MODELBUILDER_DEBUG_DEBUG
void ModelBuilder::crash_on_exception(bool coe_status) { this->bCrashOnException = coe_status }
bool ModelBuilder::crash_on_exception() { return this->_bCrashOnException }
#endif
