#include "mb_utilities.h"


void read_csv_file(const char* file_name, delamo::TPoint3<double>*& ptsarr, int& ptsarr_size)
{
	// Read the file
	std::ifstream input;
	input.open(file_name, std::ios::in);

	// Check that the file is opened without any issues
	if (!input.is_open())
	{
		if (MODELBUILDER_DEBUG_LEVEL >= MODELBUILDER_DEBUG_ERROR)
			std::cout << "ERROR: Unable to open CSV file: " << file_name << std::endl;
		throw std::runtime_error("CAD Model Builder: Operation failed!");
	}

	// Use a container to store points which are read from the CSV file
	delamo::List< delamo::TPoint3<double> > output;
	std::string line;
	std::getline(input, line); // Read first line in the CSV file

	while (std::getline(input, line))
	{
		// If the line is empty, skip it.
		if (line.empty())
			continue;

		// Now, we are on a single coordinate
		char delim = ','; // This is CSV
		unsigned long long i = 0;
		auto pos = line.find(delim);
		double x = std::stod(line.substr(i, pos - i));
		i = ++pos;
		pos = line.find(delim, pos);
		double y = std::stod(line.substr(i, pos - i));
		i = ++pos;
		pos = line.find(delim, pos);
		double z = std::stod(line.substr(i, pos - i));

		// Set point values
		delamo::TPoint3<double> point;
		point.x(x);
		point.y(y);
		point.z(z);

		// Add it to the container
		output.push_back(point);
	}

	// Get the size of the array (this is required for new() command)
	ptsarr_size = (int)output.size();
	ptsarr = new delamo::TPoint3<double>[ptsarr_size];
	// Copy the contents of the container into a pointer array
	std::copy(output.begin(), output.end(), ptsarr);

	// We don't need the container anymore, so empty its contents
	output.clear();

	// Close file handle
	input.close();
}

void point_array_flip(delamo::TPoint3<double>* ptsin, int ptsin_size, delamo::TPoint3<double>*& ptsout)
{
	ptsout = new delamo::TPoint3<double>[ptsin_size];
	int idx = ptsin_size - 1;
	for (int i = 0; i < ptsin_size; i++)
	{
		ptsout[i] = ptsin[idx];
		idx--;
	}
}

bool read_license_file(const char* file_name, char*& unlock_str)
{
	std::ifstream license_file(file_name);
	if (!license_file.good())
	{
		std::cout << "ERROR: No license file!" << std::endl;
		return false;
	}

	std::string keydata;
	std::string line;
	while (std::getline(license_file, line))
	{
		// If the line is empty, skip it.
		if (line.empty())
			continue;

		// Clean white space
		// Before editing std::isspace, see: https://stackoverflow.com/a/8364701
		line.erase(std::remove_if(line.begin(), line.end(), static_cast<int(&)(int)>(std::isspace)), line.end());

		size_t found;

		// Clean double quotes
		found = line.find("\"");
		while (found != std::string::npos)
		{
			line.erase(found, 1);
			found = line.find("\"");
		}

		// Clean colons
		found = line.find(";");
		while (found != std::string::npos)
		{
			line.erase(found, 1);
			found = line.find(";");
		}

		// Append to keydata string object
		std::stringstream ss(line);
		keydata.append(ss.str());
	}

	// Copy the license key to the output variable 
	unlock_str = new char[keydata.size()+1];
	std::copy(keydata.c_str(), keydata.c_str()+keydata.size(), unlock_str);
	// C-style strings need a terminating character
	unlock_str[keydata.size()] = '\0';
	
	license_file.close();
	return true;
}

void find_point_inside_polygon(delamo::TPoint3<double>* ptsarr, int ptsarr_size, delamo::TPoint3<double>& point_inside)
{
	// Find the minimum (Pmin) and maximum (Pmax) points using the x-coordinate
	delamo::TPoint3<double> pt_min = ptsarr[0];
	delamo::TPoint3<double> pt_max = ptsarr[ptsarr_size - 1];
	for (int i = 0; i < ptsarr_size; i++)
	{
		// Find maximum
		if (pt_max.x() > ptsarr[i].x())
			pt_max = ptsarr[i];

		// Find minimum
		if (pt_min.x() < ptsarr[i].x())
			pt_min = ptsarr[i];
	}

	// Find the middle point between Pmin and Pmax which would act as a starting point
	delamo::TPoint3<double> pt_mid((pt_max.x() + pt_min.x()) / 2, (pt_max.y() + pt_min.y()) / 2, (pt_max.z() + pt_min.z()) / 2);
	delamo::TPoint3<double> pt_mid_initial;

	// Try to find point inside the polygon
	int trials = 10;
	int trials_pmin = 0;
	int trials_pmax = 0;
	while (trials_pmin < trials)
	{
		int wn = wn_pnpoly(pt_mid, ptsarr, ptsarr_size);
		if (wn != 0)
		{
			point_inside = pt_mid;
			return;
		}
		else
		{
			pt_mid = delamo::TPoint3<double>((pt_mid.x() + pt_min.x()) / 2, (pt_mid.y() + pt_min.y()) / 2, (pt_mid.z() + pt_min.z()) / 2);
			++trials_pmin;
		}
	}

	pt_mid = pt_mid_initial;
	while (trials_pmax < trials)
	{
		int wn = wn_pnpoly(pt_mid, ptsarr, ptsarr_size);
		if (wn != 0)
		{
			point_inside = pt_mid;
			return;
		}
		else
		{
			pt_mid = delamo::TPoint3<double>((pt_mid.x() + pt_max.x()) / 2, (pt_mid.y() + pt_max.y()) / 2, (pt_mid.z() + pt_max.z()) / 2);
			++trials_pmax;
		}
	}

	// Up to this point, the function should have found a point. If not, display an error message and throw an exception
	std::cout << "ERROR: Cannot automatically find a point inside the polygon!" << std::endl;
	throw std::runtime_error("CAD Model Builder: Operation failed!");
	
	return;
}

int wn_pnpoly(delamo::TPoint3<double>& pt_check, delamo::TPoint3<double>* ptsarr, int ptsarr_size)
{
	// The winding number counter
	int wn = 0;

	// Loop through all polygon points
	for (int i = 0; i < ptsarr_size; i++)
	{
		if (ptsarr[i].y() <= pt_check.y())
		{
			if (ptsarr[i + 1].y() > pt_check.y())
			{
				if (is_left(ptsarr[i], ptsarr[i + 1], pt_check) > 0)
					++wn;
			}
		}
		else
		{
			if (ptsarr[i + 1].y() <= pt_check.y())
			{
				if (is_left(ptsarr[i], ptsarr[i + 1], pt_check) < 0)
					--wn;
			}
		}
	}
	return wn;
}

double is_left(delamo::TPoint3<double>& P0, delamo::TPoint3<double>& P1, delamo::TPoint3<double>& Pcheck)
{
	return (((P1.x() - P0.x()) * (Pcheck.y() - P0.y())) - ((Pcheck.x() - P0.x()) * (P1.y() - P0.y())));
}
