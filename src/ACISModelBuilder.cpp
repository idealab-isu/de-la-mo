#include "ACISModelBuilder.h"


void ACISModelBuilder::start()
{
	//this->_pPtNmAlgo = new PNFind_UVseek();
	this->_pPtNmAlgo = new PNFind_BBox();

	// Disable ACIS Freelisting for memory leak checking
	if (this->_mDebugMode)
	{
		base_configuration base_config;
		base_config.enable_freelists = FALSE;
		base_config.raw_allocator = malloc;
		base_config.raw_destructor = free;
		initialize_base(&base_config);
	}

	// Start ACIS modeler and initialize all core ACIS components
	// @see: Spatial Docs on "Library Initialization and Termination"
	this->_check_outcome(api_start_modeller(0), __FILE__, __LINE__, __FUNCTION__);

#ifdef _MSC_VER
	// Display a message on ACIS licensing
	std::cout << "ACIS Licensing Status: ";

	// Check if the license key is not null
	if (!this->_pUnlockStr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: ACIS requires a license key to run!" << std::endl;
		this->error_handler();
	}

	// Try to unlock ACIS and other related Spatial products (for Windows OSes only)
	spa_unlock_result out = spa_unlock_products(this->_pUnlockStr);

	switch (out.get_state())
	{
	case SPA_UNLOCK_PASS:
		// License is good
		std::cout << "PASS" << std::endl;
		break;
	case SPA_UNLOCK_PASS_WARN:
		// We have a problem with the license
		std::cout << "WARNING" << std::endl;
		std::cout << "ACIS Outcome: " << out.get_message_text() << std::endl;
		break;
	case SPA_UNLOCK_FAIL:
		// License is not good!
		std::cout << "FAIL" << std::endl;
		std::cout << "ACIS Outcome: " << out.get_message_text() << std::endl;
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Check if ACIS license is valid." << std::endl;
		this->error_handler();
		break;
	default:
		// Something unexpected happened!
		std::cout << "UNKNOWN" << std::endl;
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: ACIS Licensing returned an unknown status. Cannot continue. Please check the documentation for details." << std::endl;
		this->error_handler();
	}
#else
	// ACIS does not have any licensing options for Linux and MacOS
#endif

	// Disable ACIS bulletin board and roll back mechanism (because we don't use it)
	this->_check_outcome(api_logging(FALSE), __FILE__, __LINE__, __FUNCTION__);
}

void ACISModelBuilder::stop()
{
	// Attempt to release all memory allocated by ACIS
	// @see: Spatial Docs on "Library Initialization and Termination"
	this->_check_outcome(api_stop_modeller(), __FILE__, __LINE__, __FUNCTION__);

	if (this->_mDebugMode)
		terminate_base();
}

void ACISModelBuilder::debug_mode(bool flag)
{
	// If ACIS has already been started, there is no reason to disable freelisting
	if (!is_modeler_started())
		this->_mDebugMode = flag;
	else
		std::cout << "ERROR: Cannot change memory debugging mode after starting ACIS!" << std::endl;
}

void ACISModelBuilder::is_builder_started()
{
	// Directly call the related ACIS function to obtain modeler status
	int status = is_modeler_started();

	if (!status)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Please start the Model Builder before using it!" << std::endl;
		this->error_handler();
	}
}

void ACISModelBuilder::save_cad_model(const char* file_name, delamo::List<Layer *>& layer_list)
{
	// Check if there are any layers to save
	if (layer_list.size() <= 0)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Cannot save! Nothing to save.   " << __LINE__ << std::endl;
		this->error_handler();
	}

	// Initialize a temporary ACIS entity list for saving the model to a SAT file
	ENTITY_LIST to_be_saved;

	// Collect the generated entities to an ACIS list before saving
	int totalLayerBodies = 0;
	for (auto layer : layer_list)
	{
		LayerBody** bodylist = layer->list();
		int numLayerBodies = layer->size();
		for (int i = 0; i < numLayerBodies; i++)
		{
			to_be_saved.add(bodylist[i]->body());
		}
		totalLayerBodies += numLayerBodies;

	}

	// Create FileInfo Object
	FileInfo info;
	// Set the name in the SAT file
	info.set_product_id(file_name);
	// Set units to millimeters
	info.set_units(1.0);

	// Sets header info to be written to the SAT file
	this->_check_outcome(api_set_file_info(FileUnits | FileIdent, info), __FILE__, __LINE__, __FUNCTION__);

	// Set file version to ACIS R18 for compatibility
	this->_check_outcome(api_save_version(18, 0), __FILE__, __LINE__, __FUNCTION__);

	// Also set the option for sequence numbers (i.e. pointers) in the SAT file for debugging
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
	{
		this->_check_outcome(api_set_int_option("sequence_save_files", 1), __FILE__, __LINE__, __FUNCTION__);
	}

	// Try to create a file handle for writing
	FILE *fp = fopen(file_name, "w");
	if (fp == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open file for writing!" << std::endl;
		this->error_handler();
	}

	// Save the CAD data as a SAT file
	this->_check_outcome(api_save_entity_list(fp, true, to_be_saved), __FILE__, __LINE__, __FUNCTION__);

	// Display the success message
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
	{
		std::cout << "Saved " << totalLayerBodies << " Layer Bodies." << std::endl;
		std::cout << "SUCCESS: Saved SAT file!" << std::endl;
	}

	// Close the file handle
	fclose(fp);
}

int ACISModelBuilder::GetTrianglesFromFacetedFace(FACE* face, std::vector<SPAposition>* triVerts)
{
	// Find the attribute for facets attached to the face. This is the mesh.
	MESH * face_mesh = NULL;
	af_query((ENTITY*)face, IDX_MESH_APP, IDX_MESH_ID, face_mesh);
	INDEXED_MESH * mesh = (INDEXED_MESH*)face_mesh;

	if (mesh && mesh->get_type_of_mesh() != INDEXED_MESH_TYPE)
		mesh = NULL;

	if (mesh == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
			std::cout << "No indexed mesh available for face" << std::endl;
	}

	// Get the number of nodes of the whole face
	int nodeCount = mesh->get_num_vertex();

	// Get number of facets for this face
	int polygonCount = mesh->get_num_polygon();

	// Loop through each facet and copy the data to triangle data structure
	int triangleID = 0;
	for (int polygonIndex = 0; polygonIndex < polygonCount; polygonIndex++)
	{
		// Get the facet's data as an indexed_polygon
		indexed_polygon* poly = mesh->get_polygon(polygonIndex);

		if (poly)
		{
			// Get the number of nodes in this facet
			int nodeCount = poly->num_vertex();
			// Make sure it is a triangle
			if (nodeCount != 3)
			{
				if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
					std::cout << "Face has non-triangualar facets" << std::endl;
			}
			else
			{
				// Print the node indices of this facet
				for (int polyNodeIndex = 0; polyNodeIndex < nodeCount; polyNodeIndex++)
				{
					polygon_vertex* vertex = poly->get_vertex(polyNodeIndex);
					int vertexIndex = mesh->get_vertex_index(vertex);
					SPAposition vertexPos(mesh->get_position(vertexIndex));
					//SPAunit_vector vertexNor(mesh->get_normal(vertexIndex));
					//SPApar_pos uv(mesh->get_uv_as_entered(vertexIndex));
					triVerts->push_back(vertexPos);
				}
			}
		}
	}
	int numTriangles = polygonCount;
	return numTriangles;
}


void ACISModelBuilder::save_layer_stl(const char* file_name, Layer *layer)
{ 
	ENTITY_LIST layerBodies;
	LayerBody** bodylist = layer->list();
	int numLayerBodies = layer->size();
	for (int i = 0; i < numLayerBodies; i++)
	{
		layerBodies.add(bodylist[i]->body());
	}

	int totalNumFaces = 0;
	int totalNumTriangles = 0;

	// Set layer name based on first body name;
	char* name;
	if (numLayerBodies > 0)
		name = bodylist[0]->name();
	
	// Get bounding box to calculate triangulation tolerance
	SPAposition minPt, maxPt;
	SPAboxing_options boxOptions;
	boxOptions.set_mode(SPAboxing_tight);
	this->_check_outcome(api_get_entity_box(layerBodies, minPt, maxPt, &boxOptions, NULL), __FILE__, __LINE__, __FUNCTION__);
	double layerSize = distance_to_point(minPt, maxPt);
	
	// Specify the degree of refinement
	REFINEMENT* objRefinement;
	this->_check_outcome(api_create_refinement(objRefinement), __FILE__, __LINE__, __FUNCTION__);

	objRefinement->set_triang_mode(AF_TRIANG_ALL);
	objRefinement->set_surface_tol(0.02*layerSize);
	//objRefinement->set_normal_tol(10.0);
	objRefinement->set_edge_length(0.02*layerSize);

	facet_options_expert facetExpert;
	facetExpert.set_tolerance_specification_mode(af_surface_tolerance_from_refinement);

	// Specify the information to be recorded at each Vertex
	parameter_token ptoken[2];
	ptoken[0] = POSITION_TOKEN;
	ptoken[1] = NORMAL_TOKEN;
	//ptoken[0] = UV_TOKEN;
	VERTEX_TEMPLATE* vTemplate = new VERTEX_TEMPLATE(2, ptoken);
	this->_check_outcome(api_set_default_vertex_template(vTemplate), __FILE__, __LINE__, __FUNCTION__);

	// Initialize facetted information
	std::vector<SPAposition> triVerts;

	for (int i = 0; i < numLayerBodies; i++)
	{
		BODY* body = (BODY*)layerBodies[i];
		ENTITY_LIST faceEntityList;
		this->_check_outcome(api_get_faces(body, faceEntityList, PAT_CAN_CREATE, NULL), __FILE__, __LINE__, __FUNCTION__);

		faceEntityList.init();
		FACE* face = (FACE*)faceEntityList.first();
		int numFaces = faceEntityList.count();
		totalNumFaces += numFaces;

		// Set the refinement to the bosy
		this->_check_outcome(api_set_entity_refinement(body, objRefinement, TRUE), __FILE__, __LINE__, __FUNCTION__);

		// Loop through all the faces
		for (int faceID = 0; faceID < numFaces; faceID++)
		{
			// Get old mesh manager for the face
			MESH_MANAGER * oldMM;
			this->_check_outcome(api_get_mesh_manager(oldMM), __FILE__, __LINE__, __FUNCTION__);

			// Create a new mesh manager for the face
			INDEXED_MESH_MANAGER* faceMeshManager = ACIS_NEW INDEXED_MESH_MANAGER();

			// Set the new mesh manager for the face
			this->_check_outcome(api_set_mesh_manager(faceMeshManager), __FILE__, __LINE__, __FUNCTION__);

			// Facet the face
			this->_check_outcome(api_facet_entity(face, &facetExpert), __FILE__, __LINE__, __FUNCTION__);
			totalNumTriangles += this->GetTrianglesFromFacetedFace(face, &(triVerts));

			// Delete the mesh manager
			ACIS_DELETE faceMeshManager;
			faceMeshManager = NULL;

			// Restore the old mesh manager
			this->_check_outcome(api_set_mesh_manager(oldMM), __FILE__, __LINE__, __FUNCTION__);

			// Go to the next face
			face = (FACE*)faceEntityList.next();
		}

	}

	// Write to STL file
	std::ofstream stlFile(file_name);
	if (!stlFile.good())
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
			std::cout << "Failed to open file: " << file_name << std::endl;
	}

	stlFile << "object " << name << std::endl;
	for (int triNum = 0; triNum < totalNumTriangles; triNum++)
	{
		SPAvector side1 = triVerts[triNum * 3 + 0] - triVerts[triNum * 3 + 1];
		SPAvector side2 = triVerts[triNum * 3 + 0] - triVerts[triNum * 3 + 2];
		SPAunit_vector faceNormal = normalise(side1*side2);
		
		stlFile << "facet normal " << faceNormal.x() << " " << faceNormal.y() << " " << faceNormal.z() << std::endl;
		stlFile << "outer loop" << std::endl;
		for (int i = 0; i < 3; i++)
			stlFile << "\tvertex " << triVerts[triNum * 3 + i].x() << " " << triVerts[triNum * 3 + i].y() << " " << triVerts[triNum * 3 + i].z() << std::endl;
		stlFile << "endloop" << std::endl;
		stlFile << "endfacet" << std::endl;
	}
	stlFile.close();

	// Clear data
	triVerts.clear();
}

