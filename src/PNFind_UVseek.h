#ifndef PNFIND_UVSEEK_H
#define PNFIND_UVSEEK_H

#include "PNFind_ABS.h"


class MODELBUILDER_EXPORT PNFind_UVseek : public PNFind_ABS
{
public:
	
	PNFind_UVseek() : PNFind_ABS()
	{
		this->_mUPos = 0.01;
		this->_mVPos = 0.01;
		this->_mParPosIncrMultiplier = 0.015;
	};

	bool find_point(FACE* face_in, SPAtransf& face_transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_simple(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out);

private:
	double _mUPos;
	double _mVPos;
	double _mParPosIncrMultiplier;
};

#endif //!PNFIND_UVSEEK_H
