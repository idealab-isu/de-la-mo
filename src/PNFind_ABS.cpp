#include "PNFind_ABS.h"


void PNFind_ABS::find_normal(FACE* face_in, SPAposition& refpoint_in, SPAunit_vector& normal_out)
{
	this->find_normal_direct(face_in, refpoint_in, normal_out);
}

void PNFind_ABS::find_normal_direct(FACE* face_in, SPAposition& refpoint_in, SPAunit_vector& normal_out)
{
	normal_out = sg_get_face_normal(face_in, refpoint_in);
};
	
void PNFind_ABS::find_normal_surfeval(FACE* face_in, SPAposition& refpoint_in, SPAunit_vector& normal_out)
{
	// Evaluate surface to get the normal at refpoint_in
	surface* surf = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());
	normal_out = surf->point_normal(refpoint_in);
};
	
void PNFind_ABS::find_point_normal(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, SPAunit_vector& normal_out, bool silence_errors)
{
	bool res_fpt = this->find_point_simple(face_in, face_transf_in, refpoint_in, point_out, silence_errors);
	if (res_fpt)
		this->find_normal_direct(face_in, point_out, normal_out);
	else
		normal_out = SPAunit_vector(0.0, 0.0, 0.0);
};

void PNFind_ABS::error_handler(outcome& result)
{
	if (!result.ok())
	{
		error_info* err_info = result.get_error_info();
		std::cout << err_info->error_message() << std::endl;
	}
};

bool PNFind_ABS::point_in_face(SPAposition& test_point, FACE* test_face, SPAtransf& face_transformation)
{
	outcome result;
	point_face_containment cont_answer;

	result = api_point_in_face(test_point, test_face, face_transformation, cont_answer);
	this->error_handler(result);

	if (cont_answer == point_inside_face)
		return true;
	return false;
}

bool PNFind_ABS::uv_seek(FACE* face_in, SPAtransf& face_transf, SPAposition& refpt_in, SPAposition& refpt_out)
{
	// Get the surface equation of the input face
	surface* surf_in = face_in->geometry()->trans_surface(get_owner_transf(face_in), face_in->sense());

	// Get parametric position of the input reference point (reference parametric position)
	SPApar_pos ref_uv = surf_in->param(refpt_in);

	// Create an arbitrary parametric vector
	SPApar_vec vec_uv(0.0, 0.1);

	// Initial translation for the parametric position (scaling factors are 1.0 in both directions)
	SPApar_transf translate_uv(1.0, 1.0, vec_uv.du, vec_uv.dv);
	
	// Create a variable for testing translate
	SPApar_pos test_uv;

	// Scale the parametric vector (manually)
	double vec_len_max = 1.0;
	double vec_len_cur = vec_uv.dv;
	while (vec_uv.dv <= vec_len_max)
	{
		// Rotate the parametric vector
		int angle = 0;
		while (angle < 360)
		{
			// Translate the reference parametric position using the translation matrix
			test_uv = ref_uv * translate_uv;

			// Evaluate the (x,y,z) of the parametric position using the surface equation
			SPAposition test_pt = surf_in->eval_position(test_uv);

			// Check if the point is inside the face
			if (this->point_in_face(test_pt, face_in, face_transf))
			{
				refpt_out = test_pt;
				return true;
			}

			// Convert angle to radians
			double angle_rad = degrees_to_radians(angle);

			// Rotate the vector
			double du_p = (vec_uv.du * cos(angle_rad)) - (vec_uv.dv * sin(angle_rad));
			double dv_p = (vec_uv.dv * cos(angle_rad)) + (vec_uv.du * sin(angle_rad));

			// Update translation matrix with the rotated vector
			translate_uv.set_delta_u(du_p);
			translate_uv.set_delta_v(dv_p);

			// Increment angle
			angle++;
		}

		// Reset the parametric vector
		vec_uv.du = 0.0;
		vec_len_cur += 0.1;
		vec_uv.dv = vec_len_cur;
	}

	// Print error
	std::cout << "UVSEEK ERROR: Cannot find point on the face!" << std::endl;
	return false;
}
