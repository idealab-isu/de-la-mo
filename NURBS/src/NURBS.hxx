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

#ifndef NURBS_HXX
#define NURBS_HXX

// CPP includes
#include <cstddef>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

// Include template classes
#include "PointVector.hxx"
#include "ContainerList.hxx"


namespace delamo
{
	template <typename T>
	class NURBS
	{
	public:
		using value_type = T; /**< Default value type for the NURBS class */

		/**
		* @brief Default constructor.
		*/
		NURBS()
		{
			this->init_vars();
		}

		/**
		* @brief Copy constructor.
		* @param rhs object to be copied
		*/
		NURBS(const NURBS& rhs)
		{
			this->init_vars();
			this->copy_vars(rhs);
		}

		/**
		* @brief Default destructor.
		*/
		~NURBS()
		{
			this->delete_vars();
		}

		/**
		* @brief Copy assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		NURBS<T>& operator=(const NURBS<T>& rhs)
		{
			// Check for self assignment
			if (this != &rhs)
			{
				this->copy_vars(rhs);
			}
			return *this;
		}

		/**
		* @brief Returns the degree of the knot vector u.
		* @return degree of the knot vector
		*/
		int degree_u()
		{
			return this->_mDegree_U;
		}

		/**
		* @brief Sets the degree of the knot vector u.
		* @param degree degree of the knot vector
		* @return FALSE if any errors, TRUE otherwise 
		*/
		bool degree_u(int degree)
		{
			// Check for logical errors
			if (degree <= 0)
			{
				std::cerr << "NURBS ERROR: Degree U cannot be zero or less" << std::endl;
				return false;
			}

			this->_mDegree_U = degree;

			return true;
		}

		/**
		* @brief Returns the degree of the knot vector u.
		* @return degree of the knot vector
		*/
		int degree_v()
		{
			return this->_mDegree_V;
		}

		/**
		* @brief Sets the degree of the knot vector v.
		* @param degree degree of the knot vector
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool degree_v(int degree)
		{
			// Check for logical errors
			if (degree <= 0)
			{
				std::cerr << "NURBS ERROR: Degree V cannot be zero or less" << std::endl;
				return false;
			}

			this->_mDegree_V = degree;

			return true;
		}

		/**
		* @brief Sets the control points.
		* @param ctrlpts 1D control point array
		* @param ctrlpts_u_len number of control points in the u-dimension
		* @param ctrlpts_v_len number of control points in the v-dimension
		*/
		void ctrlpts(TPoint3<T>* ctrlpts, int ctrlpts_u_len, int ctrlpts_v_len)
		{
			// Need to fill 2D control points
			if (this->_pCtrlPts2D != nullptr)
			{
				for (int i = 0; i < this->_mNumCtrlPts_U; i++)
					delete[] this->_pCtrlPts2D[i];
				delete[] this->_pCtrlPts2D;
				this->_pCtrlPts2D = nullptr;
			}

			// Set new sizes for ctrlpts u-v
			this->_mNumCtrlPts_U = ctrlpts_u_len;
			this->_mNumCtrlPts_V = ctrlpts_v_len;

			// Check if the pointer is empty. If not, empty it.
			if (this->_pCtrlPts != nullptr)
			{
				delete[] this->_pCtrlPts;
				this->_pCtrlPts = nullptr;
			}

			// Copy 1D points
			int num_ctrlpts = ctrlpts_u_len * ctrlpts_v_len;
			this->_pCtrlPts = new TPoint3<T>[num_ctrlpts];
			std::copy(ctrlpts, ctrlpts + num_ctrlpts, this->_pCtrlPts);

			// Copy 2D points
			this->_pCtrlPts2D = new TPoint3<T>*[this->_mNumCtrlPts_U];
			for (int i = 0; i < this->_mNumCtrlPts_U; i++)
				this->_pCtrlPts2D[i] = new TPoint3<T>[this->_mNumCtrlPts_V];

			// Because we have 1D array of control points, it needs to be converted to 2D array
			this->init_ctrlpts2d(ctrlpts_u_len, ctrlpts_v_len);
			for (int i = 0; i < this->_mNumCtrlPts_U; i++)
			{
				for (int t = 0; t < this->_mNumCtrlPts_V; t++)
				{
					this->_pCtrlPts2D[i][t] = this->_pCtrlPts[i + (t * this->_mNumCtrlPts_U)];
				}
			}
		}

		/**
		* @brief Returns the control points as an 1D array.
		* @return the control points
		*/
		TPoint3<T>* ctrlpts()
		{
			return this->_pCtrlPts;
		}

		/**
		* @brief Returns the control points as a 2D array.
		* @return the control points in [u][v] array format
		*/
		TPoint3<T>** ctrlpts_2d()
		{
			return this->_pCtrlPts2D;
		}

		/**
		* @brief Returns the number of control points in the u-direction.
		* @return number of control points
		*/
		int ctrlpts_u_len()
		{
			return this->_mNumCtrlPts_U;
		}

		/**
		* @brief Returns the number of control points in the v-direction.
		* @return number of control points
		*/
		int ctrlpts_v_len()
		{
			return this->_mNumCtrlPts_V;
		}

		/**
		* @brief Returns the total number of control points in the u-v space.
		* @return number of control points
		*/
		int ctrlpts_len()
		{
			return (this->_mNumCtrlPts_U * this->_mNumCtrlPts_V);
		}

		/**
		* @brief Sets the knot vector u.
		* @param knot_vector the knot vector in C++ STL vector format
		*/
		void knotvector_u(List<T>& knot_vector)
		{
			// Check if the pointer is empty. If not, empty it.
			if (this->_pKnotVector_U != nullptr)
			{
				delete[] this->_pKnotVector_U;
				this->_pKnotVector_U = nullptr;
			}
			// Set size of the knot vector
			this->_mNumKnotVector_U = (int)knot_vector.size();
			// Set the knot vector itself
			this->_pKnotVector_U = new T[this->_mNumKnotVector_U];
			this->normalize(&knot_vector[0], this->_mNumKnotVector_U, this->_pKnotVector_U);
		}

		/**
		* @brief Sets the knot vector u.
		* @param knotvector the knot vector
		* @param num_knotvector number of elements in the knot vector
		*/
		void knotvector_u(T* knotvector, int num_knotvector)
		{
			// Check if the pointer is empty. If not, empty it.
			if (this->_pKnotVector_U != nullptr)
			{
				delete[] this->_pKnotVector_U;
				this->_pKnotVector_U = nullptr;
			}
			this->_mNumKnotVector_U = num_knotvector;
			this->_pKnotVector_U = new T[this->_mNumKnotVector_U];
			this->normalize(knotvector, num_knotvector, this->_pKnotVector_U);
		}