void ACISModelBuilder::save_layer_surface_stl(const char* file_name, Layer *layer1, Layer *layer2)
{
	ENTITY_LIST layerSurfaces;
	LayerBody** bodylist = layer1->list();
	int numLayerBodies = layer1->size();
	int totalNumFaces = 0;
	int totalNumTriangles = 0;
	double min_thickness;
	
	update_surface_pairs(layer1,layer2);

	min_thickness=layer1->thickness();
	if (min_thickness > layer2->thickness()) {
	  min_thickness=layer2->thickness();
	}
	  
	for (auto &lb_in1 : *layer1)
	{
 	        /* search for all the layer surfaces in this boundary of layer 1 */ 
		for (auto &ls_in1 : *lb_in1)
		{
		  for (auto &lb_in2 : *layer2)
		    {
		      /* search for all the layer surfaces in this boundary of layer 2 */ 
		      for (auto &ls_in2 : *lb_in2)
			{
			  if (ls_in1->pair()==ls_in2) {
			    /* surfaces are paired ... add them to our list */
			    layerSurfaces.add(ls_in1->face());
			    totalNumFaces++;
			  }
			}
		    }
		
		}
	}


	// Set layer name based on first body name;
	char* name;
	if (numLayerBodies > 0)
		name = bodylist[0]->name();

	// Get bounding box to calculate triangulation tolerance
	SPAposition minPt, maxPt;

	SPAboxing_options boxOptions;
	boxOptions.set_mode(SPAboxing_tight);
	this->_check_outcome(api_get_entity_box(layerSurfaces, minPt, maxPt, &boxOptions, NULL), __FILE__, __LINE__, __FUNCTION__);
	double layerSize = distance_to_point(minPt, maxPt);

	// Specify the degree of refinement
	REFINEMENT* objRefinement;
	this->_check_outcome(api_create_refinement(objRefinement), __FILE__, __LINE__, __FUNCTION__);

	objRefinement->set_triang_mode(AF_TRIANG_ALL);
	objRefinement->set_surface_tol(0.1*min_thickness);
	objRefinement->set_normal_tol(2);
	objRefinement->set_edge_length(0.1*layerSize);

	facet_options_expert facetExpert;
	facetExpert.set_tolerance_specification_mode(af_surface_tolerance_from_refinement);

	// Specify the information to be recorded at each Vertex
	parameter_token ptoken[2];
	ptoken[0] = POSITION_TOKEN;
	ptoken[1] = NORMAL_TOKEN;
	//ptoken[0] = UV_TOKEN;
	VERTEX_TEMPLATE* vTemplate = new VERTEX_TEMPLATE(2, ptoken);
	this->_check_outcome(api_set_default_vertex_template(vTemplate), __FILE__, __LINE__, __FUNCTION__);

	// Initialize facetted information
	std::vector<SPAposition> triVerts;

	for (int i = 0; i < totalNumFaces; i++)
	{
		FACE* face = (FACE*)layerSurfaces[i];

		// Set the refinement to the bosy
		this->_check_outcome(api_set_entity_refinement(face, objRefinement, TRUE), __FILE__, __LINE__, __FUNCTION__);

		// Get old mesh manager for the face
		MESH_MANAGER * oldMM;
		this->_check_outcome(api_get_mesh_manager(oldMM), __FILE__, __LINE__, __FUNCTION__);

		// Create a new mesh manager for the face
		INDEXED_MESH_MANAGER* faceMeshManager = ACIS_NEW INDEXED_MESH_MANAGER();

		// Set the new mesh manager for the face
		this->_check_outcome(api_set_mesh_manager(faceMeshManager), __FILE__, __LINE__, __FUNCTION__);

		// Facet the face
		this->_check_outcome(api_facet_entity(face, &facetExpert), __FILE__, __LINE__, __FUNCTION__);
		totalNumTriangles += this->GetTrianglesFromFacetedFace(face, &(triVerts));

		// Delete the mesh manager
		ACIS_DELETE faceMeshManager;
		faceMeshManager = NULL;

		// Restore the old mesh manager
		this->_check_outcome(api_set_mesh_manager(oldMM), __FILE__, __LINE__, __FUNCTION__);

	}

	// Write to STL file
	std::ofstream stlFile(file_name);
	if (!stlFile.good())
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
			std::cout << "Failed to open file: " << file_name << std::endl;
	}

	stlFile << "object " << name << std::endl;
	for (int triNum = 0; triNum < totalNumTriangles; triNum++)
	{
		SPAvector side1 = triVerts[triNum * 3 + 0] - triVerts[triNum * 3 + 1];
		SPAvector side2 = triVerts[triNum * 3 + 0] - triVerts[triNum * 3 + 2];
		SPAunit_vector faceNormal = normalise(side1*side2);

		stlFile << "facet normal " << faceNormal.x() << " " << faceNormal.y() << " " << faceNormal.z() << std::endl;
		stlFile << "outer loop" << std::endl;
		for (int i = 0; i < 3; i++)
			stlFile << "\tvertex " << triVerts[triNum * 3 + i].x() << " " << triVerts[triNum * 3 + i].y() << " " << triVerts[triNum * 3 + i].z() << std::endl;
		stlFile << "endloop" << std::endl;
		stlFile << "endfacet" << std::endl;
	}
	stlFile.close();

	// Clear data
	triVerts.clear();
}

void ACISModelBuilder::save_cad_model(const char* file_name, delamo::List<Layer *>& layer_list, delamo::List<MBBody*>& mbbody_list)
{
	// Check if there are any layers to save
	if (layer_list.size() <= 0 && mbbody_list.size() <= 0)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Cannot save! Nothing to save.   " << __LINE__ << std::endl;
		this->error_handler();
	}

	// Initialize a temporary ACIS entity list for saving the model to a SAT file
	ENTITY_LIST to_be_saved;

	// Collect the generated entities to an ACIS list before saving
	int totalLayerBodies = 0;
	for (auto layer : layer_list)
	{
		LayerBody** bodylist = layer->list();
		int numLayerBodies = layer->size();
		for (int i = 0; i < numLayerBodies; i++)
		{
			to_be_saved.add(bodylist[i]->body());
		}
		totalLayerBodies += numLayerBodies;

	}

	int totalMBBodies = 0;
	for (auto mbBody : mbbody_list)
	{
		to_be_saved.add(mbBody->body());
		totalMBBodies++;
	}

	// Create FileInfo Object
	FileInfo info;
	// Set the name in the SAT file
	info.set_product_id(file_name);
	// Set units to millimeters
	info.set_units(1.0);

	// Sets header info to be written to the SAT file
	this->_check_outcome(api_set_file_info(FileUnits | FileIdent, info), __FILE__, __LINE__, __FUNCTION__);

	// Set file version to ACIS R18 for compatibility
	this->_check_outcome(api_save_version(18, 0), __FILE__, __LINE__, __FUNCTION__);

	// Also set the option for sequence numbers (i.e. pointers) in the SAT file for debugging
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
	{
		this->_check_outcome(api_set_int_option("sequence_save_files", 1), __FILE__, __LINE__, __FUNCTION__);
	}

	// Try to create a file handle for writing
	FILE *fp = fopen(file_name, "w");
	if (fp == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open file for writing!" << std::endl;
		this->error_handler();
	}

	// Save the CAD data as a SAT file
	this->_check_outcome(api_save_entity_list(fp, true, to_be_saved), __FILE__, __LINE__, __FUNCTION__);

	// Display the success message
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_INFO)
	{
		std::cout << "Saved " << totalLayerBodies+ totalMBBodies << " bodies." << std::endl;
		std::cout << "SUCCESS: Saved SAT file!" << std::endl;
	}

	// Close the file handle
	fclose(fp);
}


void ACISModelBuilder::load_shell_sat_model(LayerMold  *lm, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list)
{
	ENTITY_LIST face_list;
	this->_check_outcome(api_get_faces(lm->body(), face_list), __FILE__, __LINE__, __FUNCTION__);
	// Extract all the loops from the shell mold
	FACE* face = (FACE*)face_list[0];

	ENTITY_LIST loopList;
	this->_check_outcome(api_get_loops((ENTITY*)face_list[0], loopList, PAT_CAN_CREATE, NULL), __FILE__, __LINE__, __FUNCTION__);

	// Make sure there is only 1 internal loop
	int numLoops = loopList.count();

	if (numLoops > 2)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_WARN)
		{
			std::cout << "WARNING: Multiple internal loops in the shell model!" << std::endl;
		}
	}

	// First edge in the loop is assumed to be the correct internal loop
	// This might not be a correct assumption, need a more rigourous test
	ENTITY* loop = (ENTITY*)loopList.first();

	// Evaluate mid point for each edge
	ENTITY_LIST coedgeList;
	this->_check_outcome(api_get_coedges(loop, coedgeList, PAT_CAN_CREATE, NULL), __FILE__, __LINE__, __FUNCTION__);

	COEDGE* coedge = (COEDGE*)coedgeList.first();
	int numCoedges = coedgeList.count();
	for (int coedgeID = 0; coedgeID < numCoedges; coedgeID++)
	{
		EDGE* edge = coedge->edge();
		SPAposition edgeMidPoint = edge_mid_pos(edge);
		SPAunit_vector edgeMidTangent = edge_mid_dir(edge);

		// Create point and add it to the return list
		delamo::TPoint3<double> mid_point;
		mid_point.set_x(edgeMidPoint.x());
		mid_point.set_y(edgeMidPoint.y());
		mid_point.set_z(edgeMidPoint.z());
		point_list.add(mid_point);

		// Create tangent and add it to the return list
		delamo::TPoint3<double> mid_tangent;
		mid_tangent.set_x(edgeMidTangent.x());
		mid_tangent.set_y(edgeMidTangent.y());
		mid_tangent.set_z(edgeMidTangent.z());
		tangent_list.add(mid_tangent);

		const SPAtransf tempTransformation;
		SPAunit_vector surf_normal = edge_mid_norm(edge, tempTransformation, face);


		// Old code for finding the normal of the surface
		/*
		SPAunit_vector surf_normal;
		if (is_planar_face(face))
		{
		// Find planar normal
		// It is going to be the same normal, so evaluate at 0, 0;

		surf_normal = face->geometry()->equation().eval_normal(SPApar_pos(0.0, 0.0));
		}
		else if (is_spline_face(face))
		{
		// Find parametric position of the midpoint on the underlying trim curve
		pcurve pSplCurve = coedge->geometry()->equation();
		bs2_curve bs2Curve = pSplCurve.cur();

		double t_start = edge->start_param();
		double t_end = edge->end_param();
		double t_mid = (t_start + t_end) / 2.0;

		SPApar_pos mid_parametric_pos = pSplCurve.eval_position(t_mid);
		surf_normal = face->geometry()->equation().eval_normal(mid_parametric_pos);
		}
		*/

		// Add normal to return list
		delamo::TPoint3<double> mid_normal;
		mid_normal.set_x(surf_normal.x());
		mid_normal.set_y(surf_normal.y());
		mid_normal.set_z(surf_normal.z());
		normal_list.add(mid_normal);

		// Next coedge
		coedge = (COEDGE*)coedgeList.next();
	}

}


void ACISModelBuilder::load_shell_sat_model(const char* file_name, bool text_mode, delamo::List<delamo::TPoint3<double>>& point_list, delamo::List<delamo::TPoint3<double>>& tangent_list, delamo::List<delamo::TPoint3<double>>& normal_list)
{
	// Try to create a file handle for reading
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open file " << std::string(file_name) << " for reading!" << std::endl;
		this->error_handler();
	}

	// Load the CAD model using the file handle
	ENTITY_LIST loaded_ents;
	this->_check_outcome(api_restore_entity_list(fp, text_mode, loaded_ents), __FILE__, __LINE__, __FUNCTION__);

	// Close the file handle
	fclose(fp);


	// This function is designed to load a single sheet body (obviously with 1 single face)
	loaded_ents.init();
	BODY* b;
	while (b = (BODY *)loaded_ents.next())
	{
		ENTITY_LIST face_list;
		this->_check_outcome(api_get_faces(b, face_list), __FILE__, __LINE__, __FUNCTION__);

		// All loaded entities should be sheet body objects with one single face
		if (face_list.iteration_count() != 1)
		{
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
				std::cout << "ERROR: Expecting a sheet body in file: " << std::string(file_name) << std::endl;
			this->error_handler();
		}
		else
		{
			// Create the shell and call the function to extract the face loops
 		        LayerMold *shellModel=new LayerMold();
			shellModel->body(b);
			load_shell_sat_model(shellModel, point_list, tangent_list, normal_list);
		}// If face

	}// If Body


}

void ACISModelBuilder::create_shell_cutout_sat(const char* file_name, bool text_mode, delamo::List<LayerMold *>& lm_list)
{
	// Try to create a file handle for reading
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open file " << std::string(file_name) << " for reading!" << std::endl;
		this->error_handler();
	}

	// Load the CAD model using the file handle
	ENTITY_LIST loaded_ents;
	this->_check_outcome(api_restore_entity_list(fp, text_mode, loaded_ents), __FILE__, __LINE__, __FUNCTION__);

	// Close the file handle
	fclose(fp);

	if (loaded_ents.count() != 2)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Expecting 2 sheet bodies in file!" << std::string(file_name) << std::endl;
		this->error_handler();
	}


	// This function is designed to load two sheet bodies one of which is used as cutout
	loaded_ents.init();
	BODY* shell = (BODY*)loaded_ents.next();
	BODY* cuttingTool = (BODY*)loaded_ents.next();

	rgb_color shellColor;
	int inherit = 1, isColored;
	bool shellToolIdentified = true;
	this->_check_outcome(api_rh_get_entity_rgb(shell, shellColor, inherit, isColored), __FILE__, __LINE__, __FUNCTION__);
	if (shellColor.red() > 0.75)
		shellToolIdentified = false;

	if (!shellToolIdentified)
	{
		loaded_ents.init();
		cuttingTool = (BODY*)loaded_ents.next();
		shell = (BODY*)loaded_ents.next();
	}


	// Cut the mold from the shell using the chop operation
	BODY* outsideShell;
	BODY* leftover;
	this->_check_outcome(api_boolean_chop_body(cuttingTool, shell, FALSE, outsideShell, leftover), __FILE__, __LINE__, __FUNCTION__);

	if (leftover != NULL_REF)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Chopping produced unexpected results!" << std::string(file_name) << std::endl;
		this->error_handler();
	}


	// Create the mold
	// Create LayerMold and add it to the return list
	LayerMold *mold=new LayerMold();
	mold->body(shell);
	mold->name("Mold_Region");
	lm_list.add(mold);

	// Create the shell and return
	LayerMold *shellModel=new LayerMold();
	shellModel->body(outsideShell);
	shellModel->name("Shell");
	lm_list.add(shellModel);

}


void ACISModelBuilder::load_cad_model(const char* file_name, bool text_mode, delamo::List<LayerMold *>& lm_list)
{
	// Try to create a file handle for reading
	FILE *fp = fopen(file_name, "r");
	if (fp == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open file " << std::string(file_name) << " for reading!" << std::endl;
		this->error_handler();
	}

	// Load the CAD model using the file handle
	ENTITY_LIST loaded_ents;
	this->_check_outcome(api_restore_entity_list(fp, text_mode, loaded_ents), __FILE__, __LINE__, __FUNCTION__);

	// Close the file handle
	fclose(fp);

	// This function is designed to load a single sheet body (obviously with 1 single face)
	loaded_ents.init();
	BODY* b;
	while (b = (BODY *)loaded_ents.next())
	{
		ENTITY_LIST face_list;
		this->_check_outcome(api_get_faces(b, face_list), __FILE__, __LINE__, __FUNCTION__);

		// All loaded entities should be sheet body objects with one single face
		if (face_list.iteration_count() != 1)
		{
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
				std::cout << "ERROR: Expecting a sheet body in file: " << std::string(file_name) << std::endl;
			this->error_handler();
		}
		else
		{
			// Check for sidedness
			FACE* test_fc = (FACE*)face_list[0];
			if (test_fc->sense())
			{
				if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
					std::cout << "ERROR: All loaded molds must be in forward direction. Check sidedness of the input mold!" << std::endl;
				this->error_handler();
			}
		}

		// Create LayerMold and add it to the return list
		LayerMold *mold=new LayerMold();
		mold->body(b);
		lm_list.add(mold);
	}
}

