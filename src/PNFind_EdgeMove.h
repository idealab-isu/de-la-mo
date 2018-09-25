#ifndef PNFIND_EDGEMOVE_H
#define PNFIND_EDGEMOVE_H

#include "PNFind_ABS.h"


// Point translation precision
#define PNFIND_EDGEMOVE_PRECISION 1000

/**
 * Point and normal find algorithms -- Edge Move Method
 */
class MODELBUILDER_EXPORT PNFind_EdgeMove : public PNFind_ABS
{
public:
	
	PNFind_EdgeMove() : PNFind_ABS()
	{
		this->_mUPos = 0.01;
		this->_mVPos = 0.01;
	};

	bool find_point(FACE* face_in, SPAtransf& face_transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_simple(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out);

private:
	double _mUPos;
	double _mVPos;
};

#endif //!PNFIND_EDGEMOVE_H