		/**
		* @brief Returns the knot vector u.
		* @return the knot vector pointer array
		*/
		T* knotvector_u()
		{
			return this->_pKnotVector_U;
		}

		/**
		* @brief Returns the number of knots in the knot vector u.
		* @return number of knots
		*/
		int knotvector_u_len()
		{
			return this->_mNumKnotVector_U;
		}

		/**
		* @brief Sets the knot vector v.
		* @param knot_vector the knot vector in C++ STL vector format
		*/
		void knotvector_v(List<T>& knot_vector)
		{
			// Check if the pointer is empty. If not, empty it.
			if (this->_pKnotVector_V != nullptr)
			{
				delete[] this->_pKnotVector_V;
				this->_pKnotVector_V = nullptr;
			}
			// Set size of the knot vector
			this->_mNumKnotVector_V = (int)knot_vector.size();
			// Set the knot vector itself
			this->_pKnotVector_V = new T[this->_mNumKnotVector_V];
			this->normalize(&knot_vector[0], this->_mNumKnotVector_V, this->_pKnotVector_V);
		}

		/**
		* @brief Sets the knot vector v.
		* @param knotvector the knot vector
		* @param num_knotvector number of elements in the knot vector
		*/
		void knotvector_v(T* knotvector, int num_knotvector)
		{
			// Check if the pointer is empty. If not, empty it.
			if (this->_pKnotVector_V != nullptr)
			{
				delete[] this->_pKnotVector_V;
				this->_pKnotVector_V = nullptr;
			}
			this->_mNumKnotVector_V = num_knotvector;
			this->_pKnotVector_V = new T[this->_mNumKnotVector_V];
			this->normalize(knotvector, num_knotvector, this->_pKnotVector_V);
		}

		/**
		* @brief Returns the knot vector V.
		* @return the knot vector pointer array
		*/
		T* knotvector_v()
		{
			return this->_pKnotVector_V;
		}

		/**
		* @brief Returns the number of knots in the knot vector v.
		* @return number of knots
		*/
		int knotvector_v_len()
		{
			return this->_mNumKnotVector_V;
		}

		/**
		* @brief Sets the weights vector from a C++ STL vector.
		* @param weights the weights vector
		*/
		void weights(List<T>& weights)
		{
			// Check if the pointer is empty. If not, empty it.
			if (this->_pWeights != nullptr)
			{
				delete[] this->_pWeights;
				this->_pWeights = nullptr;
			}
			// Set size of the weights vector
			int num_weights = (int)weights.size();
			// Set the weight vector itself
			this->_pWeights = new T[num_weights];
			std::copy(weights.begin(), weights.end(), this->_pWeights);
		}

		/**
		* @brief Sets the weights vector from a pointer array.
		* @param weights the weights vector
		* @param num_weights number of elements in the weight vector
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool weights(T* weights, int num_weights)
		{
			// By definition, num_weights should be the same as num_ctrlpts
			if (num_weights != this->ctrlpts_len())
			{
				std::cerr << "NURBS ERROR: Size of the weights vector must be equal to total number of control points" << std::endl;
				return false;
			}

			// Check if the pointer is empty. If not, empty it.
			if (this->_pWeights != nullptr)
			{
				delete[] this->_pWeights;
				this->_pWeights = nullptr;
			}
			// Copy the contents of the weights array into the new pointer array
			this->_pWeights = new T[num_weights];
			std::copy(weights, weights + num_weights, this->_pWeights);

			return true;
		}

		/**
		* @brief Returns the weights vector.
		* @return the weights vector
		*/
		T* weights()
		{
			return this->_pWeights;
		}

		/**
		* @brief Returns the calculated surface points as an 1D array.
		* @return the surface points
		*/
		TPoint3<T>* surfpts()
		{
			return this->_pSurfPts;
		}

		/**
		* @brief Returns the calculated surface points as a 2D array.
		* @return the surface points in [u][v] array format
		*/
		TPoint3<T>** surfpts_2d()
		{
			return this->_pSurfPts2D;
		}

		/**
		* @brief Retrieves the surface point from calculated points array at the given u-v position.
		* @param[in] u_value parameter in the u-direction
		* @param[in] v_value parameter in the v-direction
		* @param[out] out_value the surface point at the input u-v position
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool surfpt(T u_value, T v_value, TPoint3<T>& out_value)
		{
			// We will copy the surface point
			TPoint3<T> ret_val;

			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u,v values
			if (!this->check_uv(u_value, v_value))
				return false;

			// The surface needs to be calculated before returning the surface point
			if (!this->_pSurfPts)
				this->evaluate();

			// Find the correct u-v position in the array
			int pos_u = int(u_value / this->_mDelta);
			int pos_v = int(v_value / this->_mDelta);
			out_value = this->_pSurfPts2D[pos_u][pos_v];
			return true;
		}

		/**
		* @brief Returns the number of surface points in u-direction.
		* @return number of surface points
		*/
		int surfpts_u_len()
		{
			return this->_mNumSurfPts_U;
		}

		/**
		* @brief Returns the number of surface points in v-direction.
		* @return number of surface points
		*/
		int surfpts_v_len()
		{
			return this->_mNumSurfPts_V;
		}

		/**
		* @brief Returns the total number of surface points.
		* @return number of surface points
		*/
		int surfpts_len()
		{
			return this->_mNumSurfPts_U * this->_mNumSurfPts_V;
		}

		/**
		* @brief Sets the delta value used to generate the surface.
		*
		* @param value new delta to be set
		*/
		void delta(T value)
		{
			this->_mDelta = value;
		}

		/**
		* @brief Returns the delta value used to generate the surface.
		*
		* The knot vector, by definition, starts from 0 and ends at 1. To calculate the whole surface at once, one needs to iterate from 0 to 1 with an increment value.
		* This function returns this increment value. By default, the delta value is 0.01 which would generate 10000 surface points in the whole u-v space.
		* @return delta value
		*/
		T delta()
		{
			return T(this->_mDelta);
		}