void ACISModelBuilder::convert_nurbs_to_face(delamo::NURBS<double> *nurbs_surface, FACE*& face)
{
	// Check for sidedness (RHS or LHS) and ensure that we always use RHS
	delamo::TVector3<double> surf_normal;
	nurbs_surface->normal(this->_u_pos, this->_v_pos, surf_normal);
	if (surf_normal.z() < 0)
		nurbs_surface->transpose();

	// Get variables for ACIS API call: api_mk_fa_spl_ctrlpts()
	int num_ctrlpts = nurbs_surface->ctrlpts_len();

	// Arrange control points in the way ACIS likes
	SPAposition* ctrlpts = ACIS_NEW SPAposition[num_ctrlpts];
	int poscnt = 0;

	// ACIS: The v index varies first. That is, a row of v control points for the first u value is found first. Then, the row of v control points for the next u value.
	delamo::TPoint3<double>** nurbs_ctrlpts = nurbs_surface->ctrlpts_2d();
	for (int i = 0; i < nurbs_surface->ctrlpts_u_len(); i++)
	{
		for (int j = 0; j < nurbs_surface->ctrlpts_v_len(); j++)
		{
			ctrlpts[poscnt].set_x(nurbs_ctrlpts[i][j].x());
			ctrlpts[poscnt].set_y(nurbs_ctrlpts[i][j].y());
			ctrlpts[poscnt].set_z(nurbs_ctrlpts[i][j].z());
			poscnt++;
		}
	}

	// Set the remaining variables
	bool rational_u = false;
	bool rational_v = false;
	int form_u = 0;
	int form_v = 0;
	int pole_u = 0;
	int pole_v = 0;
	double point_tol = this->tolerance();
	double knot_tol = this->tolerance();

	// ACIS: Creates a FACE based upon a B-spline surface defined by a sequence of control points and knots
	this->_check_outcome(api_mk_fa_spl_ctrlpts(
		nurbs_surface->degree_u(),
		rational_u,
		form_u,
		pole_u,
		nurbs_surface->ctrlpts_u_len(),
		nurbs_surface->degree_v(),
		rational_v,
		form_v,
		pole_v,
		nurbs_surface->ctrlpts_v_len(),
		ctrlpts,
		nurbs_surface->weights(),
		point_tol,
		nurbs_surface->knotvector_u_len(),
		nurbs_surface->knotvector_u(),
		nurbs_surface->knotvector_v_len(),
		nurbs_surface->knotvector_v(),
		knot_tol,
		face
	), __FILE__, __LINE__, __FUNCTION__);

	// Delete the pointers in the way ACIS likes
	ACIS_DELETE[] ctrlpts;
}

void ACISModelBuilder::convert_face_to_nurbs(FACE* face, delamo::NURBS<double> *nurbs_surface)
{
	// We should be working with spline surfaces
	if (!is_spline_face(face))
	{
		ENTITY* face_spl;
		this->_check_outcome(api_convert_to_spline(face, face_spl), __FILE__, __LINE__, __FUNCTION__);
		// Update the old face
		face = (FACE*)face_spl;
	}

	// Get ACIS B-Spline surface
	spline* splsurf = (spline*)face->geometry()->trans_surface(get_owner_transf(face), face->sense());

	// Prepare for bs3_surface_to_array, which is actually reverse of api_mk_fa_spl_ctrlpts() in setFace method.
	int dim, form_u, form_v, pole_u, pole_v, num_u, num_v, degree_u, num_uknots, degree_v, num_vknots;
	logical rational_u, rational_v;
	SPAposition* ctrlpts = NULL;
	double* weights = nullptr;
	double* uknots = nullptr;
	double* vknots = nullptr;

	// ACIS: Creates arrays of control points, weights, u knots, and v knots from a B-spline surface
	bs3_surface_to_array(
		splsurf->sur(),
		dim,
		rational_u,
		rational_v,
		form_u,
		form_v,
		pole_u,
		pole_v,
		num_u,
		num_v,
		ctrlpts,
		weights,
		degree_u,
		num_uknots,
		uknots,
		degree_v,
		num_vknots,
		vknots
	);

	// Set the variables of NURBS surface
	nurbs_surface->degree_u(degree_u);
	nurbs_surface->degree_v(degree_v);
	nurbs_surface->knotvector_u(uknots, num_uknots);
	nurbs_surface->knotvector_v(vknots, num_vknots);

	// Prepare and set control points
	int num_ctrlpts = num_u * num_v;
	delamo::TPoint3<double>* ctrlpts_nurbs = new delamo::TPoint3<double>[num_ctrlpts];
	for (int i = 0; i < num_ctrlpts; i++)
	{
		delamo::TPoint3<double> point;
		point.x(ctrlpts[i].x());
		point.y(ctrlpts[i].y());
		point.z(ctrlpts[i].z());
		ctrlpts_nurbs[i] = point;
	}

	// Copy control points
	nurbs_surface->ctrlpts(ctrlpts_nurbs, num_u, num_v);

	// Prepare and set weights
	if (weights != nullptr)
	{
		// If weights pointer is not empty, fill the vector with these values
		nurbs_surface->weights(weights, num_u * num_v);
	}
	else
	{
		// If weights pointer is empty (meaning rational_u and rational_v are zero),
		// fill the vector with a constant value
		double* weights_nurbs = new double[num_ctrlpts];
		double default_value = 1.0;
		for (int i = 0; i < num_ctrlpts; i++)
		{
			weights_nurbs[i] = default_value;
		}
		nurbs_surface->weights(weights_nurbs, num_u * num_v);
		delete[] weights_nurbs;
		weights_nurbs = nullptr;
	}

	//// Finally, reverse U and V orientation of the final NURBS surface
	//nurbs_surface.transpose();

	// Delete pointers
	delete[] ctrlpts_nurbs;
	ctrlpts_nurbs = nullptr;
	ACIS_DELETE[] weights;
	ACIS_DELETE[] uknots;
	ACIS_DELETE[] vknots;
	ACIS_DELETE[] ctrlpts;
}

void ACISModelBuilder::convert_face_to_sheet(FACE* face, BODY*& sheet_body)
{
	// Copy the input face first
	ENTITY* face_copy;
	this->_check_outcome(api_deep_down_copy_entity(face, face_copy), __FILE__, __LINE__, __FUNCTION__);

	// Create a sheet body from the input face
	FACE* face_list[1];
	face_list[0] = (FACE*)face_copy;
	this->_check_outcome(api_sheet_from_ff(1, face_list, sheet_body), __FILE__, __LINE__, __FUNCTION__);

	// Convert to 1D
	this->_check_outcome(api_body_to_1d(sheet_body, true), __FILE__, __LINE__, __FUNCTION__);
}

void ACISModelBuilder::convert_nurbs_to_sheet(delamo::NURBS<double> *nurbs_surface, BODY*& sheet_body)
{
	// First, convert NURBS surface to face
	FACE* face_temp;
	this->convert_nurbs_to_face(nurbs_surface, face_temp);

	// Second, convert face to sheet body
	this->convert_face_to_sheet(face_temp, sheet_body);
}

BODY* ACISModelBuilder::thicken_block(BODY* sheet_body, double thickness)
{
	// Get sheet body's bounding box
	SPAposition box_low;
	SPAposition box_high;
	this->_check_outcome(api_get_entity_box(sheet_body, box_low, box_high), __FILE__, __LINE__, __FUNCTION__);

	// Thicken the sheet. Negative thickness means reverse direction.
	lop_options plopopts;
	logical approx_ok = FALSE;
	this->_check_outcome(api_sheet_thicken(sheet_body, thickness, false, box_low, box_high, &plopopts, approx_ok), __FILE__, __LINE__, __FUNCTION__);

	// We require spline convertion of the faces in order to evaluate parametric positions
	convert_to_spline_options conv_opts;
	conv_opts.set_do_edges(TRUE);
	conv_opts.set_do_faces(TRUE);
	conv_opts.set_in_place(TRUE);
	this->_check_outcome(api_convert_to_spline(sheet_body, &conv_opts, NULL), __FILE__, __LINE__, __FUNCTION__);

	// This is not necessary, but do it anyways
	return sheet_body;
}

void ACISModelBuilder::process_layer(Layer *layer_in, BODY* sheet_body_in, Direction ldir, bool from_stiffener)
{
	// Create a copy of input sheet body to be used as a reference to finding face directions
	ENTITY* sheet_body_ent;
	this->_check_outcome(api_deep_copy_entity(sheet_body_in, sheet_body_ent), __FILE__, __LINE__, __FUNCTION__);
	ENTITY_LIST mold_list;
	mold_list.add(sheet_body_ent);

	// Generate a LayerBody object
	LayerBody *layer_body=new LayerBody();
	// Set layer body name
	std::string body_name = "LB" + std::to_string(layer_in->next_lb_id);
	layer_body->name(body_name.c_str());
	// Set owner
	layer_body->owner(layer_in);

	// Assuming that we generate layers in the OFFSET direction by default
	SPAunit_vector reference_normal(0.0, 0.0, 1.0);

	// Always consider the direction
	double layer_thickness = layer_in->thickness();
	if (ldir == Direction::ORIG)
	{
		layer_thickness = -1 * layer_thickness;
		reference_normal = -reference_normal; // Reference normal should also be reversed in case of orig thickening/sweeping
	}

	// Generate 3D solid body
	BODY* current_body;
	current_body = this->thicken_block(sheet_body_in, layer_thickness);

	// Now we have a closed 3D solid body in "current_body" variable with all faces being a complete face (not split ones).
	layer_body->body(current_body);

	// Generate an identity transform for point discovery function
	SPAtransf current_body_transf;
	this->get_body_transf(current_body, current_body_transf);

	// Find faces, reference points and normals of the 3D solid body
	ENTITY_LIST facelist;
	this->_check_outcome(api_get_faces(current_body, facelist), __FILE__, __LINE__, __FUNCTION__);

	// Move the cursor to the beginning of the entity list
	facelist.init();

	// Traverse through all faces
	FACE* current_face;
	while (current_face = (FACE*)facelist.next())
	{
		// Find the underlying surface
		surface* current_surf = current_face->geometry()->trans_surface(get_owner_transf(current_face), current_face->sense());
		// Set an evaluation location
		SPApar_pos eval_par(this->_u_pos, this->_v_pos);

		// Find point using surface evaluation method (doesn't work well for complicated layers)
		//SPAunit_vector eval_normal = current_surf->eval_normal(eval_par);
		//SPAposition eval_pos = current_surf->eval_position(eval_par);

		// Find point using the internal point finding algorithm
		SPAposition ref_pos = current_surf->eval_position(eval_par);
		SPAunit_vector eval_normal;
		SPAposition eval_pos;
		SPAbox face_box = get_face_box(current_face);
		this->_pPtNmAlgo->find_point(current_face, current_body_transf, face_box, ref_pos, eval_pos, true);
		this->_pPtNmAlgo->find_normal(current_face, eval_pos, eval_normal);

		// Calculate the angle between the reference normal and the face normal
		double vec_angle = radians_to_degrees(angle_between(eval_normal, reference_normal));

		// Update the layer surface
		LayerSurface *current_layersurface = new LayerSurface();
		current_layersurface->id(layer_body->next_ls_id);
		current_layersurface->face(current_face);
		current_layersurface->point(eval_pos);
		current_layersurface->normal(eval_normal);
		current_layersurface->angle(vec_angle);
		// This layer surface is generated at the initial create_layer() stage
		current_layersurface->initial_surface();

		//this->find_initial_ls_direction(current_layersurface, eval_normal, reference_normal);

		// Flag surfaces generated from the stiffened mold
		if (from_stiffener)
		{
			if (Direction::SIDE != current_layersurface->direction())
				current_layersurface->stiffener_gen(true);
		}

		// Set owner of the surface
		current_layersurface->owner(layer_body);

		// Add the generated layer surface object to the layer body
		layer_body->add_surface(current_layersurface);
	}

	// Add layer body to the layer
	layer_in->add_body(layer_body);

	// Find LayerSurface directions
	this->find_initial_ls_direction(layer_in, mold_list);
}

void ACISModelBuilder::generate_mold(Layer *layer_in)
{
	// Loop through all layer bodies to generate mold
	for (auto& layerbody : *layer_in)
	{
		delamo::List<LayerSurface*> surf_offset;
		delamo::List<LayerSurface*> surf_orig;

		for (auto ls : *layerbody)
		{
			switch (ls->direction())
			{
			case Direction::OFFSET:
				surf_offset.add(ls);
				break;
			case Direction::ORIG:
				surf_orig.add(ls);
				break;
			}
		}

		// Get the number of offset and orig layer surfaces
		int offset_faces_len = surf_offset.size();
		int orig_faces_len = surf_orig.size();

		// Check for any possible errors
		if (orig_faces_len == 0 || offset_faces_len == 0)
		{
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
				std::cout << "ERROR: Cannot generate mold. Check your input!" << std::endl;
			this->error_handler();
		}

		/*
		 * Offset Side Mold Generation
		 */

		BODY* mold_offset_body;
		BODY* mold_offset;
		if (offset_faces_len == 1)
		{
			FACE* mold_offset_face = surf_offset[0]->face();
			this->convert_face_to_sheet(mold_offset_face, mold_offset_body);
		}
		else
		{
			ENTITY_LIST to_be_stitched;

			for (int i = 0; i < offset_faces_len; i++)
			{
				FACE* temp_face = surf_offset[i]->face();
				BODY* sheet_body;
				this->convert_face_to_sheet(temp_face, sheet_body);
				to_be_stitched.add(sheet_body);
			}

			ENTITY_LIST output_bodies_offset;

			// We can use multi-threaded stitching too. First, we need to make an input from the list of faces.
			stitch_input_handle* sih;
			this->_check_outcome(api_stitch_make_input(to_be_stitched, sih), __FILE__, __LINE__, __FUNCTION__);

			// Stitch the faces and get the output
			stitch_output_handle* soh;
			this->_check_outcome(api_stitch(sih, soh, output_bodies_offset), __FILE__, __LINE__, __FUNCTION__);

			// So, the stiching operation was successful, and we would be able to generate a surface from the stitched body
			output_bodies_offset.init();
			// The "output_bodies" list should have one single stitched body
			mold_offset_body = (BODY*)output_bodies_offset.first();
		}

		mold_offset = mold_offset_body;

		// Store the offset mold inside the layer
		LayerMold *layer_mold_offset=new LayerMold();
		layer_mold_offset->body(mold_offset);
		layer_mold_offset->direction(Direction::OFFSET);
		layer_mold_offset->owner(layer_in);
		layer_in->add_mold(layer_mold_offset);

		// Store a reference for the offset mold for delamination imprinting if and only if there exists no reference mold
		LayerMold* delam_profile_ref_mold = layer_in->delam_profile_ref();
		if (delam_profile_ref_mold == nullptr)
			layer_in->delam_profile_ref(layer_mold_offset);

		/*
		 * Orig Side Mold Generation
		 */

		BODY* mold_orig_body;
		BODY* mold_orig;
		if (orig_faces_len == 1)
		{
			FACE* mold_orig_face = surf_orig[0]->face();
			this->convert_face_to_sheet(mold_orig_face, mold_orig_body);
		}
		else
		{
			ENTITY_LIST to_be_stitched;

			for (int i = 0; i < orig_faces_len; i++)
			{
				FACE* temp_face = surf_orig[i]->face();
				BODY* sheet_body;
				this->convert_face_to_sheet(temp_face, sheet_body);
				to_be_stitched.add(sheet_body);
			}

			ENTITY_LIST output_bodies_orig;

			// We can use multi-threaded stitching too. First, we need to make an input from the list of faces.
			stitch_input_handle* sih;
			this->_check_outcome(api_stitch_make_input(to_be_stitched, sih), __FILE__, __LINE__, __FUNCTION__);

			// Stitch the faces and get the output
			stitch_output_handle* soh;
			this->_check_outcome(api_stitch(sih, soh, output_bodies_orig), __FILE__, __LINE__, __FUNCTION__);

			// So, the stiching operation was successful, and we would be able to generate a surface from the stitched body
			output_bodies_orig.init();
			// The "output_bodies" list should have one single stitched body
			mold_orig_body = (BODY*)output_bodies_orig.first();
		}

		// No need to fix orig mold when sweeping
		mold_orig = mold_orig_body;

		// Store the orig mold inside the layer
		LayerMold *layer_mold_orig=new LayerMold();
		layer_mold_orig->body(mold_orig);
		layer_mold_orig->direction(Direction::ORIG);
		layer_mold_orig->owner(layer_in);
		layer_in->add_mold(layer_mold_orig);

		// Add orig mold to the LayerBody object
		layerbody->mold(layer_mold_orig);
	}
}

