#include "APIConfig.h"
#include "modelbuilder_export.h"


/**
 * \brief Reads the points from a CSV file outputs as a Geometry3 pointer array.
 *
 * \param file_name CSV file containing the point data
 * \param ptsarr Array of points (OUTPUT)
 * \param ptsarr_size Size of the points array (OUTPUT)
 */
void read_csv_file(const char* file_name, delamo::TPoint3<double>*& ptsarr, int& ptsarr_size);

/**
 * \brief Flips the input point array upside down.
 *
 * \param[in] ptsin input point array to be flipped upside down
 * \param[in] ptsin_size size of the input point array
 * \param[out] ptsout output point array
 */
void point_array_flip(delamo::TPoint3<double>* ptsin, int ptsin_size, delamo::TPoint3<double>*& ptsout);

/**
 * \brief Reads ModelBuilder license key
 *
 * \param file_name file containing the license key
 * \param unlock_str license key (OUTPUT)
 * \return license key read status; on error FALSE, otherwise TRUE
 */
bool MODELBUILDER_EXPORT read_license_file(const char* file_name, char*& unlock_str);

/**
 * \brief Finds a point inside the polygon.
 *
 * \param[in] ptsarr array of points that build up the polygon
 * \param[in] ptsarr_size size of the points array
 * \param[out] point_inside point inside the polygon
 */
void find_point_inside_polygon(delamo::TPoint3<double>* ptsarr, int ptsarr_size, delamo::TPoint3<double>& point_inside);

/**
 * \brief Checks if the point inside the polygon using the winding number method
 *
 * @see: http://geomalgorithms.com/a03-_inclusion.html
 * \param[in] pt_check point to be checked
 * \param[in] ptsarr array of points that build up the polygon
 * \param[in] ptsarr_size size of the points array
 * \return the winding number, =0 when the point is outside
 */
int wn_pnpoly(delamo::TPoint3<double>& pt_check, delamo::TPoint3<double>* ptsarr, int ptsarr_size);

/**
 * \brief Checks if the point is left/on/right of an infinite line.
 *
 * @see: http://geomalgorithms.com/a01-_area.html
 * \param P0 first point that defines the line
 * \param P1 second point that defines the line
 * \param Pcheck point to be checked
 * \return >0 if the point is on the left, =0 if the point is on the line, <0 if the point is on the right
 */
double is_left(delamo::TPoint3<double>& P0, delamo::TPoint3<double>& P1, delamo::TPoint3<double>& Pcheck);