		/**
		* @brief Reads control points from a file.
		*
		* The control points should follow right-hand rule. This function is a wrapper for read_ctrlpts_from_txt()
		* @param[in] file_name the file name which contains the control points
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool read_ctrlpts(const char* file_name)
		{
			// Create the file object
			std::ifstream infile(file_name);

			// Using ifstream::exceptions() can be problematic due to the default behavior of std::getline(). This is the easiest way of checking.
			if (!infile.good())
			{
				std::cerr << "NURBS ERROR: Cannot open control points file " << "(" << file_name << ")" << " for reading" << std::endl;
				return false;
			}

			// It is easy to use vectors and then convert them into pointer arrays
			List<TPoint3<T>> ctrlpts;
			List<T> weights;
			int num_u = 0, num_v = 0;
			std::string line;
			// operator>> always ignores whitespaces, so no need to check for empty lines
			while (infile >> line)
			{
				// Now we are on the line, which is V coordinate
				std::stringstream line_stream(line);
				std::string cell;
				num_v = 0;
				List<TPoint3<T>> ctrlpts_v;
				while (std::getline(line_stream, cell, ';'))
				{
					// Now, we are on a single coordinate
					char delim = ',';
					unsigned long long i = 0;
					auto pos = cell.find(delim);
					T x, y, z;
					std::stringstream ss_x(cell.substr(i, pos - i));
					ss_x >> x;
					i = ++pos;
					pos = cell.find(delim, pos);
					std::stringstream ss_y(cell.substr(i, pos - i));
					ss_y >> y;
					i = ++pos;
					pos = cell.find(delim, pos);
					std::stringstream ss_z(cell.substr(i, pos - i));
					ss_z >> z;

					// Set point coordinates
					TPoint3<T> point;
					point.x(x);
					point.y(y);
					point.z(z);

					// Push the point into control points vector
					ctrlpts_v.push_back(point);

					// Update weights vector with the default value
					weights.push_back(T(1.0));

					// Increment V count
					num_v++;
				}

				//ctrlpts_v.reverse();
				ctrlpts.push_back(ctrlpts_v);

				// Increment U count
				num_u++;
			}

			// Close the file
			infile.close();

			// Get the number of control points on the U and V axes
			this->_mNumCtrlPts_U = num_u;
			this->_mNumCtrlPts_V = num_v;

			// Control points size
			int ctrlpts_size = num_u * num_v;

			// Copy the contents of the vector into the global variable
			if (this->_pCtrlPts)
			{
				delete[] this->_pCtrlPts;
			}
			this->_pCtrlPts = new TPoint3<T>[ctrlpts_size];
			std::copy(ctrlpts.begin(), ctrlpts.end(), this->_pCtrlPts);

			// Prepare 2D control points array
			this->init_ctrlpts2d(num_u, num_v);
			for (int i = 0; i < this->_mNumCtrlPts_U; i++)
			{
				for (int j = 0; j < this->_mNumCtrlPts_V; j++)
				{
					this->_pCtrlPts2D[i][j] = this->_pCtrlPts[j + (i * this->_mNumCtrlPts_V)];
				}
			}

			// Automatically populate the weights array defaulting to 1.0
			if (this->_pWeights)
			{
				delete[] this->_pWeights;
			}
			this->_pWeights = new T[ctrlpts_size];
			std::copy(weights.begin(), weights.end(), this->_pWeights);

			return true;
		}

		/**
		* @brief Saves control points grid to a file.
		* @param[in] file_name file name to save control points
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool save_ctrlpts(const char* file_name)
		{
			std::ofstream outfile;
			outfile.open(file_name);
			if (!outfile.is_open())
			{
				std::cerr << "NURBS ERROR: Cannot open file " << std::string(file_name) << " for saving" << std::endl;
				return false;
			}

			for (int i = 0; i < this->_mNumCtrlPts_U; i++)
			{
				for (int j = 0; j < this->_mNumCtrlPts_V; j++)
				{
					outfile << this->_pCtrlPts2D[i][j].x() << "," << this->_pCtrlPts2D[i][j].y() << "," << this->_pCtrlPts2D[i][j].z();
					if (j != this->_mNumCtrlPts_V - 1)
						outfile << ";";
					else
						outfile << "\n";
				}
			}

			outfile.close();

			return true;
		}

		/**
		* @brief Transposes control points by swapping U and V coordinates.
		*/
		void transpose_ctrlpts()
		{
			int num_ctrlpts_u_new = this->_mNumCtrlPts_V;
			int num_ctrlpts_u_old = this->_mNumCtrlPts_U;
			int num_ctrlpts_v_new = this->_mNumCtrlPts_U;
			int num_ctrlpts_v_old = this->_mNumCtrlPts_V;

			// Flip 1D control points array using a temporary array
			TPoint3<T>* ctrlpts_new = new TPoint3<T>[this->_mNumCtrlPts_U * this->_mNumCtrlPts_V];
			int ci = 0;
			for (int j = 0; j < num_ctrlpts_v_old; j++)
			{
				for (int i = 0; i < num_ctrlpts_u_old; i++)
				{
					ctrlpts_new[ci] = this->_pCtrlPts2D[i][j];
					ci++;
				}
			}
			
			// Delete 2D control points array
			for (int i = 0; i < this->_mNumCtrlPts_U; i++)
				delete[] this->_pCtrlPts2D[i];
			delete[] this->_pCtrlPts2D;
			this->_pCtrlPts2D = nullptr;

			// Update 1D control points array
			this->_mNumCtrlPts_U = num_ctrlpts_u_new;
			this->_mNumCtrlPts_V = num_ctrlpts_v_new;
			delete[] this->_pCtrlPts;
			this->_pCtrlPts = new TPoint3<T>[this->_mNumCtrlPts_U * this->_mNumCtrlPts_V];
			std::copy(ctrlpts_new, ctrlpts_new + (num_ctrlpts_u_new * num_ctrlpts_v_new), this->_pCtrlPts);

			// Delete temporary array
			delete[] ctrlpts_new;
			ctrlpts_new = nullptr;

			// Update 2D control points array
			this->init_ctrlpts2d(num_ctrlpts_u_new, num_ctrlpts_v_new);
			for (int i = 0; i < num_ctrlpts_u_new; i++)
			{
				for (int j = 0; j < num_ctrlpts_v_new; j++)
				{
					this->_pCtrlPts2D[i][j] = this->_pCtrlPts[j + (i * this->_mNumCtrlPts_V)];
				}
			}
		}

		/**
		* @brief Transposes the surface by swapping U and V coordinates.
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool transpose()
		{
			// Check that we have the necessary variables for surface reversal
			if (!this->pre_calculate())
				return false;

			/*
			* #### Copy old values ####
			*/

			// Process degrees
			int degree_u_new = this->_mDegree_V;
			int degree_v_new = this->_mDegree_U;

			// Process knot vectors
			int num_knot_vector_u_new = this->_mNumKnotVector_V;
			int num_knot_vector_u_old = this->_mNumKnotVector_U;
			int num_knot_vector_v_new = this->_mNumKnotVector_U;
			int num_knot_vector_v_old = this->_mNumKnotVector_V;

			T* knot_vector_u_new = new T[num_knot_vector_u_new];
			std::copy(this->_pKnotVector_V, this->_pKnotVector_V + num_knot_vector_u_old, knot_vector_u_new);

