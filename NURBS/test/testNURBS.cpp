// Copyright (c) 2018 by Adarsh Krishnamurthy et. al. and Iowa State University. 
// All rights reserved.
//
// Permission to use, copy, modify, and distribute this software and its
// documentation for non-profit use, without fee, and without written agreement is
// hereby granted, provided that the above copyright notice and the following
// two paragraphs appear in all copies of this software.
//
// IN NO EVENT SHALL IOWA STATE UNIVERSITY BE LIABLE TO ANY PARTY FOR
// DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
// OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF IOWA STATE UNIVERSITY
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// IOWA STATE UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
// INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
// ON AN "AS IS" BASIS, AND IOWA STATE UNIVERSITY HAS NO OBLIGATION TO
// PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//
// Initial version April 20 2018 - Adarsh Krishnamurthy et. al.
//

#include "PointVector.hxx"
#include "ContainerList.hxx"
#include "NURBS.hxx"


// Function prototypes
void pause();

int main(int argc, char** argv)
{
	using namespace delamo;	using _DataType = float;

	// Prepare NURBS surface
	NURBS<_DataType> mold;
	if (!mold.read_ctrlpts("NURBS_CP_Planar1.txt"))
	{
		pause();
		return EXIT_FAILURE;
	}

	int degree_u = 3;
	int degree_v = 3;
	List<_DataType> knot_vector_u = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };
	List<_DataType> knot_vector_v = { 0, 0, 0, 0, 1, 2, 3, 3, 3, 3 };
	mold.degree_u(degree_u);
	mold.degree_v(degree_v);
	mold.knotvector_u(&knot_vector_u[0], (int)knot_vector_u.size());
	mold.knotvector_v(&knot_vector_v[0], (int)knot_vector_v.size());

	// Evaluate the surface
	if (!mold.evaluate())
	{
		pause();
		return EXIT_FAILURE;
	}

	// Evaluate surface derivatives at the given u,v parametric coords
	int d = 2;
	TPoint3<_DataType>** SKL = nullptr;
	if (!mold.derivatives(_DataType(0.3), _DataType(0.2), d, SKL))
	{
		pause();
		return EXIT_FAILURE;
	}

	// Delete pointers
	for (int i = 0; i <= d; i++)
		delete[] SKL[i];
	delete[] SKL;

	// Evaluate the surface point at the given u,v parametric coords
	TPoint3<_DataType> pt1;
	if (!mold.surfpoint(_DataType(0.33), _DataType(0.33), pt1))
	{
		pause();
		return EXIT_FAILURE;
	}


	// Evaluate the surface normal at the given u,v parametric coords
	TVector3<_DataType> norm1;
	if (!mold.normal(_DataType(0.33), _DataType(0.33), norm1))
	{
		pause();
		return EXIT_FAILURE;
	}

	// Convert the normal vector to a unit vector
	norm1.normalize();

	TPoint3<_DataType>** surfpts_cont2d = mold.surfpts_2d();
	TPoint3<_DataType>* surfpts = mold.surfpts();
	TPoint3<_DataType>* ctrlpts = mold.ctrlpts();

	// Testing TPoint3 constructors
	_DataType ptlist[3] = { 1.0, 2.0, 5.0 };
	TPoint3<_DataType> test_pt(ptlist);

	// Testing List class
	List<_DataType> test_list = { 7 , 8, 9, 10, 11 };
	List<_DataType> test_list2 = { 7 , 8, 9, 10, 11 };
	test_list.push_back(_DataType(500));
	_DataType popped_var = test_list.pop_front();
	test_list.push_front(_DataType(100));
	test_list.push_back(test_list2);
	test_list.push_front(test_list2);

	// Pause the execution
	pause();

	// Return success
	return EXIT_SUCCESS;
}

void pause()
{
	std::string dummy;
	std::cout << "Press ENTER to continue..." << std::endl;
	std::getline(std::cin, dummy);
}
