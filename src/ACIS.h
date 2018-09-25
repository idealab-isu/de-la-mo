#ifndef ACIS_H
#define ACIS_H

// Not include max functions from Windows api
#define NOMINMAX

//ACIS Includes
#include <acis.hxx>					// Declares system wide parameters
#include <license.hxx>		    	// Licensing
#include <spa_unlock_result.hxx>	// Licensing
#include <cstrapi.hxx>				// Declares constructor APIs
#include <kernapi.hxx>				// Declares kernel APIs
#include <body.hxx>					// Declares BODY class
#include <boolapi.hxx>				// Declares boolean APIs
#include <fileinfo.hxx> 			// Declares FileInfo class
#include <spa_progress_info.hxx>	// Outcome Progress
#include <position.hxx> 			// Declares position class
#include <vector.hxx>				// Declares vector class
#include <unitvec.hxx>				// Declares unit vector class
#include <vector_utils.hxx>			// Declares distance utilites
#include <param.hxx>				// Declares parametric coordinates
#include <lists.hxx>    			// Declares ENTITY_LIST class
#include <surface.hxx>				// Declares SURFACE class
#include <sur.hxx>					// Declares surface class
#include <coverapi.hxx>				// Declares face covering APIs
#include <sweepapi.hxx>				// Declares sweep APIs
#include <stchapi.hxx>				// Declares stitching APIs
#include <mt_stitch_apis.hxx>		// Declares multi-threaded stitching APIs
#include <shl_api.hxx>				// Declares shelling APIs
#include <cover_options.hxx>		// Declares wire cover oOptions
#include <bs3surf.hxx>				// Declares bs3 spline surface class
#include <sps3srtn.hxx>				// Declares bs3 spline surface class
#include <proj_opts.hxx>			// Declares projection options
#include <intrapi.hxx>				// Declares APIs for point in face and check entity
#include <ofstapi.hxx>				// Declares APIs for offseting
#include <wire_offset_options.hxx>	// Declares offset options class
#include <at_name.hxx>				// Defines a named attribute for the Generic Attributes Component
#include <at_int.hxx>				// Defines a generic attribute that contains an integer value
#include <ga_api.hxx>				// Generic Attributes API
#include <queryapi.hxx>				// Declares interfaces related to Model Query
#include <container_utils.hxx>		// Provides std::vector like interfaces within ACIS
#include <faceutil.hxx>
#include <geom_utl.hxx>
#include <getowner.hxx>
#include <cnv2spl_opts.hxx>			// Declares Convert to Spline options class
#include <lop_api.hxx>				// Defines Local Operations API
#include <sgquery.hxx>
#include <blendapi.hxx>				// Declares Basic Blending API
#include <spline.hxx>
#include <transfrm.hxx>
#include <edge.hxx>					// Declares EDGE class
#include <curve.hxx>				// Declares CURVE class
#include <curdef.hxx>				// Declares curve class
#include <mprop.hxx>				// Defines a class (mass_prop) for evaluated physical properties
#include <box.hxx>					// Defines a bounding box (SPAbox)
#include <acistol.hxx>
#include <getbox.hxx>
#include <raytest.hxx>
#include <geometry.hxx>
#include <eulerapi.hxx>
#include "pcudef.hxx"				// Declares pcurve class
#include "pcurve.hxx"				// Declares PCURVE class
#include "bs2curve.hxx"				// Declares bs2 spline curve class
#include "sps2crtn.hxx" 			// Declares bs2 spline curve class
#include "rgbcolor.hxx" 			// Declares colors for entities
#include "rnd_api.hxx" 				// Declares rendering apis
#include "ppmeshmg.hxx" 			// Declares poly_point_mesh manager
#include "refine.hxx"				// Declares refinement class
#include "facet_options.hxx"		// Declares facet options class
#include "af_enum.hxx"				// Declares enum types
#include "af_api.hxx"				// Declares faceter API's



// ACIS Debug Includes
#include <debug.hxx>				// Declares debug routines

// LayerElements
typedef BODY* DLM_BODYP;
typedef FACE* DLM_FACEP;
typedef SPAposition DLM_POSITION;
typedef SPAunit_vector DLM_UNITVECTOR;

#endif /* ACIS_H */