			T* knot_vector_v_new = new T[num_knot_vector_v_new];
			std::copy(this->_pKnotVector_U, this->_pKnotVector_U + num_knot_vector_v_old, knot_vector_v_new);

			// Set new values 
			this->_mDegree_U = degree_u_new;
			this->_mDegree_V = degree_v_new;
			this->_mNumKnotVector_U = num_knot_vector_u_new;
			this->_mNumKnotVector_V = num_knot_vector_v_new;
			delete[] this->_pKnotVector_U;
			this->_pKnotVector_U = new T[this->_mNumKnotVector_U];
			std::copy(knot_vector_u_new, knot_vector_u_new + num_knot_vector_u_new, this->_pKnotVector_U);
			delete[] this->_pKnotVector_V;
			this->_pKnotVector_V = new T[this->_mNumKnotVector_V];
			std::copy(knot_vector_v_new, knot_vector_v_new + num_knot_vector_v_new, this->_pKnotVector_V);

			// Delete temporary pointers
			delete[] knot_vector_u_new;
			knot_vector_u_new = nullptr;
			delete[] knot_vector_v_new;
			knot_vector_v_new = nullptr;

			// Transpose control points
			this->transpose_ctrlpts();

			return true;
		}

		/**
		* @brief Evaluates the surface.
		* 
		* It is possible to get the surface point array using the getter function surfpts().
		* Implementation of Algorithm A3.5 of The NURBS Book by Piegl and Tiller.
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool evaluate()
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Small u values
			T uv_min = 0.0;
			T uv_max = 1.0;

			List<T> inc_u_vect;
			List<T> inc_v_vect;

			while (uv_min < uv_max)
			{
				inc_u_vect.push_back(uv_min);
				inc_v_vect.push_back(uv_min);
				uv_min += T(this->_mDelta);
			}
			inc_u_vect.push_back(uv_max);
			inc_v_vect.push_back(uv_max);

			// Initialize surface points 2D array
			this->init_surfpts2d(int(inc_u_vect.size()), int(inc_v_vect.size()));

			// Initialize surface points 1D array
			this->_pSurfPts = new TPoint3<T>[int(inc_u_vect.size()) * int(inc_v_vect.size())];
			int surfpt_ct = 0;

			// Algorithm A3.5
			for (int iv = 0; iv < (int)inc_v_vect.size(); iv++)
			{
				// Process V axis
				int span_v = this->find_span(this->_mDegree_V, this->_pKnotVector_V, this->_mNumCtrlPts_V, inc_v_vect[iv]);
				T* basis_funs_v = new T[this->_mDegree_V + 1];
				this->basis_functions(this->_mDegree_V, this->_pKnotVector_V, span_v, inc_v_vect[iv], basis_funs_v);

				// Set surface points for U-axis to zero
				this->_mNumSurfPts_U = 0;

				for (int iu = 0; iu < (int)inc_u_vect.size(); iu++)
				{
					// Process U axis
					int span_u = this->find_span(this->_mDegree_U, this->_pKnotVector_U, this->_mNumCtrlPts_U, inc_u_vect[iu]);
					T* basis_funs_u = new T[this->_mDegree_U + 1];
					this->basis_functions(this->_mDegree_U, this->_pKnotVector_U, span_u, inc_u_vect[iu], basis_funs_u);

					int uind = span_u - this->_mDegree_U;
					TPoint3<T> surfpt(0.0);
					for (int l = 0; l <= this->_mDegree_V; l++)
					{
						TPoint3<T> temp(0.0);
						int vind = span_v - this->_mDegree_V + l;
						for (int k = 0; k <= this->_mDegree_U; k++)
						{
							temp.x(temp.x() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].x()));
							temp.y(temp.y() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].y()));
							temp.z(temp.z() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].z()));
						}
						surfpt.x(surfpt.x() + (basis_funs_v[l] * temp.x()));
						surfpt.y(surfpt.y() + (basis_funs_v[l] * temp.y()));
						surfpt.z(surfpt.z() + (basis_funs_v[l] * temp.z()));
					}
					this->_pSurfPts2D[iu][iv] = surfpt;
					this->_mNumSurfPts_U++;

					// Also, add the calculated surface point to the 1D array
					this->_pSurfPts[surfpt_ct] = surfpt;
					surfpt_ct++;

					delete[] basis_funs_u;
				}
				this->_mNumSurfPts_V++;

				delete[] basis_funs_v;
			}

			return true;
		}

		/**
		* @brief Evaluates a single surface point at the given u-v coordinate.
		*
		* Implementation of Algorithm A3.5 from The NURBS Book by Piegl and Tiller.
		* @param[in] u_value input u-coordinate
		* @param[in] v_value input v-coordinate
		* @param[out] out_value the calculated surface point
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool surfpoint(T u_value, T v_value, TPoint3<T>& out_value)
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u,v values
			if (!this->check_uv(u_value, v_value))
				return false;

			// Algorithm A3.5
			int span_u = this->find_span(this->_mDegree_U, this->_pKnotVector_U, this->_mNumCtrlPts_U, u_value);
			T* basis_funs_u = new T[this->_mDegree_U + 1];
			this->basis_functions(this->_mDegree_U, this->_pKnotVector_U, span_u, u_value, basis_funs_u);

			int span_v = this->find_span(this->_mDegree_V, this->_pKnotVector_V, this->_mNumCtrlPts_V, v_value);
			T* basis_funs_v = new T[this->_mDegree_V + 1];
			this->basis_functions(this->_mDegree_V, this->_pKnotVector_V, span_v, v_value, basis_funs_v);

			int uind = span_u - this->_mDegree_U;
			out_value = TPoint3<T>(0.0);

			for (int l = 0; l <= this->_mDegree_V; l++)
			{
				TPoint3<T> temp(0.0);
				int vind = span_v - this->_mDegree_V + l;
				for (int k = 0; k <= this->_mDegree_U; k++)
				{
					temp.x(temp.x() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].x()));
					temp.y(temp.y() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].y()));
					temp.z(temp.z() + (basis_funs_u[k] * this->_pCtrlPts2D[uind + k][vind].z()));
				}
				out_value.x(out_value.x() + (basis_funs_v[l] * temp.x()));
				out_value.y(out_value.y() + (basis_funs_v[l] * temp.y()));
				out_value.z(out_value.z() + (basis_funs_v[l] * temp.z()));
			}

			// Deallocate the temporary pointers
			delete[] basis_funs_v;
			basis_funs_v = nullptr;
			delete[] basis_funs_u;
			basis_funs_u = nullptr;

			return true;
		}

		/**
		* @brief Evaluates the derivates of the surface at the given u-v coordinate.
		*
		* Implementation of Algorithm A3.6 from The NURBS Book by Piegl and Tiller.
		* @param[in] u_value input u-coordinate
		* @param[in] v_value input v-coordinate
		* @param[in] d derivative order
		* @param[out] SKL the calculated surface point and the derivatives
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool derivatives(T u_value, T v_value, int d, TPoint3<T>**& SKL)
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u,v values
			if (!this->check_uv(u_value, v_value))
				return false;

			// Allocate memory for the SKL pointer array (leave deallocation to the user)
			SKL = new TPoint3<T>*[d + 1];
			for (int i = 0; i <= d; i++)
				SKL[i] = new TPoint3<T>[d + 1];

			// Algorithm A3.6
			int du = std::min(d, this->_mDegree_U);
			for (int k = this->_mDegree_U + 1; k <= d; k++)
			{
				for (int l = 0; l <= d - k; l++)
				{
					SKL[k][l] = TPoint3<T>(0.0);
				}
			}

			int dv = std::min(d, this->_mDegree_V);
			for (int l = this->_mDegree_V + 1; l <= d; l++)
			{
				for (int k = 0; k <= d - l; k++)
				{
					SKL[k][l] = TPoint3<T>(0.0);
				}
			}

			int span_u = this->find_span(this->_mDegree_U, this->_pKnotVector_U, this->_mNumCtrlPts_U, u_value);
			T** basis_funs_ders_u = new T*[du + 1];
			for (int i = 0; i < du + 1; i++)
			{
				basis_funs_ders_u[i] = new T[this->_mDegree_U + 1];
			}
			this->basis_functions_ders(this->_mDegree_U, this->_pKnotVector_U, span_u, u_value, du, basis_funs_ders_u);

			int span_v = this->find_span(this->_mDegree_V, this->_pKnotVector_V, this->_mNumCtrlPts_V, v_value);
			T** basis_funs_ders_v = new T*[dv + 1];
			for (int i = 0; i < dv + 1; i++)
			{
				basis_funs_ders_v[i] = new T[this->_mDegree_V + 1];
			}
			this->basis_functions_ders(this->_mDegree_V, this->_pKnotVector_V, span_v, v_value, dv, basis_funs_ders_v);

			for (int k = 0; k <= du; k++)
			{
				TPoint3<T>* temp = new TPoint3<T>[this->_mDegree_V + 1];
				for (int s = 0; s <= this->_mDegree_V; s++)
				{
					temp[s] = TPoint3<T>(0.0);
					for (int r = 0; r <= this->_mDegree_U; r++)
					{
						temp[s].x(temp[s].x() + (basis_funs_ders_u[k][r] * this->_pCtrlPts2D[span_u - this->_mDegree_U + r][span_v - this->_mDegree_V + s].x()));
						temp[s].y(temp[s].y() + (basis_funs_ders_u[k][r] * this->_pCtrlPts2D[span_u - this->_mDegree_U + r][span_v - this->_mDegree_V + s].y()));
						temp[s].z(temp[s].z() + (basis_funs_ders_u[k][r] * this->_pCtrlPts2D[span_u - this->_mDegree_U + r][span_v - this->_mDegree_V + s].z()));
					}
				}
				int dd = std::min(d - k, dv);
				for (int l = 0; l <= dd; l++)
				{
					SKL[k][l] = TPoint3<T>(0.0);
					for (int s = 0; s <= this->_mDegree_V; s++)
					{
						SKL[k][l].x(SKL[k][l].x() + (basis_funs_ders_v[l][s] * temp[s].x()));
						SKL[k][l].y(SKL[k][l].y() + (basis_funs_ders_v[l][s] * temp[s].y()));
						SKL[k][l].z(SKL[k][l].z() + (basis_funs_ders_v[l][s] * temp[s].z()));
					}
				}
				delete[] temp;
				temp = nullptr;
			}

			// Delete temporary pointers
			for (int i = 0; i < du + 1; i++)
			{
				delete[] basis_funs_ders_u[i];
			}
			delete[] basis_funs_ders_u;
			basis_funs_ders_u = nullptr;

			for (int i = 0; i < dv + 1; i++)
			{
				delete[] basis_funs_ders_v[i];
			}
			delete[] basis_funs_ders_v;
			basis_funs_ders_v = nullptr;

			return true;
		}

		/**
		* @brief Evaluates the tangent of the surface w.r.t. to u direction at the given u-v coordinates.
		*
		* Calls derivatives() with the order parameter 1.
		* @param[in] u_value the parametric coordinate u
		* @param[in] v_value the parametric coordinate v
		* @param[out] out_value the first derivative of the surface with respect to u
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool tangent_u(T u_value, T v_value, TPoint3<T>& out_value)
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u-v values
			if (!this->check_uv(u_value, v_value))
				return false;

			// Tangent of the surface is the first derivative
			int d = 1;
			TPoint3<T>** SKL = new TPoint3<T>*[d + 1];
			for (int i = 0; i <= d; i++)
				SKL[i] = new TPoint3<T>[d + 1];
			this->derivatives(u_value, v_value, d, SKL);

			// Return S_u
			out_value = SKL[d][0];

			// Clean-up the temporary pointers
			for (int i = 0; i <= d; i++)
				delete[] SKL[i];
			delete[] SKL;
			SKL = nullptr;

			return true;
		}

		/**
		* @brief Evaluates the tangent of the surface w.r.t. to v direction at the given u-v coordinates.
		*
		* Calls derivatives() with the order parameter 1.
		* @param[in] u_value the parametric coordinate u
		* @param[in] v_value the parametric coordinate v
		* @param[out] out_value the first derivative of the surface with respect to v
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool tangent_v(T u_value, T v_value, TPoint3<T>& out_value)
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u-v values
			if (!this->check_uv(u_value, v_value))
				return false;

			// Tangent of the surface is the first derivative
			int d = 1;
			TPoint3<T>** SKL = new TPoint3<T>*[d + 1];
			for (int i = 0; i <= d; i++)
				SKL[i] = new TPoint3<T>[d + 1];
			this->derivatives(u_value, v_value, d, SKL);

			// Return S_v
			out_value = SKL[0][d];

			// Clean-up the temporary pointers
			for (int i = 0; i <= d; i++)
				delete[] SKL[i];
			delete[] SKL;
			SKL = nullptr;

			return true;
		}

		/**
		* @brief Evaluates the normal to the surface at the given u-v coordinates.
		*
		* By definition, a normal to a surface at a point is the same as a normal to the tangent plane to that surface at that point.
		* It is possible to calculate a normal vector to a planar surface by using vector cross-product.
		* @see The NURBS Book, Equation 6.74
		* @param[in] u_value input u-coordinate
		* @param[in] v_value input v-coordinate
		* @param[out] out_value the calculated normal
		* @return FALSE if any errors, TRUE otherwise
		*/
		bool normal(T u_value, T v_value, TVector3<T>& out_value)
		{
			// Check that we have the necessary variables for surface calculations
			if (!this->pre_calculate())
				return false;

			// Check u,v values
			if (!this->check_uv(u_value, v_value, true))
				return false;

			// First, we need to find the tangent plane by finding the first derivative
			int d = 1; TPoint3<T>** SKL = nullptr;
			this->derivatives(u_value, v_value, d, SKL);

			// Second, extract the points on the tangent plane
			TVector3<T> u_derv(SKL[d][0]); // S_u at (u,v)
			TVector3<T> v_derv(SKL[0][d]); // S_v at (u,v)

			// Third, compute the output vector
			out_value = u_derv.cross(v_derv);

			// Clean-up the temporary pointers
			for (int i = 0; i < d + 1; i++)
			{
				delete[] SKL[i];
				SKL[i] = nullptr;
			}
			delete[] SKL;
			SKL = nullptr;

			return true;
		}

