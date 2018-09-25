#ifndef PNFIND_BBOX_H
#define PNFIND_BBOX_H

#include "PNFind_ABS.h"


// Point translation precision
#define PNFIND_BBOX_PRECISION 25
// Don't change the value below. If you really need to, check int divisions (e.g. int val = 5/2) carefully
#define PNFIND_BOX_ITERMAX 40

/**
 * Point and normal find algorithms -- Bounding Box Method
 */
class MODELBUILDER_EXPORT PNFind_BBox : public PNFind_ABS
{
public:
	PNFind_BBox();
	bool find_point(FACE* face_in, SPAtransf& face_transf_in, SPAbox& cohesive_face_bbox, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_simple(FACE* face_in, SPAtransf& face_transf_in, SPAposition& refpoint_in, SPAposition& point_out, bool silence_errors = false);
	bool find_point_mid(FACE* face_in, SPAtransf& face_transf_in, SPAposition& point_out);

private:
	double _mScanAngle;
};

#endif //!PNFIND_BBOX_H
