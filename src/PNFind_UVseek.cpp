#include "PNFind_UVseek.h"


bool PNFind_UVseek::find_point(FACE* face_in, SPAtransf& transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpt_in, SPAposition& point_out, bool silence_errors)
{
	/*
	 * Point seek-and-find algorithm that works in parametric space
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

	// Use the UVSeek algorithm implemented in PNFind_ABS class
	if (this->uv_seek(face_in, transf_in, refpt_in, point_out))
		return true;

	// Couldn't find the point, return the error condition
	if (silence_errors != true)
		std::cout << "POINT FIND ERROR: Cannot find point on the face!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}

bool PNFind_UVseek::find_point_simple(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors)
{
	SPAbox face_box = get_face_box(face_in, &face_transf_in);
	return this->find_point(face_in, face_transf_in, face_box, refpoint_in, point_out, silence_errors);
}

bool PNFind_UVseek::find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out)
{
	// Return NOT IMPLEMENTED message for now
	std::cout << "POINT FIND ERROR: Midpoint find function is not implemented!" << std::endl;
	point_out = SPAposition(0.0, 0.0, 0.0);
	return false;
}