		/**
		 * @brief Checks that all necessary NURBS parameters are set for conversions and evaluations.
		 *
		 * This function is a wrapper for the private method pre_calculate() with an additional error message.
		 * @return FALSE if any errors, TRUE otherwise
		 */
		bool check()
		{
			bool check_val = this->pre_calculate();
			if (!check_val)
				std::cerr << "NURBS ERROR: NURBS parameters are not set" << std::endl;
			return check_val;
		}

	private:
		int _mDegree_U; /**< Degree of the u knot vector  */
		int _mDegree_V; /**< Degree of the v knot vector  */
		TPoint3<T>* _pCtrlPts; /**< Control points (1D array) */
		TPoint3<T>** _pCtrlPts2D; /**< Control points (2D array) */
		int _mNumCtrlPts_U; /**< Number of control points in u-direction */
		int _mNumCtrlPts_V; /**< Number of control points in v-direction */
		T* _pWeights; /**< Weights vector */
		T* _pKnotVector_U; /**< Knot vector for u-direction */
		T* _pKnotVector_V; /**< Knot vector for v-direction */
		int _mNumKnotVector_U; /**< Number of knots in the knot vector for u-direction */
		int _mNumKnotVector_V; /**< Number of knots in the knot vector for v-direction */
		TPoint3<T>* _pSurfPts; /**< Calculated surface points (1D array) */
		TPoint3<T>** _pSurfPts2D; /**< Calculated surface points (2D array) */
		int _mNumSurfPts_U; /**< Number of surface points in u-direction */
		int _mNumSurfPts_V; /**< Number of surface points in v-direction */
		T _mDelta; /**< Delta value for surface point spacing */