void ACISModelBuilder::create_layer(delamo::NURBS<double> *nurbs_in, Direction ldir, double thickness, Layer *layer_out)
{
	// Check whether ACIS is running or not
	this->is_builder_started();

	// Check whether thickness is positive or not
	if (thickness <= 0)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Please provide a positive thickness!" << std::endl;
		this->error_handler();
	}

	// Set layer type
	layer_out->type(LayerType::LAMINA);

	// Set layer generation direction
	layer_out->direction(ldir);

	// Set thickness of the layer (NURBS layer is always the first one)
	layer_out->position(0.0, thickness);

	// Create the mold sheet body using the NURBS surface
	BODY* sheet_body;
	this->convert_nurbs_to_sheet(nurbs_in, sheet_body);

	// Create the layer
	this->process_layer(layer_out, sheet_body, ldir, "LB0");

	// Set a unique layer ID using the BODY element of the first LayerBody
	//tag_id_type layer_id;
	//this->_outcome = api_get_entity_id(layer_out.list()->body(), layer_id);
	//this->get_api_log(__FILE__, __FUNCTION__, __LINE__);
	//layer_out.id(layer_id);
	layer_out->id(this->next_layer_id());

	//// Save this layer for testing
	//delamo::List<Layer> test_list;
	//test_list.add(layer_out);
	//this->save_cad_model("Test.SAT", test_list);

	// Generate molds
	this->generate_mold(layer_out);

	// Update layer surface owners
	layer_out->update_owners();
}

void ACISModelBuilder::create_layer(delamo::NURBS<double> *nurbs_surface_in, double thickness, Layer *layer_out)
{
	this->create_layer(nurbs_surface_in, Direction::OFFSET, thickness, layer_out);
}

void ACISModelBuilder::create_layer(Layer *layer_in, Direction ldir, double thickness, Layer *layer_out)
{
	// Check whether ACIS is running or not
	this->is_builder_started();

	// Check whether thickness is positive or not
	if (thickness <= 0)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Please provide a positive thickness!" << std::endl;
		this->error_handler();
	}

	// Check the input layer has a body or not
	if (0 == layer_in->size())
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Input layer is empty!" << std::endl;
		this->error_handler();
	}

	// Set layer type
	layer_out->type(LayerType::LAMINA);

	// Set layer generation direction
	layer_out->direction(ldir);

	// Initial position depends on the input direction
	double pos_at_orig;
	if (ldir == Direction::OFFSET)
		pos_at_orig = layer_in->position_offset();
	else
		pos_at_orig = layer_in->position_orig();

	// Set thickness of the layer
	layer_out->position(pos_at_orig, pos_at_orig + thickness);

	// To set correct name for new layer bodies
	int body_cnt = 0;

	LayerMold** lm_list = layer_in->list_mold();
	int lm_list_size = layer_in->size_mold();

	// A custom range-based "for loop" for layer molds
	for (int i =0; i < lm_list_size; i++)
	{
		// Use mold in the chosen direction
		if (ldir == lm_list[i]->direction())
		{
			// Get the mold
			BODY* sheet_body = lm_list[i]->body();

			/*
			* VERY IMPORTANT POINT TO CONSIDER:
			* ACIS Sweeping API requires creating a completely new FACE with no BODY connections or bounds.
			* This is due to the design of api_sweep_* API calls.
			* When an unbounded new FACE is used, Sweeping API creates the BODY and makes this FACE as a part of this BODY entity.
			* When a bounded existing FACE is used, Sweeping API updates the BODY which the FACE is bounded/connected to.
			* Because we need a new BODY, we **must** use a completely new FACE created from the NURBS surface.
			*/
			ENTITY* sheet_body_copy;
			this->_check_outcome(api_deep_down_copy_entity((ENTITY*)sheet_body, sheet_body_copy), __FILE__, __LINE__, __FUNCTION__);

			// Create the layer
			this->process_layer(layer_out, (BODY*)sheet_body_copy,  ldir, lm_list[i]->is_stiffener_gen());

			// Increment the layer body counter
			body_cnt++;
		}
	}

	// Set a unique layer ID using the BODY element of the first LayerBody
	layer_out->id(this->next_layer_id());

	// Generate molds
	this->generate_mold(layer_out);

	// Update layer surface owners
	layer_out->update_owners();
}

void ACISModelBuilder::create_layer(LayerMold *mold_in, Direction ldir, double thickness, Layer *layer_out)
{
	// Check whether ACIS is running or not
	this->is_builder_started();

	// Check whether thickness is positive or not
	if (thickness <= 0)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Please provide a positive thickness!" << std::endl;
		this->error_handler();
	}

	// Retrieve sheet body from input LayerMold
	BODY* sheet_body = mold_in->body();
	if (sheet_body == NULL)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Input mold is empty!" << std::endl;
		this->error_handler();
	}

	// Set layer type
	layer_out->type(LayerType::LAMINA);

	// Set layer generation direction
	layer_out->direction(ldir);

	// Find the z position of the sheet body
	ENTITY_LIST face_list;
	this->_check_outcome(api_get_faces(sheet_body, face_list), __FILE__, __LINE__, __FUNCTION__);
	FACE* sheet_face = (FACE *)face_list[0];

	surface* sheet_surf = sheet_face->geometry()->trans_surface(get_owner_transf(sheet_face), sheet_face->sense());
	SPApar_pos sheet_par_pos(0.0, 0.0);
	SPAposition sheet_pos = sheet_surf->eval_position(sheet_par_pos);
	layer_out->position(sheet_pos.z(), sheet_pos.z() + thickness);

	// Create the BODY object and find faces & points
	this->process_layer(layer_out, sheet_body, ldir, "LB0");

	// Set a unique layer ID using the BODY element of the first LayerBody
	layer_out->id(this->next_layer_id());

	// Generate molds
	this->generate_mold(layer_out);

	// Update layer surface owners
	layer_out->update_owners();
}

void ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset)
{
	// Find pairs before applying any imprint operations
	this->update_surface_pairs(layer_offset, layer_orig);

	// Imprint layers to each other
	for (auto& lb_orig : *layer_orig)
	{
		for (auto& lb_offset : *layer_offset)
			this->_check_outcome(api_imprint(lb_orig->body(), lb_offset->body()), __FILE__, __LINE__, __FUNCTION__);
	}

	// Use an arbitrary reference position
	SPAposition reference_pos(0, 0, 0);

	// Use a reference normal according to layer generation direction
	SPAunit_vector reference_normal(0, 0, 1);
	if (Direction::ORIG == layer_orig->direction())
		reference_normal.set_z(-1.0);

	// Work on the Layer on the ORIG side
	for (auto& lb : *layer_orig)
	{
		// Get the BODY object from LayerBody
		BODY* current_body = lb->body();

		// Find all faces in the BODY
		ENTITY_LIST facelist;
		this->_check_outcome(api_get_faces(current_body, facelist), __FILE__, __LINE__, __FUNCTION__);

		// Only update the LayerSurface list if we have new faces
		if (facelist.iteration_count() > lb->size())
		{
			// Generate an identity transform for point discovery function
			SPAtransf current_body_transf;
			// If the body has a transform, we can use it too
			TRANSFORM* cb_transfrm = current_body->transform();
			if (cb_transfrm != NULL)
				current_body_transf = cb_transfrm->transform();

			// Traverse through all faces of the current body
			FACE* current_face;
			while (current_face = (FACE*)facelist.next())
			{
				// Check if this face exists in our list
				int face_idx = lb->face_id(current_face);

				// Find a reference point and a normal
				SPAunit_vector eval_normal;
				SPAposition eval_pos;
				this->_pPtNmAlgo->find_point_normal(current_face, current_body_transf, reference_pos, eval_pos, eval_normal, true);

				// If face exists, then update. Otherwise, add the new face to the LayerSurface list
				if (face_idx >= 0)
				{
					LayerSurface* face_ls = lb->at(face_idx);
					// Should process OFFSET side of the ORIG face
					if (Direction::OFFSET == face_ls->direction())
					{
						// Update existing LayerSurface
						face_ls->initial_surface(false);
						face_ls->point(eval_pos);
						face_ls->normal(eval_normal);
					}
				}
				else
				{
					// Calculate the angle between the reference normal and the face normal
					double vec_angle = radians_to_degrees(angle_between(eval_normal, reference_normal));

					// Update the layer surface
					LayerSurface *current_layersurface=new LayerSurface();
					current_layersurface->id(lb->next_ls_id);
					current_layersurface->face(current_face);
					current_layersurface->point(eval_pos);
					current_layersurface->normal(eval_normal);
					current_layersurface->angle(vec_angle);

					//this->find_initial_ls_direction(current_layersurface, eval_normal, reference_normal);

					// All new faces should be on the OFFSET side for orig layer
					current_layersurface->direction(Direction::OFFSET);
					
					// Set owner of the surface
					current_layersurface->owner(lb);

					// Add the generated layer surface object to the layer body
					lb->add_surface(current_layersurface);
				}
			}
		}

		// Clear face list
		facelist.clear();
	}

	// Work on the Layer on the OFFSET side
	if (Direction::ORIG == layer_offset->direction())
		reference_normal.set_z(-1.0);
	else
		reference_normal.set_z(1.0);

	for (auto& lb : *layer_offset)
	{
		// Get the BODY object from LayerBody
		BODY* current_body = lb->body();

		// Find all faces in the BODY
		ENTITY_LIST facelist;
		this->_check_outcome(api_get_faces(current_body, facelist), __FILE__, __LINE__, __FUNCTION__);

		// Only update the LayerSurface list if we have new faces
		if (facelist.iteration_count() > lb->size())
		{
			// Generate an identity transform for point discovery function
			SPAtransf current_body_transf;
			// If the body has a transform, we can use it too
			TRANSFORM* cb_transfrm = current_body->transform();
			if (cb_transfrm != NULL)
				current_body_transf = cb_transfrm->transform();

			// Traverse through all faces of the current body
			FACE* current_face;
			while (current_face = (FACE*)facelist.next())
			{
				// Check if this face exists in our list
				int face_idx = lb->face_id(current_face);

				// Find a reference point and a normal
				SPAunit_vector eval_normal;
				SPAposition eval_pos;
				this->_pPtNmAlgo->find_point_normal(current_face, current_body_transf, reference_pos, eval_pos, eval_normal, true);

				// If face exists, then update. Otherwise, add the new face to the LayerSurface list
				if (face_idx >= 0)
				{
					LayerSurface* face_ls = lb->at(face_idx);
					// Should process ORIG side of the OFFSET face
					if (Direction::ORIG == face_ls->direction())
					{
						// Update existing LayerSurface
						face_ls->initial_surface(false);
						face_ls->point(eval_pos);
						face_ls->normal(eval_normal);
					}
				}
				else
				{
					// Calculate the angle between the reference normal and the face normal
					double vec_angle = radians_to_degrees(angle_between(eval_normal, reference_normal));

					// Update the layer surface
					LayerSurface *current_layersurface=new LayerSurface();
					current_layersurface->id(lb->next_ls_id);
					current_layersurface->face(current_face);
					current_layersurface->point(eval_pos);
					current_layersurface->normal(eval_normal);
					current_layersurface->angle(vec_angle);

					//this->find_initial_ls_direction(current_layersurface, eval_normal, reference_normal);

					// All new faces should be on the ORIG side for offset layer
					current_layersurface->direction(Direction::ORIG);

					// Set owner of the surface
					current_layersurface->owner(lb);

					// Add the generated layer surface object to the layer body
					lb->add_surface(current_layersurface);
				}
			}
		}

		// Clear face list
		facelist.clear();
	}

	// Update mold of the orig layer if the offset layer is a stiffener
	if (layer_offset->type() == LayerType::STIFFENER)
	{
		this->generate_stiffened_mold(layer_orig, layer_offset);
	}

	// Update layer surface owners after imprinting operation
	layer_orig->update_owners();
	layer_offset->update_owners();

	// New layer surfaces generated by imprinting will be paired here
	this->update_surface_pairs(layer_offset, layer_orig);

	// Set adjacent pairs
	layer_offset->bond_pair(Direction::ORIG, layer_orig);
	layer_orig->bond_pair(Direction::OFFSET, layer_offset);

	if (this->_pInitialLayer == nullptr)
		this->_pInitialLayer = layer_orig;
}

void ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer *layer_offset, const char* file_name)
{
	// Read delamination points
	delamo::TPoint3<double>* delampts = nullptr;
	int delampts_size;
	read_csv_file(file_name, delampts, delampts_size);

	// Check if we were able to load some points from the file
	if (delampts == nullptr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: There is a problem processing delamination profile. Please check your input file: " << std::string(file_name) << std::endl;
		this->error_handler();
	}
	else
	{
		// Check if the last and the first delamination profile points are equal
		if (delampts[0] != delampts[delampts_size - 1])
		{
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
				std::cout << "ERROR: The first and the last delamination profile points must be equal. Skipping delamination imprint..." << std::endl;

			// Do layer imprinting without delamination
			this->adjacent_layers(layer_orig, layer_offset);
			return;
		}
	}

	// Update surface pairs before processing delamination
	this->update_surface_pairs(layer_offset, layer_orig);

	// Process delamination
	this->process_delamination(layer_orig, layer_offset, delampts, delampts_size);

	// Delete delamination points
	delete[] delampts;
	delampts = nullptr;

	// Update layer surface owners after imprinting operation
	layer_orig->update_owners();
	layer_offset->update_owners();

	// New layer surfaces generated by imprinting will be paired here
	this->update_surface_pairs(layer_offset, layer_orig);

	// Set adjacent pairs
	layer_offset->bond_pair(Direction::ORIG, layer_orig);
	layer_orig->bond_pair(Direction::OFFSET, layer_offset);

	if (this->_pInitialLayer == nullptr)
		this->_pInitialLayer = layer_orig;
}

