#include "PNFind_EdgeMove.h"


bool PNFind_EdgeMove::find_point(FACE* face_in, SPAtransf& transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpt_in, SPAposition& point_out, bool silence_errors)
{
	outcome result;

	/*
	 * STEP 1: Check if the reference point is inside
	 */

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

	/*
	 * STEP 2: Find the closest point to the FACE and move a little bit to find inside point
	 */

	// We would like to find the closest entity (i.e. edge) to our reference point and a point on this entity
	SPAposition closest_pos; double distance; param_info ent_info;
	result = api_entity_point_distance(face_in, refpt_in, closest_pos, distance, ent_info);
	this->error_handler(result);

	// If somehow the closest point is inside the face, then return it
	if (ent_is_face == ent_info.entity_type())
	{
		point_out = closest_pos;
		return true;
	}

	EDGE* ent_edge = NULL;
	VERTEX* ent_vert = NULL;
	switch (ent_info.entity_type())
	{
	case ent_is_vertex:
		ent_vert = (VERTEX*)ent_info.entity();
		// Get the owner of this vertex, exactly the 1st edge which owns this vertex
		// "owner()" returns only 1 edge; @see Vertex: Implementation, https://doc.spatial.com/get_doc_page/articles/v/e/r/Vertex.html
		ent_edge = (EDGE*)ent_vert->owner();
		// Try to find another point on the edge
		closest_pos = edge_mid_pos(ent_edge);
		break;
	case ent_is_edge:
		ent_edge = (EDGE*)ent_info.entity();
		break;
	default:
		// You shouldn't see this message unless if you have a serious problem in your model or shape
		std::cout << "POINT FIND CRITICAL ERROR: Something is wrong with the point finding function!" << std::endl;
		return false;
	}

	const curve& curve_edge = ent_edge->geometry()->equation();
	SPAvector curvature_vec = curve_edge.point_curvature(closest_pos);

	// If the curvature vector is not zero, it means that we will be able to move the point inside the FACE
	if (!curvature_vec.is_zero())
	{
		SPAtransf pt_translate = translate_transf(curvature_vec / PNFIND_EDGEMOVE_PRECISION);
		point_out = closest_pos * pt_translate;
		// TO-DO: Might need to add some extra checks here
		return true;
	}

	// TO-DO: Should implement the condition when curvature vector is zero

	// Couldn't find the point, return the error condition
	if (silence_errors != true)
		std::cout << "POINT FIND ERROR: Cannot find point on the face!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}

bool PNFind_EdgeMove::find_point_simple(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors)
{
	SPAbox face_box = get_face_box(face_in, &face_transf_in);
	return this->find_point(face_in, face_transf_in, face_box, refpoint_in, point_out, silence_errors);
}

bool PNFind_EdgeMove::find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out)
{
	// Return NOT IMPLEMENTED message for now
	std::cout << "POINT FIND ERROR: Midpoint find function is not implemented!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}
