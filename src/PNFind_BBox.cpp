#include "PNFind_BBox.h"


PNFind_BBox::PNFind_BBox() : PNFind_ABS()
{
	this->_mScanAngle = 5.0;
}

bool PNFind_BBox::find_point(FACE* face_in, SPAtransf& transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpt_in, SPAposition& point_out, bool silence_errors)
{
	/*
	 * The bounding box method -- 07/17/2017 (O.R.B.)
	 */

	outcome result;

	// If the reference point is inside the face...
	if (this->point_in_face(refpt_in, face_in, transf_in))
	{
		// Display some debugging messages
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
		{
			std::cout << "DEBUG: Found the input ref point" << " ("
				<< refpt_in.x() << ", "
				<< refpt_in.y() << ", "
				<< refpt_in.z() << ") "
				<< "inside the face."
				<< std::endl;
		}

		// And, we are all set!
		point_out = refpt_in;
		return true;
	}

	// Generate FACE BBox
	SPAbox face_box = get_face_box(face_in, &transf_in);

	// Create the BBox diagonal
	SPAunit_vector line_direction;
	SPAposition line_point;
	SPAposition pts[3];
	pts[0] = face_box.low();
	pts[1] = face_box.mid();
	pts[2] = face_box.high();

	if (!get_line_from_points(3, pts, line_point, line_direction))
	{
		// This might need a little bit brushing
		std::cout << "ERROR: Critial geometry problem! Check your model." << std::endl;
		return false;
	}

	// Third, try to find the intersection of bbox diagonal and the face in consideration
	SPAposition lower_pt = face_box.low();
	SPAposition upper_pt = face_box.high();

	// Move the lower point to the negative side on the diagonal
	SPAtransf lower_pt_transf = translate_transf(-line_direction);
	SPAposition ray_root_pt = lower_pt * lower_pt_transf;

	// Ray testing
	hit* hitlist;
	ENTITY* entlist[1];
	entlist[0] = (ENTITY*)face_in;

	result = api_raytest_ents(ray_root_pt, line_direction, SPAresabs, 2, 1, entlist, hitlist);
	this->error_handler(result);

	SPAposition test_pt;
	if (hitlist != NULL)
	{
		// Move the root point to the ray-entity intersection 
		SPAtransf root_pt_translate = translate_transf(line_direction * hitlist->ray_param);
		test_pt = ray_root_pt * root_pt_translate;
	}
	else
	{
		// Find the curvature vector of the curve composing the face
		double distance;
		param_info ent_info;
		result = api_entity_point_distance(face_in, refpt_in, test_pt, distance, ent_info);
		this->error_handler(result);

		EDGE* ent_edge = NULL;
		VERTEX* ent_vert = NULL;
		bool non_face = false;
		switch (ent_info.entity_type())
		{
		case ent_is_vertex:
			ent_vert = (VERTEX*)ent_info.entity();
			// Get the owner of this vertex, exactly the 1st edge which owns this vertex
			// "owner()" returns only 1 edge; @see Vertex: Implementation, https://doc.spatial.com/get_doc_page/articles/v/e/r/Vertex.html
			ent_edge = (EDGE*)ent_vert->owner();
			// Try to find another point on the edge
			test_pt = edge_mid_pos(ent_edge);
			non_face = true;
			break;
		case ent_is_edge:
			ent_edge = (EDGE*)ent_info.entity();
			non_face = true;
			break;
		case ent_is_face:
			// This even shouldn't be happening, but just in case.
			test_pt = refpt_in;
			break;
		default:
			// You shouldn't see this message unless if you have a serious problem in your model or shape
			std::cout << "CRITICAL ERROR: Something is wrong with the point finding function!" << std::endl;
		}

		if (non_face)
		{
			const curve& curve_edge = ent_edge->geometry()->equation();
			SPAvector curvature_vec = curve_edge.point_curvature(test_pt);
			// If curvature vector is zero, the algorithm must have hit to some planar edges
			if (curvature_vec.is_zero())
			{
				// Use the cohesive face's bounding box to find the point inside the face
				SPAunit_vector cfc_line_direction;
				SPAposition cfc_line_point;
				SPAposition cfc_pts[3];
				cfc_pts[0] = cohesive_face_bbox.low();
				cfc_pts[1] = cohesive_face_bbox.mid();
				cfc_pts[2] = cohesive_face_bbox.high();
				if (!get_line_from_points(3, cfc_pts, cfc_line_point, cfc_line_direction))
				{
					// This might need a little bit brushing
					std::cout << "ERROR: Critial geometry problem! Check your model." << std::endl;
					return false;
				}

				SPAposition cfc_lower_pt = cohesive_face_bbox.low();
				SPAposition cfc_upper_pt = cohesive_face_bbox.high();
				SPAtransf cfc_lower_pt_transf = translate_transf(-cfc_line_direction);
				SPAposition cfc_ray_root_pt = cfc_lower_pt * cfc_lower_pt_transf;

				SPAtransf cfc_rotate;
				cfc_rotate = rotate_transf(this->_mScanAngle, SPAvector(0, 0, 1));
				int angle = 0;
				while (angle < 360)
				{
					result = api_raytest_ents(cfc_ray_root_pt, cfc_line_direction, SPAresabs, 2, 1, entlist, hitlist);
					this->error_handler(result);
					if (hitlist != NULL)
					{
						// Move the root point to the ray-entity intersection 
						SPAtransf cfc_root_pt_translate = translate_transf(cfc_line_direction * hitlist->ray_param);
						test_pt = cfc_ray_root_pt * cfc_root_pt_translate;
						// Update point translation vector
						line_direction = cfc_line_direction;
						break;
					}
					else
					{
						cfc_line_direction = cfc_line_direction * cfc_rotate;
						angle++;
					}
				}

				if (angle == 360)
				{
					std::cout << "POINT FIND ERROR: Cannot find point on the face!" << std::endl;
					point_out = SPAposition(0.0, 0.0, 0.0);
					return false;
				}
			}
			else
			{
				// Set the new direction as the unit normal vector
				line_direction = normalise(curvature_vec);
			}
		}

	}
	
	// Move point along the diagonal
	int counter = 0;
	while (!is_equal(test_pt, upper_pt))
	{
		// Check the test point
		if (this->point_in_face(test_pt, face_in, transf_in))
		{
			// Display some debugging messages
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
			{
				std::cout << "DEBUG: Found the point" << " ("
					<< test_pt.x() << ", "
					<< test_pt.y() << ", "
					<< test_pt.z() << ") "
					<< "inside the face."
					<< std::endl;
			}

			// Set the output point
			point_out = test_pt;

			// And, we are done!
			return true;
		}
		else
		{
			SPAtransf pt_move_transf = translate_transf(line_direction / PNFIND_BBOX_PRECISION);

			// Force the algorithm a little bit
			if (counter == int(PNFIND_BOX_ITERMAX / 4) || counter == int(PNFIND_BOX_ITERMAX / 2) || counter == int(3*PNFIND_BOX_ITERMAX / 4))
			{
				// Display some debugging messages
				if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
				{
					std::cout << "POINT FIND DEBUG: Trying to find the closest point to face..." << std::endl;
				}

				SPAposition ref_pt = test_pt;

				result = api_find_cls_ptto_face(ref_pt, face_in, test_pt);
				this->error_handler(result);

				SPAposition ref_pt2;
				this->uv_seek(face_in, transf_in, test_pt, ref_pt2);
				test_pt = ref_pt2;
			}
			else
			{
				test_pt = test_pt * pt_move_transf;
			}

			// Too many trials, stop it!
			if (counter > PNFIND_BOX_ITERMAX)
				break;

			// Increment the trial counter
			++counter;
		}
	}

	// The function should have found a point during the traversal on the bb diagonal line
	if (silence_errors != true)
		std::cout << "Cannot find point on the face!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}

bool PNFind_BBox::find_point_simple(FACE* face_in, SPAtransf& transf_in, SPAposition& refpt_in, SPAposition& point_out, bool silence_errors)
{
	/*
	 * The bounding box algorithm -- simplified for non-delam faces (O.R.B.)
	 */

	outcome result;

	// If the reference point is inside the face...
	if (this->point_in_face(refpt_in, face_in, transf_in))
	{
		// Display some debugging messages
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
		{
			std::cout << "DEBUG: Found the input ref point" << " ("
				<< refpt_in.x() << ", "
				<< refpt_in.y() << ", "
				<< refpt_in.z() << ") "
				<< "inside the face."
				<< std::endl;
		}

		// Set the output point
		point_out = refpt_in;

		// And, we are done!
		return true;
	}

	// We have got no luck from the ref point. Try to find another point using face bounding box
	SPAbox face_box = get_face_box(face_in, &transf_in);

	// Create the BBox diagonal
	SPAunit_vector line_direction;
	SPAposition line_point;
	SPAposition pts[3];
	pts[0] = face_box.low();
	pts[1] = face_box.mid();
	pts[2] = face_box.high();

	if (!get_line_from_points(3, pts, line_point, line_direction))
	{
		// This might need a little bit brushing
		std::cout << "ERROR: Critial geometry problem! Check your model." << std::endl;
		return false;
	}

	SPAposition test_pt = face_in->bound()->low();
	SPAposition upper_pt = face_in->bound()->high();

	// Move lower bound point along the diagonal
	int counter = 0;
	while (!is_equal(test_pt, upper_pt))
	{
		// If the reference point is inside the face...
		if (this->point_in_face(test_pt, face_in, transf_in))
		{
			// Display some debugging messages
			if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
			{
				std::cout << "DEBUG: Found the point" << " ("
					<< refpt_in.x() << ", "
					<< refpt_in.y() << ", "
					<< refpt_in.z() << ") "
					<< "inside the face."
					<< std::endl;
			}

			// Set the output point
			point_out = test_pt;

			// And, we are done!
			return true;
		}
		else
		{
			SPAtransf pt_move_transf = translate_transf(line_direction / PNFIND_BBOX_PRECISION);

			// Force the algorithm a little bit
			if (counter == int(PNFIND_BOX_ITERMAX / 20) || counter == int(PNFIND_BOX_ITERMAX / 10) || counter == int(2 * PNFIND_BOX_ITERMAX / 10))
			{
				// Display some debugging messages
				if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
				{
					std::cout << "POINT FIND DEBUG: Trying to find the closest point to face..." << std::endl;
				}

				SPAposition ref_pt = test_pt;

				result = api_find_cls_ptto_face(ref_pt, face_in, test_pt);
				this->error_handler(result);
			}
			else
			{
				test_pt = test_pt * pt_move_transf;
			}

			// Too many trials, stop it!
			if (counter > (PNFIND_BOX_ITERMAX / 4))
				break;

			// Increment the trial counter
			++counter;
		}
	}

	// The function should have found a point during the traversal on the bb diagonal line
	if (silence_errors != true)
		std::cout << "Cannot find point on the face!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}

bool PNFind_BBox::find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out)
{
	outcome result;

	// Generate FACE BBox
	SPAbox face_box = get_face_box(face_in, &face_transf_in);

	// Get the mid point of the diagonal
	SPAposition mid_pt = face_box.mid();

	// If the mid point is inside the face...
	if (this->point_in_face(mid_pt, face_in, face_transf_in))
	{
		// Display some debugging messages
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
		{
			std::cout << "DEBUG: Found the mid point" << " ("
				<< mid_pt.x() << ", "
				<< mid_pt.y() << ", "
				<< mid_pt.z() << ") "
				<< "inside the face."
				<< std::endl;
		}

		// And, we are all set!
		point_out = mid_pt;
		return true;
	}

	SPAposition test_pt;
	result = api_find_cls_ptto_face(mid_pt, face_in, test_pt);
	this->error_handler(result);

	if (this->point_in_face(test_pt, face_in, face_transf_in))
	{
		// Display some debugging messages
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_DEBUG)
		{
			std::cout << "DEBUG: Found the mid point" << " ("
				<< mid_pt.x() << ", "
				<< mid_pt.y() << ", "
				<< mid_pt.z() << ") "
				<< "inside the face."
				<< std::endl;
		}

		// And, we are all set!
		point_out = test_pt;
		return true;
	}

	//std::cout << "Cannot find mid point on the face!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}