void ACISModelBuilder::split_layer(Layer *layer_in, const char* file_name)
{
	// Check that the input layer has only 1 layer body
	if (layer_in->size() != 1)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Input layer contains multiple bodies. Cannot split!" << std::endl;
		// This is not a fatal error, no need to call error handler.
		return;
	}

	// Check that the input layer has 1 orig and 1 offset surface
	delamo::List<LayerSurface *> orig_surfaces;
	delamo::List<LayerSurface *> offset_surfaces;
	for (auto lb : *layer_in)
	{
		for (auto ls : *lb)
		{
			switch (ls->direction())
			{
			case Direction::OFFSET: offset_surfaces.add(ls); break;
			case Direction::ORIG: orig_surfaces.add(ls); break;
			}
		}
	}

	// Choose the surface that we are going to project the split path
	if (orig_surfaces.size() > 1 && offset_surfaces.size() > 1)
	{
		// There must be something wrong with the input layer
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Cannot split input layer. A possible reason is that the input layer might have already been altered by another damage-incorporation operation." << std::endl;
		this->error_handler();
	}

	double start_z = layer_in->position_orig();
	double end_z = layer_in->position_offset();

	// Read the split path from a CSV file
	delamo::TPoint3<double>* splitpts;
	int splitpts_size;
	read_csv_file(file_name, splitpts, splitpts_size);

	// If there is only 1 point in the CSV file, throw an error
	if (splitpts_size < 2)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Not enough points to generate a split path." << std::endl;
		// Deallocate the pointer before throwing the error
		delete[] splitpts;
		splitpts = nullptr;
		// This must be a fatal error
		this->error_handler();
	}

	/*
	 * Create a wire from the input points
	 */

	// Process delamination points
	SPAposition* split_wirepts = ACIS_NEW SPAposition[splitpts_size];
	for (int i = 0; i < splitpts_size; i++)
		split_wirepts[i] = SPAposition(splitpts[i].x(), splitpts[i].y(), start_z);

	// Delete the points read from CSV file as we no longer need them
	delete[] splitpts;
	splitpts = nullptr;

	// Create a spline that interpolates the input points
	EDGE* spline_edge;
	this->_check_outcome(api_curve_spline(splitpts_size, split_wirepts, NULL, NULL, spline_edge, true, true), __FILE__, __LINE__, __FUNCTION__);

	// Create a wire body from the spline edge
	BODY* split_wire;
	this->_check_outcome(api_make_ewire(1, &spline_edge, split_wire), __FILE__, __LINE__, __FUNCTION__);

	// Split a copy of the mold to find reference points
	ENTITY* layer_mold_copy_ent;
	LayerMold** layer_in_molds = layer_in->list_mold();
	int layer_in_molds_size = layer_in->size_mold();
	for (int i = 0; i < layer_in_molds_size; i++)
	{
		// Assuming that input layer has only one LayerMold element on the original direction
		if (layer_in_molds[i]->direction() == Direction::ORIG)
			this->_check_outcome(api_deep_down_copy_entity((ENTITY*)layer_in_molds[i]->body(), layer_mold_copy_ent), __FILE__, __LINE__, __FUNCTION__);
	}
	BODY* layer_mold_copy = (BODY*)layer_mold_copy_ent;

	// Get faces for wire projection
	ENTITY_LIST face_list;
	this->_check_outcome(api_get_faces(layer_mold_copy, face_list), __FILE__, __LINE__, __FUNCTION__);

	// Project wire to the mold
	project_options project_opts;
	project_opts.set_target_faces(face_list);
	project_opts.set_imprint_type(proj_imprint_type::IMPRINT_ALL);
	this->_check_outcome(api_project_wire_to_body(split_wire, layer_mold_copy, &project_opts), __FILE__, __LINE__, __FUNCTION__);

	// Now, we have the split mold with multiple faces. Create sheet bodies from these faces.
	face_list.clear();
	this->_check_outcome(api_get_faces(layer_mold_copy, face_list), __FILE__, __LINE__, __FUNCTION__);
	face_list.init(); FACE* f; ENTITY_LIST body_list;
	while (f = (FACE*)face_list.next())
	{
		BODY* sb_temp;
		this->convert_face_to_sheet(f, sb_temp);
		body_list.add(sb_temp);
	}

	/*
	 * Sweep the wire body along the layer thickness to generate a face to be used in chopping operation
	 */

	 // Set sweep options
	sweep_options sweep_opts;
	sweep_opts.set_bool_type(KEEP_BOTH);

	SPAvector sweep_path(0, 0, (end_z - start_z));

	// Sweep through the sweeping path to create a body
	BODY* tool_body = NULL;
	this->_check_outcome(api_sweep_with_options(split_wire, sweep_path, &sweep_opts, tool_body), __FILE__, __LINE__, __FUNCTION__);

	this->_check_outcome(api_body_to_1d(split_wire, true), __FILE__, __LINE__, __FUNCTION__);

	this->process_split(layer_in, split_wire);

	// Find LayerSurface directions
	this->find_initial_ls_direction(layer_in, body_list);
}

void ACISModelBuilder::process_split(Layer *layer_in, BODY* tool_body)
{
	/*
	 * Split the layer by chopping it (a shortcut for boolean subtract and intersect)
	 */

	// Get the blank body
	LayerBody* layerbody_in = layer_in->list()[0];
	assert(layer_in->size()==1); // because for now it looks like we only operate on the first body (!)
	BODY* blank_body = layerbody_in->body(); // This is going to be modified to store intersection result after chopping

	// Set the parameters and chop the original layer
	logical nonreg = false;
	BODY* outside = NULL; // This will store the chopped body
	BODY* leftovers = NULL;
	BODY* result_body = NULL;
	this->_check_outcome(api_boolean_chop_body(tool_body, blank_body, nonreg, outside, leftovers, NDBOOL_KEEP::NDBOOL_KEEP_BOTH, result_body), __FILE__, __LINE__, __FUNCTION__);

	// Initialize the list of errors to null
	insanity_list* ilist = nullptr;
	// Check chopping errors
	this->_check_outcome(api_check_entity((ENTITY*)outside, ilist), __FILE__, __LINE__, __FUNCTION__);

	if (ilist != nullptr)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Layer splitting failed due to invalid geometry!" << std::endl;
		this->error_handler();
	}

	/*
	 * Find new bodies and update Layer structure
	 */

	// Put the split bodies in an entity list
	ENTITY_LIST body_list;
	body_list.add(result_body);
	body_list.add(outside);

	// Get a reference normal
	SPAunit_vector reference_normal;
	for (auto lb : *layer_in)
	{
		for (auto ls : *lb)
		{
			if (Direction::OFFSET == ls->direction())
				reference_normal = ls->normal();
		}
	}

	// Delete the layer bodies stored inside the layer
	layer_in->clear();

	// Loop through the list of newly generated BODY objects
	body_list.init();
	BODY* b; int mold_cnt = 0;
	while (b = (BODY*)body_list.next())
	{
		// Get BODY transform for point finding function
		SPAtransf current_body_transf;
		this->get_body_transf(b, current_body_transf);

		// Create a layer body
		LayerBody *current_layerbody=new LayerBody();

		// Set BODY, owner and name of the layer body
		current_layerbody->body(b);
		current_layerbody->owner(layer_in);
		std::string lb_name = std::string(layer_in->name()) + "LB" + std::to_string(layer_in->next_lb_id);
		current_layerbody->name(lb_name.c_str());

		// Let's find the faces
		ENTITY_LIST facelist;
		this->_check_outcome(api_get_faces(b, facelist), __FILE__, __LINE__, __FUNCTION__);

		// Move the cursor to the beginning of the entity list
		facelist.init();

		// Traverse through all faces
		FACE* current_face;
		while (current_face = (FACE*)facelist.next())
		{
			// Find the underlying surface
			surface* current_surf = current_face->geometry()->trans_surface(get_owner_transf(current_face), current_face->sense());
			// Set an evaluation location
			SPApar_pos eval_par(this->_u_pos, this->_v_pos);

			// Find point using the internal point finding algorithm
			SPAposition ref_pos = current_surf->eval_position(eval_par);
			SPAunit_vector eval_normal;
			SPAposition eval_pos;
			this->_pPtNmAlgo->find_point_normal(current_face, current_body_transf, ref_pos, eval_pos, eval_normal);

			// Calculate the angle between the reference normal and the face normal
			double vec_angle = radians_to_degrees(angle_between(eval_normal, reference_normal));

			// Update the layer surface
			LayerSurface *current_layersurface=new LayerSurface();
			current_layersurface->id(current_layerbody->next_ls_id);
			current_layersurface->face(current_face);
			current_layersurface->point(eval_pos);
			current_layersurface->normal(eval_normal);
			current_layersurface->angle(vec_angle);
			// This layer surface is generated at the initial create_layer() stage
			current_layersurface->initial_surface();

			// Set owner of the surface
			current_layersurface->owner(current_layerbody);

			// Add the generated layer surface object to the layer body
			current_layerbody->add_surface(current_layersurface);
		}

		// Add layer body to the layer
		layer_in->add_body(current_layerbody);
	}

	// Update owners
	layer_in->update_owners();
}

void ACISModelBuilder::_check_outcome(const outcome& api_retval, const char* file_name, int line_number, const char* function_name)
{
	// Check if ACIS has encountered any errors (fail-safe or critical)
	if (api_retval.encountered_errors())
	{
		// Query for ACIS error number
		err_mess_type err_no = api_retval.error_number();

		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
		{
			std::cout << "ACIS ERROR: " << find_err_mess(err_no) << " (" << find_err_ident(err_no) << ")" << std::endl;
			//std::cout << "Module Info: " << find_err_module(err_no) << std::endl;
			std::cout << "Function: " << function_name << " in file: " << file_name << " on line: " << line_number << std::endl;
		}

		// Display error information, crash program if critical
		if (api_retval.ok())
		{
			std::cout << "ERROR: ACIS encountered a fail-safe error. Please check the final output for inconsistencies." << std::endl;
			sys_warning(err_no);
		}
		else
		{
			std::cout << "ERROR: ACIS encountered a critical error!" << std::endl;
			sys_error(err_no);
		}
	}

	//// Print warnings
	//err_mess_type *warnings;
	//int nwarn = get_warnings(warnings);
	//for (int i = 0; i < nwarn; ++i)
	//{
	//	//std::cout << "WARNING " << warnings[i] << ": " << find_err_mess(warnings[i]) << std::endl;
	//	sys_warning(warnings[i]);
	//}
	//init_warnings();
}

void ACISModelBuilder::create_hat_stiffener(Layer *layer_orig, Layer *stiffener, const char* file_name, double radius)
{
	// Rounded layers
	if (radius != 0)
		this->_blending_radius = radius;

	// Read stiffener points
	delamo::TPoint3<double>* stiffenerpts = nullptr;
	int num_points;
	read_csv_file(file_name, stiffenerpts, num_points);

	SPAposition ref_point;
	SPAunit_vector ref_normal;
	SPAbox ref_box;

	for (auto orig_lb : *layer_orig)
	{
		for (auto orig_ls : *orig_lb)
		{
			if (Direction::OFFSET == orig_ls->direction() && orig_ls->is_initial_surface())
			{
				ref_point = orig_ls->point();
				ref_normal = orig_ls->normal();
				const SPAtransf* face_transf = get_owner_transf_ptr(orig_ls->face());
				ref_box = get_face_box(orig_ls->face(), face_transf);
			}
		}
	}

	// Set the vertices
	SPAposition* vertex_list = ACIS_NEW SPAposition[num_points];
	for (int i = 0; i < num_points; i++)
	{
		vertex_list[i] = SPAposition(stiffenerpts[i].x(), stiffenerpts[i].y(), ref_point.z() + stiffenerpts[i].z());
	}

	// Delete the points read from CSV file as we no longer need them
	delete[] stiffenerpts;
	stiffenerpts = nullptr;

	//// TODO: Find the thickness of the stiffener
	//double stiffener_thickness = 1.0;

	// Make wire from the vertex list
	BODY* wire_body;
	this->_check_outcome(api_make_wire(NULL, num_points, vertex_list, wire_body), __FILE__, __LINE__, __FUNCTION__);

	// Prepare the path
	SPAposition array_pts[2];
	array_pts[0] = ref_box.low();
	array_pts[0].set_y(0);
	array_pts[0].set_z(ref_point.z() + (layer_orig->thickness() / 2.0));
	array_pts[1] = ref_box.high();
	array_pts[1].set_y(0);
	array_pts[1].set_z(ref_point.z() + (layer_orig->thickness() / 2.0));

	// Set sweep path
	BODY* sweep_path = NULL;
	this->_check_outcome(api_make_wire(NULL, 2, array_pts, sweep_path), __FILE__, __LINE__, __FUNCTION__);

	// Set sweep options
	sweep_options sweep_opts;

	// Sweep the wire body through the defined path
	BODY* swept_body;
	this->_check_outcome(api_sweep_with_options(wire_body, sweep_path, &sweep_opts, swept_body), __FILE__, __LINE__, __FUNCTION__);

	// Do spline conversion
	convert_to_spline_options convertOptions;
	convertOptions.set_do_edges(true);
	convertOptions.set_do_faces(true);
	convertOptions.set_in_place(true);
	this->_check_outcome(api_convert_to_spline(wire_body, &convertOptions, NULL), __FILE__, __LINE__, __FUNCTION__);

	// We know add we have one single layer body object for the stiffener
	LayerBody *stiffener_lb=new LayerBody();
	stiffener_lb->name("SB0");
	stiffener_lb->body(wire_body);
	stiffener_lb->owner(stiffener);

	// Let's find the faces.
	ENTITY_LIST faceList;
	this->_check_outcome(api_get_faces(wire_body, faceList), __FILE__, __LINE__, __FUNCTION__);

	// Move the cursor to the beginning of the entity list
	faceList.init();

	// Traverse through all faces
	FACE* f;
	while (f = (FACE*)faceList.next())
	{
		// Get the surface equation of the face
		surface* current_surf = f->geometry()->trans_surface(get_owner_transf(f), f->sense());

		// Set an evaluation position in the parametric space
		SPApar_pos eval_par(this->_u_pos, this->_v_pos);
		// Find normal using the parametric position
		SPAunit_vector eval_normal = current_surf->eval_normal(eval_par);
		// Find point using the parametric position
		SPAposition eval_pos = current_surf->eval_position(eval_par);

		// Calculate the angle between the reference normal and the face normal
		double vec_angle = radians_to_degrees(angle_between(eval_normal, ref_normal));

		// Create LayerSurface
		LayerSurface *stiffener_ls=new LayerSurface();
		stiffener_ls->id(stiffener_lb->next_ls_id);
		stiffener_ls->face(f);
		stiffener_ls->point(eval_pos);
		stiffener_ls->normal(eval_normal);
		stiffener_ls->angle(vec_angle);

		// This LayerSurface can be considered as one of the initial surfaces
		stiffener_ls->initial_surface();

		// Find LayerSurface direction
		this->find_initial_ls_direction_simple(stiffener_ls, eval_normal, ref_normal);

		// Set owner of the surface
		stiffener_ls->owner(stiffener_lb);

		// Add the generated layer surface object to the layer body
		stiffener_lb->add_surface(stiffener_ls);
	}

	stiffener->add_body(stiffener_lb);
	//stiffener.thickness(stiffener_thickness);
	stiffener->name("HatStiffener");
	stiffener->direction(Direction::OFFSET);

	this->generate_mold(stiffener);

	// Set layer type to "stiffener"
	stiffener->type(LayerType::STIFFENER);

	stiffener->update_owners();

	// Delete vertex list
	ACIS_DELETE[] vertex_list;
	vertex_list = NULL;
}

