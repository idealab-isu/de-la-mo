// CPP includes
#include <iostream>
#include <iomanip>

// ModelBuilder API
#include "../Layer.h"
#include "../ACISModelBuilder.h"

// Pauses the command line messages
void pause();

// Debug FAL using a text file
void WriteFAL(std::string fileName, FaceAdjacency* fal, int fal_size);
void WritePoints(std::string fileName, delamo::List<delamo::TPoint3<double>> points_list);

using namespace delamo;