		/**
		* @brief Helper function for constructors.
		*/
		void init_vars()
		{
			this->_mDegree_U = 0;
			this->_mDegree_V = 0;
			this->_mNumCtrlPts_U = 0;
			this->_mNumCtrlPts_V = 0;
			this->_pWeights = nullptr;
			this->_pKnotVector_U = nullptr;
			this->_pKnotVector_V = nullptr;
			this->_mNumKnotVector_U = 0;
			this->_mNumKnotVector_V = 0;
			this->_pCtrlPts = nullptr;
			this->_pCtrlPts2D = nullptr;
			this->_pSurfPts = nullptr;
			this->_pSurfPts2D = nullptr;
			this->_mNumSurfPts_U = 0;
			this->_mNumSurfPts_V = 0;
			this->_mDelta = T(0.01);
		}

		/**
		* @brief Helper function for the destructor.
		*/
		void delete_vars()
		{
			// Deallocate the memory for the weights vector
			if (this->_pWeights)
			{
				delete[] this->_pWeights;
				this->_pWeights = nullptr;
			}

			// Deallocate the memory for the knot vector U
			if (this->_pKnotVector_U)
			{
				delete[] this->_pKnotVector_U;
				this->_pKnotVector_U = nullptr;
			}

			// Deallocate the memory for the knot vector V
			if (this->_pKnotVector_V)
			{
				delete[] this->_pKnotVector_V;
				this->_pKnotVector_V = nullptr;
			}

			// Deallocate the memory for the control points
			if (this->_pCtrlPts)
			{
				delete[] this->_pCtrlPts;
				this->_pCtrlPts = nullptr;
			}

			// Deallocate the memory for the control points 2D array
			if (this->_pCtrlPts2D)
			{
				for (int i = 0; i < this->_mNumCtrlPts_U; i++)
				{
					delete[] this->_pCtrlPts2D[i];
				}
				delete[] this->_pCtrlPts2D;
				this->_pCtrlPts2D = nullptr;
			}

			// Deallocate the memory for the surface points
			if (this->_pSurfPts)
			{
				delete[] this->_pSurfPts;
				this->_pSurfPts = nullptr;
			}

			// Deallocate the memory for the surface points 2D array
			if (this->_pSurfPts2D)
			{
				for (int i = 0; i < this->_mNumSurfPts_U; i++)
				{
					delete[] this->_pSurfPts2D[i];
				}
				delete[] this->_pSurfPts2D;
				this->_pSurfPts2D = nullptr;
			}
		}

