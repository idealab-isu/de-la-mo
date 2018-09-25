#include "testcase_includes.h"



void pause()
{
	std::string dummy;
	std::cout << "Press ENTER to continue..." << std::endl;
	std::getline(std::cin, dummy);
}

void WriteFAL(std::string fileName, FaceAdjacency* fal, int fal_size)
{
	std::ofstream outFile;
	outFile.open(fileName, std::ios::out | std::ios::app);

	for (int i = 0; i < fal_size; i++)
	{
		outFile << std::setprecision(3) << fal[i].point1.x() << " " << fal[i].point1.y() << " " << fal[i].point1.z() << " | ";
		outFile << std::setprecision(3) << fal[i].point2.x() << " " << fal[i].point2.y() << " " << fal[i].point2.z() << " | ";
		outFile << std::setprecision(3) << fal[i].vector1.x() << " " << fal[i].vector1.y() << " " << fal[i].vector1.z() << " | ";
		outFile << std::setprecision(3) << fal[i].vector1.x() << " " << fal[i].vector2.y() << " " << fal[i].vector2.z() << " | ";
		outFile << fal[i].name1 << " ";
		outFile << fal[i].name2 << " ";
		
		std::string bctype_str;
		switch (fal[i].bcType)
		{
		case DelaminationType::NODELAM: bctype_str = "No Delamination"; break;
		case DelaminationType::NOMODEL: bctype_str = "No Model"; break;
		case DelaminationType::CONTACT: bctype_str = "Contact"; break;
		case DelaminationType::COHESIVE: bctype_str = "Cohesive"; break;
		case DelaminationType::TIE: bctype_str = "Tie"; break;
		  //case DelaminationType::CONTACT_OR_NOMODEL: bctype_str = "Contact or No Model"; break;
		default: bctype_str = "Undefined Type";
		}

		outFile << bctype_str << std::endl;
	}

	// Add separator
	outFile << std::endl;

	outFile.close();
}

void WritePoints(std::string fileName, delamo::List<delamo::TPoint3<double>> points_list)
{
	std::ofstream outFile;
	outFile.open(fileName, std::ios::out | std::ios::app);

	for (auto point : points_list)
	{
		outFile << std::setprecision(3) << point.x() << " " << point.y() << " " << point.z() << std::endl;
	}

	// Add separator
	outFile << std::endl;

	outFile.close();
}