void ACISModelBuilder::update_surface_pairs(Layer *layer1, Layer *layer2)
{
	/*
	 * Housekeeping
	 */

	// Clear all surface pairs for the 1st input layer
	for (auto& layer1_lb : *layer1)
	{
		for (auto& layer1_ls : *layer1_lb)
		{
			layer1_ls->pair_clear();
		}
	}

	// Clear all surface pairs for the 2nd input layer
	for (auto& layer2_lb : *layer2)
	{
		for (auto& layer2_ls : *layer2_lb)
		{
			layer2_ls->pair_clear();
		}
	}

	/*
	 * Set stiffener special condition
	 */
	bool layer_is_stiffener = false;
	if (LayerType::STIFFENER == layer1->type() || LayerType::STIFFENER == layer2->type())
		layer_is_stiffener = true;

	/*
	 * Start finding surface pairs
	 */

	for (auto& layer1_lb : *layer1) // START L1 LB
	{
		// Get the BODY element of the layer
		BODY* layer1_lb_body = layer1_lb->body();

		// Generate an identity transform for point discovery function
		SPAtransf layer1_lb_transf;

		// If the body has a transform, we must use it
		TRANSFORM* layer1_transfrm = layer1_lb_body->transform();
		if (layer1_transfrm != NULL)
		{
			layer1_lb_transf = layer1_transfrm->transform();
		}

		for (auto& layer1_ls : *layer1_lb) // START L1 LS
		{
			for (auto& layer2_lb : *layer2) // START L2 LB
			{
				for (auto& layer2_ls : *layer2_lb) // START L2 LS
				{
					// Check that the faces in consideration are touching to each other
					if (antiparallel(layer1_ls->normal(), layer2_ls->normal(), this->tolerance()) && (abs(layer1_ls->point_coords().z() - layer2_ls->point_coords().z()) < this->tolerance()))
					{
						// Parametric position for initial guessing of the position
						FACE* face_in = layer2_ls->face();
						surface* surf_in = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());
						SPAinterval u_range = surf_in->param_range_u();
						SPAinterval v_range = surf_in->param_range_v();
						SPApar_pos test_uv_guess(this->_u_pos * u_range.length() + u_range.start_pt(), this->_v_pos * v_range.length() + v_range.start_pt());
						// A variable to store the api_point_in_face() result
						point_face_containment cont_answer;
						// Use the cache
						logical use_cache = TRUE;

						// Check that layer2 face contains the point which is on layer1 face
						this->_check_outcome(api_point_in_face(layer1_ls->point(), layer2_ls->face(), layer1_lb_transf, cont_answer, test_uv_guess, use_cache), __FILE__, __LINE__, __FUNCTION__);

						// Check if we have matching surfaces
						if (point_inside_face == cont_answer/* || point_boundary_face == cont_answer*/)
						{
							// We have a match!
							layer2_ls->pair(layer1_ls);
							layer1_ls->pair(layer2_ls);
							// Pairing with stiffener
							layer2_ls->stiffener_paired(layer_is_stiffener);
							layer1_ls->stiffener_paired(layer_is_stiffener);
						}
					}

					// A dirty fix for fixing stiffener paired surface flags. As a result, it works!
					if (layer_is_stiffener)
					{
						if (layer1_ls->pair() == NULL)
							layer1_ls->stiffener_paired(false);
						if (layer2_ls->pair() == NULL)
							layer2_ls->stiffener_paired(false);
					}

				} // END L2 LS
			} // END L2 LB
		} // END L1 LS
	} // END L1 LB
}

void ACISModelBuilder::imprint_delamination(LayerSurface* layersurface_in, Direction surf_direction, BODY* profile_out, BODY* profile_in, BODY* profile_in_par, delamo::List<LayerSurface *>& lsc_new)
{
	// Get stiffened paired property from the input LayerSurface
	bool stiffener_paired_flag = layersurface_in->is_stiffener_paired();

	// Get the solid body
	LayerBody* current_lb = layersurface_in->owner();
	BODY* current_body = current_lb->body();

	// Get body transformation matrix
	SPAtransf current_body_transf;
	this->get_body_transf(current_body, current_body_transf);

	// Get the face contained in the input layer surface
	FACE* face_in = layersurface_in->face();
	SPAbox face_in_bbox = get_face_box(face_in, &current_body_transf);

	// Prepare ACIS variables for projecting operation
	ENTITY_LIST delamination_faces;
	delamination_faces.add(face_in);

	// Set project options for outside profile body
	project_options project_opts_out;
	project_opts_out.set_target_faces(delamination_faces);
	// Although, projection can imprint wires to the chosen face, most of the time it fails.
	project_opts_out.set_imprint_type(proj_imprint_type::IMPRINT_NONE);

	// Set project options for inside profile body
	project_options project_opts_in;
	project_opts_in.set_target_faces(delamination_faces);
	// Although, projection can imprint wires to the chosen face, most of the time it fails.
	project_opts_in.set_imprint_type(proj_imprint_type::IMPRINT_NONE);

	// Project outer delamination profile to the chosen faces
	this->_check_outcome(api_project_wire_to_body(profile_out, current_body, &project_opts_out), __FILE__, __LINE__, __FUNCTION__);
	BODY* profile_wire_out = project_opts_out.get_projected_wire_body();

	// Project inner delamination profile to the chosen faces
	this->_check_outcome(api_project_wire_to_body(profile_in, current_body, &project_opts_in), __FILE__, __LINE__, __FUNCTION__);
	BODY* profile_wire_in = project_opts_in.get_projected_wire_body();

	// If projections are null, we don't have to continue the imprinting
	if (profile_wire_in == NULL || profile_wire_out == NULL)
		return;

	// Imprint the wires to the block. This will create the delaminated layer by splitting the face automatically
	this->_check_outcome(api_imprint(current_body, profile_wire_out), __FILE__, __LINE__, __FUNCTION__);
	this->_check_outcome(api_imprint(current_body, profile_wire_in), __FILE__, __LINE__, __FUNCTION__);

	// Now, we have the delamination profiles imprinted. Let's find faces to figure out which faces are new.
	ENTITY_LIST facelist;
	this->_check_outcome(api_get_faces(current_body, facelist), __FILE__, __LINE__, __FUNCTION__);

	// Our current layer surface object can provide us some reference points (reference to cohesive zone)
	SPAposition face_refin = layersurface_in->point();

	// Create inner delamination outline as a face in parametric space
	FACE* ref_delam_inner;
	this->generate_delam_ref_face(profile_in_par, ref_delam_inner);

	DelaminationType delam_type;
	facelist.init(); FACE* f;
	while (f = (FACE*)facelist.next())
	{
		// Find the face in the current layer body
		int face_idx = current_lb->face_id(f);

		SPAposition face_refpt;
		SPAunit_vector face_refnormal;
		if (this->_pPtNmAlgo->find_point(f, current_body_transf, face_in_bbox, face_refin, face_refpt, stiffener_paired_flag))
		{
			this->_pPtNmAlgo->find_normal(f, face_refin, face_refnormal);
		}

		// Check if we have this face in our layer surface array
		if (face_idx >= 0)
		{
			// We have the face in our layer surface array
			LayerSurface* face_ls = current_lb->at(face_idx);
			if (surf_direction == face_ls->direction() && face_in == face_ls->face() && DelaminationType::COHESIVE == face_ls->delam_type())
			{
				if (stiffener_paired_flag)
				{
					face_ls->delam_type(DelaminationType::NODELAM);
				}
				else
				{
					this->find_delam_bc(f, current_body_transf, face_refpt, face_refin, ref_delam_inner, delam_type);
					face_ls->delam_type(delam_type);
				}

				// Update point and normal for this layer surface
				face_ls->point(face_refpt);
				face_ls->normal(face_refnormal);
				// And, this is not our initial layer surface anymore
				face_ls->initial_surface(false);
				face_ls->stiffener_paired(stiffener_paired_flag);
			}
		}
		else
		{
			// face_idx is -1, so this face might be our new face. Let's check...
			bool _process_new_face = false;
			ATTRIB_GEN_INTEGER* face_attrib = (ATTRIB_GEN_INTEGER*)find_attrib((ENTITY*)f);
			if (face_attrib != NULL)
			{
				// This is an existing face
				int attrib_val = face_attrib->value();
				if (attrib_val == 0)
					_process_new_face = true;
			}
			else
			{
				// The face must be new
				_process_new_face = true;
			}

			// Only consider this face if it is not marked as "processed"
			if (_process_new_face)
			{
				// For the next iterations, this entity must be considered as an existing face
				this->_check_outcome(api_add_generic_named_attribute(f, "DELAM_FACE_PROCESSED", 1), __FILE__, __LINE__, __FUNCTION__);

				// Continue with the geometric operations
				SPAunit_vector ref_normal(0, 0, 1);
				double vec_angle = radians_to_degrees(angle_between(ref_normal, face_refnormal));

				// Generate a new layer surface
				LayerSurface *ls_new = new LayerSurface();
				ls_new->id(current_lb->next_ls_id);
				ls_new->face(f);
				ls_new->point(face_refpt);
				ls_new->normal(face_refnormal);
				ls_new->owner(current_lb);
				ls_new->angle(vec_angle);
				ls_new->direction(surf_direction);
				ls_new->stiffener_paired(stiffener_paired_flag);
				//ls_new->created_from(layersurface_in);

				if (stiffener_paired_flag)
				{
					ls_new->delam_type(DelaminationType::NODELAM);
				}
				else
				{
					this->find_delam_bc(f, current_body_transf, face_refpt, face_refin, ref_delam_inner, delam_type);
					ls_new->delam_type(delam_type);
				}

				//ls_new.delam_type(DelaminationType::NEEDS_UPDATE);

				// Add to the container
				lsc_new.add(ls_new);
			}
		}
	}

	// Reset the ENTITY_LIST that holds delamination faces
	delamination_faces.clear();
}

void ACISModelBuilder::generate_stiffened_mold(Layer *lamina, Layer *stiffener)
{
	// Container to store new layer molds
	delamo::List<LayerMold *> lmc_new;

	/*
	* GET THE DETAILS OF THE STIFFENER
	*/

	ENTITY_LIST to_be_stitched_faces;
	delamo::List<LayerSurface*> to_be_subtracted;

	for (auto stiffener_lb : *stiffener)
	{
		for (auto stiffener_ls : *stiffener_lb)
		{
			if (Direction::OFFSET == stiffener_ls->direction())
				to_be_stitched_faces.add(stiffener_ls->face());
			if (Direction::ORIG == stiffener_ls->direction())
				to_be_subtracted.add(stiffener_ls);
		}
	}

	for (auto lamina_lb : *lamina)
	{
		for (auto lamina_ls : *lamina_lb)
		{
			if (Direction::OFFSET == lamina_ls->direction())
			{
				for (auto subs_ls : to_be_subtracted)
				{
					// Parametric position for initial guessing of the position
					FACE* face_in = lamina_ls->face();
					surface* surf_in = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());
					SPAinterval u_range = surf_in->param_range_u();
					SPAinterval v_range = surf_in->param_range_v();
					SPApar_pos test_uv_guess(this->_u_pos * u_range.length() + u_range.start_pt(), this->_v_pos * v_range.length() + v_range.start_pt());
					SPAtransf transf_in;
					// SImply, the result
					point_face_containment cont_answer;
					// Use the cache
					logical use_cache = TRUE;

					// Query for a reference point on the input face
					this->_check_outcome(api_point_in_face(subs_ls->point(), lamina_ls->face(), transf_in, cont_answer, test_uv_guess, use_cache), __FILE__, __LINE__, __FUNCTION__);

					if (cont_answer != point_inside_face)
					{
						to_be_stitched_faces.add(lamina_ls->face());
					}
				}
			}
		}
	}


	/*
	* STITCH THE FACES
	*/

	// Loop through the entity list to create sheet bodies as api_stitch likes them more than FACEs
	ENTITY_LIST to_be_stitched;
	to_be_stitched_faces.init();
	for (int i = 0; i < to_be_stitched_faces.iteration_count(); i++)
	{
		FACE* face = (FACE*)to_be_stitched_faces[i];
		FACE* face_list[1];
		face_list[0] = face;
		BODY* sheet_body;
		this->_check_outcome(api_sheet_from_ff(1, face_list, sheet_body), __FILE__, __LINE__, __FUNCTION__);
		to_be_stitched.add(sheet_body);
	}

	// Now, we have the sheet bodies. We can call the Core Stitching API.
	ENTITY_LIST output_bodies;

	// We can use multi-threaded stitching too. First, we need to make an input from the list of faces.
	stitch_input_handle* sih;
	this->_check_outcome(api_stitch_make_input(to_be_stitched, sih), __FILE__, __LINE__, __FUNCTION__);

	// Stitch the faces and get the output
	stitch_output_handle* soh;
	this->_check_outcome(api_stitch(sih, soh, output_bodies), __FILE__, __LINE__, __FUNCTION__);

	// Get the stitching result
	mt_stitch_query_container* scont;
	this->_check_outcome(api_stitch_query(soh, scont), __FILE__, __LINE__, __FUNCTION__);

	if (this->_blending_radius != 0)
	{
		// Query for the stitched edges
		ENTITY_LIST stitched_edges;
		scont->get_stitched_edges(stitched_edges);

		// Blend the stitched edges using the Blending API
		double blending_radius = this->_blending_radius;
		this->_check_outcome(api_blend_edges(stitched_edges, blending_radius), __FILE__, __LINE__, __FUNCTION__);
	}

	// So, the stiching operation was successful, and we would be able to generate a surface from the stitched body
	output_bodies.init();
	// The "output_bodies" list should have one single stitched body
	BODY* stitched_body = (BODY*)output_bodies.first();

	// Create a new mold for the stiffened layer
	LayerMold *new_mold=new LayerMold();
	new_mold->body(stitched_body);
	new_mold->direction(Direction::OFFSET);
	new_mold->owner(lamina);
	new_mold->stiffener_gen(true);

	// Add the new mold to the container
	lmc_new.add(new_mold);

	/*
	* UPDATE THE BASE LAYER MOLDS
	*/

	// Update the mold for the base layer
	if (0 != lmc_new.size())
	{
		// We only update the offset mold
		LayerMold** bl_molds = lamina->list_mold();
		int bl_molds_size = lamina->size_mold();
		for (int i = 0; i < bl_molds_size; i++)
		{
			if (Direction::ORIG == bl_molds[i]->direction())
				lmc_new.add(bl_molds[i]);
		}

		// Clear the old mold list and update it with the new one
		lamina->clear_mold();
		for (auto lm_new : lmc_new)
		{
			lamina->add_mold(lm_new);
		}
	}
}