		/**
		* @brief Helper function for copy ctor / operator.
		* @param rhs object on the right side
		*/
		void copy_vars(const NURBS& rhs)
		{
			this->_mDegree_U = rhs._mDegree_U;
			this->_mDegree_V = rhs._mDegree_V;

			if (this->_pKnotVector_U != nullptr)
			{
				delete[] this->_pKnotVector_U;
				this->_pKnotVector_U = nullptr;
			}
			this->_pKnotVector_U = new T[rhs._mNumKnotVector_U];
			std::copy(rhs._pKnotVector_U, rhs._pKnotVector_U + rhs._mNumKnotVector_U, this->_pKnotVector_U);
			this->_mNumKnotVector_U = rhs._mNumKnotVector_U;

			if (this->_pKnotVector_V != nullptr)
			{
				delete[] this->_pKnotVector_V;
				this->_pKnotVector_V = nullptr;
			}
			this->_pKnotVector_V = new T[rhs._mNumKnotVector_V];
			std::copy(rhs._pKnotVector_V, rhs._pKnotVector_V + rhs._mNumKnotVector_V, this->_pKnotVector_V);
			this->_mNumKnotVector_V = rhs._mNumKnotVector_V;

			int ctrlpts_size = rhs._mNumCtrlPts_U * rhs._mNumCtrlPts_V;
			if (this->_pCtrlPts != nullptr)
			{
				delete[] this->_pCtrlPts;
				this->_pCtrlPts = nullptr;
			}
			this->_pCtrlPts = new TPoint3<T>[ctrlpts_size];
			std::copy(rhs._pCtrlPts, rhs._pCtrlPts + ctrlpts_size, this->_pCtrlPts);
			this->_mNumCtrlPts_U = rhs._mNumCtrlPts_U;
			this->_mNumCtrlPts_V = rhs._mNumCtrlPts_V;

			if (this->_pCtrlPts2D != nullptr)
			{
				for (int i = 0; i < this->_mNumCtrlPts_V; i++)
					delete[] this->_pCtrlPts2D[i];
				delete[] this->_pCtrlPts2D;
				this->_pCtrlPts2D = nullptr;
			}

			this->_pCtrlPts2D = new TPoint3<T>*[rhs._mNumCtrlPts_U];
			for (int i = 0; i < rhs._mNumCtrlPts_U; i++)
			{
				this->_pCtrlPts2D[i] = new TPoint3<T>[rhs._mNumCtrlPts_V];
				std::copy(rhs._pCtrlPts2D[i], rhs._pCtrlPts2D[i] + rhs._mNumCtrlPts_V, this->_pCtrlPts2D[i]);
			}

			if (this->_pWeights != nullptr)
			{
				delete[] this->_pWeights;
				this->_pWeights = nullptr;
			}
			this->_pWeights = new T[ctrlpts_size];
			std::copy(rhs._pWeights, rhs._pWeights + ctrlpts_size, this->_pWeights);

			// Don't copy the surface points, they can be calculated later
			this->_mNumSurfPts_U = 0;
			this->_mNumSurfPts_V = 0;
			this->_pSurfPts = nullptr;
			this->_pSurfPts2D = nullptr;
		}

		/**
		* @brief Initializes the 2D control point array.
		* @param num_u number of elements in u-direction
		* @param num_v number of elements in v-direction
		*/
		void init_ctrlpts2d(int num_u, int num_v)
		{
			// Clear the pointer array to prevent memory leaks
			if (this->_pCtrlPts2D)
			{
				for (int i = 0; i < this->_mNumCtrlPts_V; i++)
				{
					delete[] this->_pCtrlPts2D[i];
				}
				delete[] this->_pCtrlPts2D;
				this->_pCtrlPts2D = nullptr;
			}

			// Initialize the 2D pointer array
			this->_pCtrlPts2D = new TPoint3<T>*[num_u];
			for (int i = 0; i < num_u; i++)
			{
				this->_pCtrlPts2D[i] = new TPoint3<T>[num_v];
			}
		}

		/**.
		* @brief Initializes the 2D surface point array
		* @param num_u number of elements in u-direction
		* @param num_v number of elements in v-direction
		*/
		void init_surfpts2d(int num_u, int num_v)
		{
			// Clear the pointer array to prevent memory leaks
			if (this->_pSurfPts2D)
			{
				for (int i = 0; i < this->_mNumSurfPts_V; i++)
				{
					delete[] this->_pSurfPts2D[i];
				}
				delete[] this->_pSurfPts2D;
				this->_pSurfPts2D = nullptr;
			}

			// Initialize the 2D pointer array
			this->_pSurfPts2D = new TPoint3<T>*[num_u];
			for (int i = 0; i < num_u; i++)
			{
				this->_pSurfPts2D[i] = new TPoint3<T>[num_v];
			}
		}

		/**
		* @brief Checks the given u and v coordinates are valid for the B-spline equations.
		*
		* Additionally, checks the given coordinates are on an edge or on a vertex.
		* @param u u-coordinate to be checked
		* @param v v-coordinate to be checked
		* @param test_normal a bool value to enable executing additional tests for normal calculations
		* @return if the input coordinates are valid, returns true
		*/
		bool check_uv(T u, T v, bool test_normal = false)
		{
			// Additional tests for normal calculations
			if (test_normal)
			{
				// Edge check
				if (u + this->_mDelta > 1.0 || v + this->_mDelta > 1.0)
				{
					std::cerr << "NURBS ERROR: Cannot calculate normal on an edge" << std::endl;
					return false;
				}
				if (u + this->_mDelta < 0.0 || v + this->_mDelta < 0.0)
				{
					std::cerr << "NURBS ERROR: Cannot calculate normal on an edge" << std::endl;
					return false;
				}
			}

			// The min value of u or v is 0.0 and max value is 1.0
			if (u < 0.0 || u > 1.0)
			{
				std::cerr << "NURBS ERROR: The value of U must be between 0.0 and 1.0" << std::endl;
				return false;
			}
			if (v < 0.0 || v > 1.0)
			{
				std::cerr << "NURBS ERROR: The value of V must be between 0.0 and 1.0" << std::endl;
				return false;
			}

			// Otherwise, everything should be okay
			return true;
		}

		/**
		* @brief Normalizes the knot vector.
		*
		* The knot vector, by definition, is in an increasing order and all knots must be located between 0 and 1.
		* This function rearranges the knot values between 0 and 1.
		* Please note that you must initialize the output knot vector array before input.
		* @param knot_vector_in input knot vector to be normalized (INPUT)
		* @param knot_vector_in_size size of the input knot vector (INPUT)
		* @param knot_vector_out normalized knot vector (OUTPUT)
		*/
		void normalize(T* knot_vector_in, int knot_vector_in_size, T* knot_vector_out)
		{
			// If the pointer array length is zero, do not normalize!
			if (knot_vector_in_size == 0)
			{
				return;
			}

			// Find first and last element of the input knot vector
			T first_knot = knot_vector_in[0];
			T last_knot = knot_vector_in[knot_vector_in_size - 1];

			// Iterate through the knot vector to normalize knot values
			for (int i = 0; i < knot_vector_in_size; i++)
			{
				knot_vector_out[i] = (knot_vector_in[i] - first_knot) / (last_knot - first_knot);
			}
		}

		/**
		* @brief Evaluates the basis functions which controls the input knot.
		*
		* Implementation of Algorithm A2.2 from The NURBS Book by Piegl and Tiller.
		* @param degree degree of the input knot vector (INPUT)
		* @param knot_vector input knot vector to be normalized (INPUT)
		* @param span current span on the knot vector (INPUT)
		* @param knot knot value (INPUT)
		* @param basis_funs_out calculated basis functions array (OUTPUT)
		*/
		void basis_functions(int degree, T* knot_vector, int span, T knot, T* basis_funs)
		{
			// Algorithm A2.2
			T* left = new T[degree + 1];
			T* right = new T[degree + 1];
			T saved;
			T temp;

			// N[0] = 1.0 by definition
			basis_funs[0] = 1.0;

			for (int j = 1; j <= degree; j++)
			{
				left[j] = knot - knot_vector[span + 1 - j];
				right[j] = knot_vector[span + j] - knot;
				saved = 0.0;
				for (int r = 0; r < j; r++)
				{
					temp = basis_funs[r] / (right[r + 1] + left[j - r]);
					basis_funs[r] = saved + right[r + 1] * temp;
					saved = left[j - r] * temp;
				}
				basis_funs[j] = saved;
			}

			// Delete temporary pointers
			delete[] left;
			left = nullptr;
			delete[] right;
			right = nullptr;
		}