void ACISModelBuilder::find_closest_points(Layer** layer_list, int layer_list_size, delamo::TPoint3<double> point_in, delamo::TPoint3<double>*& point_list, delamo::TPoint3<double>*& normal_list, char**& name_list, int& list_size)
{
	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
	{
		std::cout << "WARNING: This function is not implemented yet!" << std::endl;
	}

	return;
}

void ACISModelBuilder::find_closest_face_to_point(Layer *layer_in, delamo::TPoint3<double> point_in, delamo::TPoint3<double>& point_out, delamo::TPoint3<double>& normal_out, char*& name_out)
{
	// Find the closest face
	double distance_min = std::numeric_limits<int>::max();
	SPAposition in_point(point_in.x(), point_in.y(), point_in.z());
	for (auto lb : *layer_in)
	{
		// Find closest face to the given point
		SPAposition closest_pos;
		double distance;
		param_info ent_info;
		this->_check_outcome(api_entity_point_distance(lb->body(), in_point, closest_pos, distance, ent_info), __FILE__, __LINE__, __FUNCTION__);
		if (distance < distance_min)
		{
			FACE* closest_face;
			if (ent_is_edge == ent_info.entity_type())
				// Going from EDGE -> COEDGE -> LOOP -> FACE
				closest_face = (FACE*)((((EDGE*)ent_info.entity())->owner())->owner())->owner();
			else
				closest_face = (FACE*)ent_info.entity();

			// Find face normal
			SPAunit_vector face_normal = sg_get_face_normal(closest_face, in_point);
			// Add face reference point and normal to the relevant list
			point_out = delamo::TPoint3<double>(closest_pos.x(), closest_pos.y(), closest_pos.z());
			normal_out = delamo::TPoint3<double>(face_normal.x(), face_normal.y(), face_normal.z());
			name_out = strdup(lb->name());

			// Set the minimum distance
			distance_min = distance;
		}
	}
}

void ACISModelBuilder::adjacent_layers(Layer *layer_orig, Layer* layer_offset, delamo::List<std::string>& file_names)
{
	// Update surface pairs before processing delamination
	this->update_surface_pairs(layer_offset, layer_orig);

	for (auto file_name : file_names)
	{
		// Read delamination points
		delamo::TPoint3<double>* delampts = nullptr;
		int delampts_size;
		read_csv_file(file_name.c_str(), delampts, delampts_size);

		// Check if we were able to load some points from the file
		if (delampts == nullptr)
		{
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
				std::cout << "ERROR: There is a problem processing delamination profile. Please check your input file: " << std::string(file_name) << std::endl;
			this->error_handler();
		}
		else
		{
			// Check if the last and the first delamination profile points are equal
			if (delampts[0] != delampts[delampts_size - 1])
			{
				if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
					std::cout << "ERROR: The first and the last delamination profile points must be equal. Skipping delamination imprint..." << std::endl;
				continue;
			}
		}

		// Process delamination
		this->process_delamination(layer_orig, layer_offset, delampts, delampts_size);

		// Delete delamination points
		delete[] delampts;
		delampts = nullptr;

		// Update layer surface owners after imprinting operation
		layer_orig->update_owners();
		layer_offset->update_owners();

		// New layer surfaces generated by imprinting will be paired here
		this->update_surface_pairs(layer_offset, layer_orig);
	}

	// Set adjacent pairs
	layer_offset->bond_pair(Direction::ORIG, layer_orig);
	layer_orig->bond_pair(Direction::OFFSET, layer_offset);

	if (this->_pInitialLayer == nullptr)
		this->_pInitialLayer = layer_orig;
}

void ACISModelBuilder::process_delamination(Layer *layer_orig, Layer *layer_offset, delamo::TPoint3<double>* delampts, int delampts_size)
{
	// Check if the orig layer has 1 mold with a single face on its offset direction
	LayerMold* delam_profile_ref_mold = layer_orig->delam_profile_ref();
	if (delam_profile_ref_mold == nullptr)
	{
		std::cout << "ERROR: Cannot find a reference face for delamination profile generation. Please check your input layers. Aborting delamination imprinting!" << std::endl;
		return;
	}

	BODY* ref_mold_sb = delam_profile_ref_mold->body();
	ENTITY_LIST face_list;
	this->_check_outcome(api_get_faces(ref_mold_sb, face_list), __FILE__, __LINE__, __FUNCTION__);

	FACE* ref_mold = NULL;
	if (face_list.iteration_count() > 1)
	{
		std::cout << "ERROR: Reference mold has multiple faces. Aborting delamination imprinting!" << std::endl;
		return;
	}
	else
	{
		ref_mold = (FACE*)face_list[0];
	}

	if (ref_mold == NULL)
	{
		std::cout << "ERROR: Please check your input layers and verify that they have correct molds. Aborting delamination imprinting!" << std::endl;
		return;
	}

	// Imprint delamination shape to both layers
	for (auto& lb_orig : *layer_orig) // START LB LOOP
	{
		// Container to store faces after imprint operation
		delamo::List<LayerSurface *> lsc_new_orig;

		for (auto& ls_orig : *lb_orig) // START LS LOOP
		{
			// Only imprint to the faces in the chosen direction
			if (Direction::OFFSET == ls_orig->direction() && DelaminationType::COHESIVE == ls_orig->delam_type())
			//if (Direction::OFFSET == ls_orig.direction() && DelaminationType::COHESIVE == ls_orig.delam_type() && !ls_orig.is_stiffener_paired())
			{
				// Generate delamination profiles
				BODY* inner_profile; BODY* inner_profile_parametric;  BODY* outer_profile; BODY* outer_profile_parametric;
				this->generate_delamination_profiles(ref_mold, delampts, delampts_size, outer_profile, inner_profile, outer_profile_parametric, inner_profile_parametric);

				// Imprint delamination outlines
				this->imprint_delamination(ls_orig, Direction::OFFSET, outer_profile, inner_profile, inner_profile_parametric, lsc_new_orig);

				// OFFSET PAIR
				LayerSurface* ls_offset = ls_orig->pair();
				if (ls_offset != nullptr)
				{
					LayerBody* lb_offset = ls_offset->owner();

					// Imprint delamination outlines
					delamo::List<LayerSurface *> lsc_new_offset;
					this->imprint_delamination(ls_offset, Direction::ORIG, outer_profile, inner_profile, inner_profile_parametric, lsc_new_offset);

					if (lsc_new_offset.size() > 0)
					{
						for (auto ls_new : lsc_new_offset)
						{
							lb_offset->add_surface(ls_new);
						}
					}
					lsc_new_offset.clear();
				}

				// Update surface pairs after each imprinting couple
				this->update_surface_pairs(layer_orig, layer_offset);
			}
		} // END LS LOOP

		  // Add new faces to the layer body
		if (lsc_new_orig.size() > 0)
		{
			for (auto ls_new : lsc_new_orig)
			{
				lb_orig->add_surface(ls_new);
			}
		}
		lsc_new_orig.clear();

	} // END LB LOOP
}

void ACISModelBuilder::generate_delam_ref_face(BODY* delam_wire, FACE*& delam_face_inner)
{
	// Create a planar face in parametric space defined with the interval [0, 1]
	SPAposition ref_point(0.0, 0.0, 0.0); SPAvector ref_normal(0.0, 0.0, 1.0);
	FACE* ref_face_cover;
	this->_check_outcome(api_face_plane(ref_point, 1.0, 1.0, &ref_normal, ref_face_cover), __FILE__, __LINE__, __FUNCTION__);

	// Wire covering operates better with a reference surface
	surface* ref_face_surf = ref_face_cover->geometry()->trans_surface(get_owner_transf(ref_face_cover), ref_face_cover->sense());
	//const surface& ref_face_surf = *(surface*)NULL_REF;

	// Copy delam wire as wire covering deletes it
	ENTITY* delam_wire_copy_ent;
	this->_check_outcome(api_deep_down_copy_entity(delam_wire, delam_wire_copy_ent), __FILE__, __LINE__, __FUNCTION__);
	BODY* delam_wire_copy = (BODY*)delam_wire_copy_ent;

	// Cover wires to create inner delamination outline as a face
	ENTITY_LIST covered_faces;
	this->_check_outcome(api_cover_wires(delam_wire_copy, *ref_face_surf, covered_faces), __FILE__, __LINE__, __FUNCTION__);

	int num_covered_faces = covered_faces.iteration_count();
	if (num_covered_faces != 1)
	{
		// @TODO: This part needs a little bit more testing and brushing
		std::cout << "There is a problem with the wire covering operation!" << std::endl;
	}
	delam_face_inner = (FACE*)covered_faces[0];
}

void ACISModelBuilder::find_delam_bc(FACE* face_in, SPAtransf& transf_in, SPAposition& ref_point, SPAposition& ref_cohesive_zone, FACE* ref_contact_zone, DelaminationType& delam_type)
{
	// Set up some required variables for face-point containment API
	surface* surf_in = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());
	SPAinterval u_range = surf_in->param_range_u();
	SPAinterval v_range = surf_in->param_range_v();
	SPApar_pos test_uv_guess(this->_u_pos * u_range.length() + u_range.start_pt(), this->_v_pos * v_range.length() + v_range.start_pt());
	point_face_containment cont_answer;
	logical use_cache = TRUE;

	// Test cohesive zone
	this->_check_outcome(api_point_in_face(ref_cohesive_zone, face_in, transf_in, cont_answer, test_uv_guess, use_cache), __FILE__, __LINE__, __FUNCTION__);
	if (cont_answer == point_inside_face)
	{
		delam_type = DelaminationType::COHESIVE;
		return;
	}

	//// Debugging contact zone
	//FILE* fdbg = fopen("debug_face.txt", "w");
	//ref_contact_zone->debug_ent(fdbg);
	//fclose(fdbg);

	// Find parametric position of the input reference point
	SPApar_pos ref_point_pp = surf_in->param(ref_point);
	// Apply type conversion to parametric reference point
	SPAposition ref_point_pp_conv(ref_point_pp.u / u_range.length(), ref_point_pp.v / v_range.length(), 0.0);

	// Please note that contact zone reference face is in parametric space
	SPAtransf identity_transf;
	this->_check_outcome(api_point_in_face(ref_point_pp_conv, ref_contact_zone, identity_transf, cont_answer, test_uv_guess, use_cache), __FILE__, __LINE__, __FUNCTION__);
	if (cont_answer == point_inside_face)
	{
		delam_type = DelaminationType::CONTACT;
		return;
	}

	// So, the reference point must be inside the no model zone
	delam_type = DelaminationType::NOMODEL;
	return;
}

void ACISModelBuilder::find_initial_ls_direction_simple(LayerSurface *ls_in, SPAunit_vector& eval_normal, SPAunit_vector& reference_normal)
{
	// Calculate the angle between the reference normal and the face normal
	double vec_angle = radians_to_degrees(angle_between(eval_normal, reference_normal));

	// Find offset face
	if (parallel(eval_normal, reference_normal, this->tolerance()))
	{
		ls_in->direction(Direction::OFFSET);
	}
	// Find orig face
	else if (antiparallel(eval_normal, reference_normal, this->tolerance()))
	{
		ls_in->direction(Direction::ORIG);
	}
	// Offset side inclined faces
	else if (vec_angle >= 1.0 + this->tolerance() && vec_angle <= 89.0 - this->tolerance())
	{
		ls_in->direction(Direction::OFFSET);
	}
	// Other side should considered as an orig face
	else if (vec_angle >= 91.0 + this->tolerance() && vec_angle <= 179.0 - this->tolerance())
	{
		ls_in->direction(Direction::ORIG);
	}
	// These should be the side faces
	else
	{
		ls_in->direction(Direction::SIDE);
		ls_in->delam_type(DelaminationType::NODELAM);
	}
}

void ACISModelBuilder::find_initial_ls_direction(Layer *layer_in, ENTITY_LIST& mold_list)
{
	delamo::List<SPAposition> refpts_vec;
	delamo::List<SPAunit_vector> refnormals_vec;
	delamo::List<REVBIT> refsense_vec;
	mold_list.init(); BODY* b;
	while (b = (BODY*)mold_list.next())
	{
		ENTITY_LIST facelist_mold;
		this->_check_outcome(api_get_faces(b, facelist_mold), __FILE__, __LINE__, __FUNCTION__);

		SPAtransf sheet_mold_transf;
		this->get_body_transf(b, sheet_mold_transf);

		// Get all faces of the mold sheet body to find reference points
		facelist_mold.init(); FACE* sheet_face;
		while (sheet_face = (FACE*)facelist_mold.next())
		{
			SPAbox face_box = get_face_box(sheet_face);
			surface* sheet_surf = sheet_face->geometry()->trans_surface(get_owner_transf(sheet_face), sheet_face->sense());
			SPApar_pos sheet_surf_eval_pp(this->_u_pos, this->_v_pos);
			SPAposition sheet_mold_refpt_in = sheet_surf->eval_position(sheet_surf_eval_pp);
			SPAposition mold_refpt;
			this->_pPtNmAlgo->find_point(sheet_face, sheet_mold_transf, face_box, sheet_mold_refpt_in, mold_refpt);
			refpts_vec.add(mold_refpt);
			SPAunit_vector mold_refnormal;
			this->_pPtNmAlgo->find_normal(sheet_face, mold_refpt, mold_refnormal);
			refnormals_vec.add(mold_refnormal);
			refsense_vec.add(sheet_face->sense());
		}
	}

	// Start looping through all reference points
	int itrcnt = 0;
	for (auto refpt : refpts_vec)
	{
		// Initialize required variables
		FACE* ref_face = NULL;
		SPAposition orig_pos, offset_pos; SPAtransf offset_translation; SPAunit_vector ref_normal;

		// Find LayerSurface on the original direction
		for (auto& lb_in : *layer_in)
		{
			for (auto& ls_in : *lb_in)
			{
				FACE* face_in = ls_in->face();
				SPAtransf face_transf = get_owner_transf(face_in);

				// Check if the reference point found using the mold loop is on the face
				if (this->_pPtNmAlgo->point_in_face(refpt, face_in, face_transf))
				{
					logical normal_check;
					// Normal check depends on the face sense
					if (TRUE == refsense_vec[itrcnt])
						normal_check = parallel(ls_in->normal(), refnormals_vec[itrcnt], this->tolerance());
					else
						normal_check = antiparallel(ls_in->normal(), refnormals_vec[itrcnt], this->tolerance());

					// Sometimes api_point_in_face gets confused, so check if the normals are parallel to make sure that we are processing the correct face
					if (normal_check)
					{
						SPAposition mid_pos_orig;
						if (this->_pPtNmAlgo->find_point_mid(face_in, face_transf, mid_pos_orig))
						{
							// We found the face which belongs to the reference point, this must be our orig face
							ls_in->direction(Direction::ORIG);
							ref_face = face_in;
							orig_pos = mid_pos_orig;
						}
					}
				}
			}
		}

		// There must be something wrong if we have an empty reference face object
		if (ref_face == NULL)
			this->error_handler();

		// Find normals and offset positions
		this->_pPtNmAlgo->find_normal(ref_face, orig_pos, ref_normal);
		if (layer_in->direction() == Direction::OFFSET)
			ref_normal = -ref_normal;
		SPAvector ref_translate = ref_normal * layer_in->thickness();
		offset_translation = translate_transf(ref_translate);
		offset_pos = orig_pos * offset_translation;

		// Find offset LayerSurface elements
		for (auto& lb_in : *layer_in)
		{
			int layer_id = -1;
			double min_dist = std::numeric_limits<double>::max();

			for (auto& ls_in : *lb_in)
			{
				FACE* face_in = ls_in->face();
				// We skip already-processed orig LayerSurfaces
				if (ls_in->direction() == Direction::NODIR)
				{
					SPAposition mid_pos_offset;
					SPAtransf face_transf = get_owner_transf(face_in);
					this->_pPtNmAlgo->find_point_mid(face_in, face_transf, mid_pos_offset);

					double distpt = distance_to_point(mid_pos_offset, offset_pos);
					if (distpt <= min_dist)
					{
						layer_id = ls_in->id();
						min_dist = distpt;
					}
				}
			}

			// Check if we have an offset face
			if (layer_id != -1)
			  (lb_in->at(layer_id))->direction(Direction::OFFSET);
			else
				std::cout << "ERROR: Cannot find offset face!" << std::endl;
		}

		// Increment normal list counter (there is no enumerate capability in C++11)
		itrcnt++;
	} // End reference point loop

	// Clear reference point and normal list
	refpts_vec.clear();
	refnormals_vec.clear();

	// Find side LayerSurface elements
	for (auto& lb_in : *layer_in)
	{
		for (auto& ls_in : *lb_in)
		{
			if (ls_in->direction() == Direction::NODIR)
			{
				ls_in->direction(Direction::SIDE);
			}
		}
	}
}