		/**
		* @brief Evaluates the basis functions and their derivatives in the given order.
		*
		* Implementation of Algorithm A2.3 from The NURBS Book by Piegl and Tiller.
		* @param degree degree of the input knot vector (INPUT)
		* @param knot_vector input knot vector to be normalized (INPUT)
		* @param span current span on the knot vector (INPUT)
		* @param knot the knot value or the small u value (INPUT)
		* @param n the derivative order
		* @param ders calculated basis functions and derivatives array (OUTPUT)
		*/
		void basis_functions_ders(int degree, T* knot_vector, int span, T knot, int n, T** ders)
		{
			// Algorithm A2.3
			T* left = new T[degree + 1];
			T* right = new T[degree + 1];
			T saved;
			T temp;
			T** ndu = new T*[degree + 1];
			for (int i = 0; i < degree + 1; i++)
			{
				ndu[i] = new T[degree + 1];
			}

			// This is by definition
			ndu[0][0] = 1.0;

			for (int j = 1; j <= degree; j++)
			{
				left[j] = knot - knot_vector[span + 1 - j];
				right[j] = knot_vector[span + j] - knot;
				saved = 0.0;
				for (int r = 0; r < j; r++)
				{
					// Lower triangle
					ndu[j][r] = right[r + 1] + left[j - r];
					temp = ndu[r][j - 1] / ndu[j][r];
					// Upper triangle
					ndu[r][j] = saved + (right[r + 1] * temp);
					saved = left[j - r] * temp;
				}
				ndu[j][j] = saved;
			}

			// Load the basis functions
			for (int j = 0; j <= degree; j++)
			{
				ders[0][j] = ndu[j][degree];
			}

			/* Start calculating derivatives */
			T** a = new T*[2];
			for (int i = 0; i < 2; i++)
			{
				a[i] = new T[degree + 1];
			}
			// Loop over function index
			for (int r = 0; r <= degree; r++)
			{
				// Alternate rows in array a
				int s1 = 0;
				int s2 = 1;
				a[0][0] = 1.0;
				// Loop to compute k-th derivative
				for (int k = 1; k <= n; k++)
				{
					T d = 0.0;
					int rk = r - k;
					int pk = degree - k;
					if (r >= k)
					{
						a[s2][0] = a[s1][0] / ndu[pk + 1][rk];
						d = a[s2][0] * ndu[rk][pk];
					}
					int j1, j2;
					if (rk >= -1)
						j1 = 1;
					else
						j1 = -rk;
					if (r - 1 <= pk)
						j2 = k - 1;
					else
						j2 = degree - r;
					for (int j = j1; j <= j2; j++)
					{
						a[s2][j] = (a[s1][j] - a[s1][j - 1]) / ndu[pk + 1][rk + j];
						d += a[s2][j] * ndu[rk + j][pk];
					}
					if (r <= pk)
					{
						a[s2][k] = -a[s1][k - 1] / ndu[pk + 1][r];
						d += a[s2][k] * ndu[r][pk];
					}
					ders[k][r] = d;

					// Switch rows
					int j = s1;
					s1 = s2;
					s2 = j;
				}
			}

			// Multiply through by the correct factors
			int r = degree;
			for (int k = 1; k <= n; k++)
			{
				for (int j = 0; j <= degree; j++)
				{
					ders[k][j] *= r;
				}
				r *= (degree - k);
			}

			// Delete temporary pointers
			delete[] left;
			left = nullptr;
			delete[] right;
			right = nullptr;
			for (int i = 0; i < 2; i++)
				delete[] a[i];
			delete[] a;
			a = nullptr;
			for (int i = 0; i < degree + 1; i++)
				delete[] ndu[i];
			delete[] ndu;
			ndu = nullptr;
		}

		/**
		* @brief Finds the span of the input knot on the knot vector for basis functions calculations.
		*
		* Implementation of Algorithm A2.1 from The NURBS Book by Piegl and Tiller.
		* @param degree degree of the input knot vector
		* @param knot_vector input knot vector
		* @param num_ctrlpts number of control points
		* @param u knot value
		* @return the span value
		*/
		int find_span(int degree, T* knot_vector, int num_ctrlpts, T u)
		{
			// Algorithm A2.1
			if (knot_vector[num_ctrlpts] == u)
			{
				return num_ctrlpts - 1;
			}

			int low = degree;
			int high = num_ctrlpts;
			int mid = (low + high) / 2;

			while (u < knot_vector[mid] || u >= knot_vector[mid + 1])
			{
				if (u < knot_vector[mid])
				{
					high = mid;
				}
				else
				{
					low = mid;
				}
				mid = (low + high) / 2;
			}
			return mid;
		}

		/**
		* @brief Surface pre-calculation checks.
		*
		* It checks if the class has all the required parameters to calculate a NURBS surface.
		* @return TRUE if the surface is calculatable, FALSE otherwise
		*/
		bool pre_calculate()
		{
			// Check degrees
			if (this->_mDegree_U == 0)
			{
				std::cerr << "NURBS ERROR: Degree U cannot be zero" << std::endl;
				return false;
			}
			if (this->_mDegree_V == 0)
			{
				std::cerr << "NURBS ERROR: Degree V cannot be zero" << std::endl;
				return false;
			}

			// Check knot vectors
			if (!this->_pKnotVector_U)
			{
				std::cerr << "NURBS ERROR: Knot vector U is necessary for surface calculations" << std::endl;
				return false;
			}
			if (!this->_pKnotVector_V)
			{
				std::cerr << "NURBS ERROR: Knot vector V is necessary for surface calculations" << std::endl;
				return false;
			}

			// Check control points
			if (!this->_pCtrlPts)
			{
				std::cerr << "NURBS ERROR: Control points are necessary for surface calculations" << std::endl;
				return false;
			}

			// Check weights vector
			if (!this->_pWeights)
			{
				std::cerr << "NURBS ERROR: A weight vector is necessary for surface calculations" << std::endl;
				return false;
			}

			// Everything is good, now we can start calculations
			return true;
		}
	};
}
#endif // !NURBS_HXX