void ACISModelBuilder::get_body_transf(BODY* body_in, SPAtransf& transf_out)
{
	TRANSFORM* cb_transfrm = body_in->transform();
	if (cb_transfrm != NULL)
	{
		transf_out = cb_transfrm->transform();
	}
}

void ACISModelBuilder::parpos_csv_to_pos(const char* csv_in, Layer *ref_layer, Direction ref_dir, const char* csv_out)
{
	if (ref_layer->size() > 1)
	{
		std::cout << "MULTI LB: This layer has already been altered by some damage-incorporation method!" << std::endl;
		this->error_handler();
	}

	FACE* ref_face = NULL;
	for (auto ref_lb : *ref_layer)
	{
		int face_cnt = 0;
		for (auto ref_ls : *ref_lb)
		{
			if (ref_ls->direction() == ref_dir)
			{
				ref_face = ref_ls->face();
				face_cnt++;
			}
		}
		if (face_cnt > 1)
		{
			std::cout << "MULTI LS: This layer has already been altered by some damage-incorporation method!" << std::endl;
			this->error_handler();
		}
	}

	if (ref_face == NULL)
	{
		std::cout << "NO FACE: This layer has not been correctly generated!" << std::endl;
		this->error_handler();
	}

	surface* ref_surf = ref_face->geometry()->trans_surface(get_owner_transf(ref_face), ref_face->sense());
	SPAinterval u_range = ref_surf->param_range_u();
	SPAinterval v_range = ref_surf->param_range_v();

	delamo::TPoint3<double>* pps; int pps_size;
	read_csv_file(csv_in, pps, pps_size);

	std::ofstream outfile;
	outfile.open(csv_out);
	outfile << "\"Points:0\",\"Points:1\",\"Points:2\"\n";

	for (int i = 0; i < pps_size; i++)
	{
		SPApar_pos pp_temp((pps[i].x() * u_range.length() + u_range.start_pt()), (pps[i].y() * v_range.length() + v_range.start_pt()));
		SPAposition pos_temp = ref_surf->eval_position(pp_temp);
		outfile << pos_temp.x() << "," << pos_temp.y() << "," << pos_temp.z() << "\n";
	}

	outfile.close();
}

void ACISModelBuilder::generate_delamination_profiles(FACE* face_in, delamo::TPoint3<double>* delampts, int delampts_size, BODY*& profile_out, BODY*& profile_in, BODY*& profile_out_par, BODY*& profile_in_par)
{
	// Get body transformation matrix
	const SPAtransf* current_transf = get_owner_transf_ptr(face_in);

	// Get the face contained in the input layer surface
	SPAbox face_in_bbox = get_face_box(face_in, current_transf);

	double multifact;
	double face_x_len = face_in_bbox.x_range().length();
	double face_y_len = face_in_bbox.y_range().length();
	if (face_x_len > face_y_len)
		multifact = face_x_len;
	else
		multifact = face_y_len;

	// First find closest points to the face itself
	SPAposition* outer_delam_points = ACIS_NEW SPAposition[delampts_size + 1];
	for (int i = 0; i < delampts_size; i++)
	{
		outer_delam_points[i].set_x(delampts[i].x());
		outer_delam_points[i].set_y(delampts[i].y());
		outer_delam_points[i].set_z(delampts[i].z());
	}
	outer_delam_points[delampts_size] = outer_delam_points[0];

	SPAposition* projected_points_outer = ACIS_NEW SPAposition[delampts_size + 1];
	double* distances = new double[delampts_size + 1];
	this->_check_outcome(api_entity_point_distance(face_in, delampts_size+1, outer_delam_points, projected_points_outer, distances), __FILE__, __LINE__, __FUNCTION__);

	// No need for the distance values
	delete[] distances;
	distances = nullptr;

	// Find parametric positions of the delamination outline w.r.t. input face's surface
	surface* surf_in = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());
	// Sometimes ACIS returns the parametric interval different than [0,1]. Let's get this interval.
	SPAinterval u_range = surf_in->param_range_u();
	SPAinterval v_range = surf_in->param_range_v();
	SPAposition* outer_parpos = ACIS_NEW SPAposition[delampts_size+1];
	for (int i = 0; i < delampts_size; i++)
	{
		// Find parametric position of the 3D point w.r.t to the input surface
		SPApar_pos temp;
		temp = surf_in->param(projected_points_outer[i]);
		// Scale parametric positions into [0, 1] interval
		temp.u = temp.u / u_range.length();
		temp.v = temp.v / v_range.length();
		// Store parametric positions as SPAposition
		outer_parpos[i].set_x(temp.u);
		outer_parpos[i].set_y(temp.v);
		outer_parpos[i].set_z(0.0);
	}
	// Just to fix double-precision errors due to surface evaluation
	outer_parpos[delampts_size] = outer_parpos[0];

	// Create a spline that interpolates the outer parametric positions points
	EDGE* spline_edge_out_par;
	this->_check_outcome(api_curve_spline(delampts_size + 1, outer_parpos, NULL, NULL, spline_edge_out_par, TRUE, TRUE), __FILE__, __LINE__, __FUNCTION__);

	// Create a wire body from the spline edge
	//BODY* profile_out_par;
	this->_check_outcome(api_make_ewire(1, &spline_edge_out_par, profile_out_par), __FILE__, __LINE__, __FUNCTION__);

	// Check for open wires
	if (!api_closed_wire(profile_out_par).ok())
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Input delamination profile points generate an open wire. Check your input!" << std::endl;
		this->error_handler();
	}

	//// Try to estimate the offset wire normal by using curve tangents and cross product
	//const curve& poutp_crv = spline_edge_out_par->geometry()->equation();
	//SPAinterval poutp_crv_int = poutp_crv.param_range();
	//SPAunit_vector wire_tangent_start = poutp_crv.eval_direction(poutp_crv_int.start_pt());
	//SPAunit_vector wire_tangent_next = poutp_crv.eval_direction(poutp_crv_int.mid_pt());
	//SPAvector wire_crosspr = wire_tangent_start * wire_tangent_next;

	// A positive offset distance is in the direction of the cross product (wire_tangent x wire_plane_normal). A negative offset is in the opposite direction.
	SPAunit_vector wire_normal(0.0, 0.0, 1.0);

	// Set wire offset options for the inner profile
	wire_offset_options wire_opts;
	wire_opts.set_plane_normal(wire_normal);
	wire_opts.set_distance(this->offset_distance() / multifact);
	//wire_opts.set_gap_type(sg_gap_type::arc);

	// Offset outer profile to create inner profile.
	// @see https://doc.spatial.com/get_doc_page/articles/o/f/f/HowTo~Offset_planar_wires_f19f.html
	//BODY* profile_in_par;
	this->_check_outcome(api_offset_planar_wire(profile_out_par, &wire_opts, profile_in_par), __FILE__, __LINE__, __FUNCTION__);

	//// Check for open wires
	//if (!api_closed_wire(profile_in_par).ok())
	//{
	//	if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
	//		std::cout << "ERROR: Offset delamination profile points generate an open wire. Check your input!" << std::endl;
	//	this->error_handler();
	//}

	// Check if we have more than 1 loop (which would cause some problems)
	ENTITY_LIST loop_list;
	this->_check_outcome(api_get_loops(profile_in_par, loop_list), __FILE__, __LINE__, __FUNCTION__);
	if (loop_list.iteration_count() > 2)
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Offsetting input profile generates more than 1 loop. Please check your input profile!" << std::endl;
		this->error_handler();
	}

	// profile_in is generated in parametric space. We need to find 3D points.
	ENTITY_LIST edge_list;
	this->_check_outcome(api_get_edges(profile_in_par, edge_list), __FILE__, __LINE__, __FUNCTION__);

	// Offset wire should have any number of edges
	delamo::List<SPAposition> inner_parpos;
	EDGE* eg; edge_list.init();
	while (eg = (EDGE*)edge_list.next())
	{
		const curve& wire_in_curve = eg->geometry()->equation();
		SPAinterval wire_in_range = wire_in_curve.param_range();
		double wire_in_range_len = wire_in_range.length();

		// Scan the curve parametric space
		double wire_u = wire_in_range.start_pt();
		while (wire_u <= wire_in_range.end_pt())
		{
			SPAposition temp = wire_in_curve.eval_position(wire_u);
			inner_parpos.add(temp);
			wire_u += wire_in_range_len / delampts_size;
		}
	}
	int inner_delam_points_sz = (int)inner_parpos.size();
	SPAposition* inner_delam_points = ACIS_NEW SPAposition[inner_delam_points_sz+1];
	for (int i = 0; i < inner_delam_points_sz; i++)
	{
		SPApar_pos temp(inner_parpos[i].x() * u_range.length() + u_range.start_pt(), inner_parpos[i].y() * v_range.length() + v_range.start_pt());
		inner_delam_points[i] = surf_in->eval_position(temp);
	}
	inner_delam_points[inner_delam_points_sz] = inner_delam_points[0];

	// Create the outer wire of the delamination shape
	EDGE* spline_edge_out;
	this->_check_outcome(api_curve_spline(delampts_size+1, outer_delam_points, NULL, NULL, spline_edge_out, TRUE, FALSE), __FILE__, __LINE__, __FUNCTION__);

	//BODY* profile_out;
	this->_check_outcome(api_make_ewire(1, &spline_edge_out, profile_out), __FILE__, __LINE__, __FUNCTION__);

	// Create the inner wire of the delamination shape
	EDGE* spline_edge_in;
	this->_check_outcome(api_curve_spline(inner_delam_points_sz + 1, inner_delam_points, NULL, NULL, spline_edge_in, TRUE, TRUE), __FILE__, __LINE__, __FUNCTION__);

	//BODY* profile_in;
	this->_check_outcome(api_make_ewire(1, &spline_edge_in, profile_in), __FILE__, __LINE__, __FUNCTION__);

	// Delete pointers that we don't need anymore
	ACIS_DELETE[] projected_points_outer;
	ACIS_DELETE[] outer_delam_points;
	ACIS_DELETE[] inner_delam_points;
	ACIS_DELETE[] outer_parpos;
	inner_parpos.clear();
}

LayerSurface* ACISModelBuilder::find_closest_side(Layer *layer_in, delamo::TPoint3<double>& point_in)
{
	// Convert input point to SPAposition
	SPAposition in_point(point_in.data_ptr());

	// Find all side LayerSurface - closest distance relations w.r.t. the input point and store in a temporary list
	delamo::List<LSDistanceRel> lsrel_list;
	for (auto &lb_in : *layer_in)
	{
		for (auto &ls_in : *lb_in)
		{
			if (Direction::SIDE == ls_in->direction())
			{
				SPAposition closest_pos; double distance;
				this->_check_outcome(api_entity_point_distance(ls_in->face(), in_point, closest_pos, distance), __FILE__, __LINE__, __FUNCTION__);
				LSDistanceRel temp;	temp.ls = ls_in; temp.dist = distance;
				lsrel_list.add(temp);
			}
		}
	}

	// Find the closest side LayerSurface w.r.t. the input point
	LayerSurface* closest_ls = nullptr; double min_distance = std::numeric_limits<double>::max();
	for (auto lsrel : lsrel_list)
	{
		if (min_distance > lsrel.dist)
		{
			min_distance = lsrel.dist;
			closest_ls = lsrel.ls;
		}
	}

	// Clear the temporary list
	lsrel_list.clear();

	// Return the pointer refering to the closest LayerSurface w.r.t. the input point
	return closest_ls;
}

void ACISModelBuilder::translate_shell_edge_points(delamo::List<delamo::TPoint3<double>>& edge_point_list, delamo::List<delamo::TPoint3<double>>& edge_normal_list, double thickness, delamo::List<delamo::TPoint3<double>>& layer_point_list)
{
	for (unsigned int edgeNum = 0; edgeNum < edge_point_list.size(); edgeNum++)
	{
		delamo::TPoint3<double> point = edge_point_list[edgeNum];
		delamo::TPoint3<double> normal = edge_normal_list[edgeNum];
		double translated_x = point.x() + normal.x() * thickness;
		double translated_y = point.y() + normal.y() * thickness;
		double translated_z = point.z() + normal.z() * thickness;

		// Add normal to return list
		delamo::TPoint3<double> translated_point;
		translated_point.set_x(translated_x);
		translated_point.set_y(translated_y);
		translated_point.set_z(translated_z);
		layer_point_list.add(translated_point);
	}
}


void ACISModelBuilder::find_side_faces(Layer *layer_in, delamo::List<delamo::TPoint3<double>>& side_point_list, delamo::List<delamo::TPoint3<double>>& point_out, delamo::List<delamo::TPoint3<double>>& normal_out)
{

	for (unsigned int sideNum = 0; sideNum < side_point_list.size(); sideNum++)
	{
		delamo::TPoint3<double> side_point = side_point_list[sideNum];
		LayerSurface* closest_face = this->find_closest_side(layer_in, side_point);
		delamo::TPoint3<double> surface_point = closest_face->point_coords();
		delamo::TPoint3<double> surface_normal = closest_face->normal_coords();
		point_out.add(surface_point);
		normal_out.add(surface_normal);
	}
}
