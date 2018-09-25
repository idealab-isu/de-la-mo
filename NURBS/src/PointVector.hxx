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

#ifndef POINTVECTOR_HXX
#define POINTVECTOR_HXX

// CPP includes
#include <iostream>
#include <cmath>

#ifndef POINTVECTOR_EQUALITY_TOL
#define POINTVECTOR_EQUALITY_TOL 10e-5
#endif // !POINTVECTOR_EQUALITY_TOL

namespace delamo
{

	template <typename T>
	class TPoint3
	{
	public:
		using size_type = unsigned int; /**< Default size type for the TPoint3 class */
		using value_type = T; /**< Default value type for the TPoint3 class */

		/**
		* @brief Default constructor.
		*
		* Sets all coordinates to zero.
		*/
		TPoint3()
		{
			this->_rgCoord[0] = T(0.0);
			this->_rgCoord[1] = T(0.0);
			this->_rgCoord[2] = T(0.0);
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets all coordinates to the input value.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param value coordinate value (same for all coordinates)
		*/
		explicit TPoint3(T value)
		{
			this->_rgCoord[0] = value;
			this->_rgCoord[1] = value;
			this->_rgCoord[2] = value;
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets the coordinates to the input values, respectively.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param x_value value of the x-coordinate
		* @param y_value value of the y-coordinate
		* @param z_value value of the z-coordinate
		*/
		explicit TPoint3(T x_value, T y_value, T z_value)
		{
			this->_rgCoord[0] = x_value;
			this->_rgCoord[1] = y_value;
			this->_rgCoord[2] = z_value;
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets the coordinates to the input values, respectively.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param p array containing the values of x-, y- and z- coordinates
		*/
		explicit TPoint3(T p[3])
		{
			this->_rgCoord[0] = p[0];
			this->_rgCoord[1] = p[1];
			this->_rgCoord[2] = p[2];
		}

		/**
		* @brief Copy constructor.
		* @param rhs object to be copied
		*/
		TPoint3(const TPoint3<T>& rhs)
		{
			this->_rgCoord[0] = rhs._rgCoord[0];
			this->_rgCoord[1] = rhs._rgCoord[1];
			this->_rgCoord[2] = rhs._rgCoord[2];
		}

		/**
		* @brief Default destructor.
		*/
		~TPoint3()
		{
			// Empty destructor
		}

		/**
		* @brief Copy assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TPoint3<T>& operator=(const TPoint3<T>& rhs)
		{
			// Check for self assignment
			if (this != &rhs)
			{
				this->_rgCoord[0] = rhs._rgCoord[0];
				this->_rgCoord[1] = rhs._rgCoord[1];
				this->_rgCoord[2] = rhs._rgCoord[2];
			}
			return *this;
		}

		/**
		* @brief Addition assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TPoint3<T>& operator+=(const TPoint3<T>& rhs)
		{
			this->_rgCoord[0] += rhs._rgCoord[0];
			this->_rgCoord[1] += rhs._rgCoord[1];
			this->_rgCoord[2] += rhs._rgCoord[2];
			return *this;
		}

		/**
		* @brief Subtraction assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TPoint3<T>& operator-=(const TPoint3<T>& rhs)
		{
			this->_rgCoord[0] -= rhs._rgCoord[0];
			this->_rgCoord[1] -= rhs._rgCoord[1];
			this->_rgCoord[2] -= rhs._rgCoord[2];
			return *this;
		}

		/**
		* @brief Multiplication assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TPoint3<T>& operator*=(const TPoint3<T>& rhs)
		{
			this->_rgCoord[0] *= rhs._rgCoord[0];
			this->_rgCoord[1] *= rhs._rgCoord[1];
			this->_rgCoord[2] *= rhs._rgCoord[2];
			return *this;
		}

		/**
		* @brief Division assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TPoint3<T>& operator/=(const TPoint3<T>& rhs)
		{
			this->_rgCoord[0] /= rhs._rgCoord[0];
			this->_rgCoord[1] /= rhs._rgCoord[1];
			this->_rgCoord[2] /= rhs._rgCoord[2];
			return *this;
		}

		/**
		* @brief Addition assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TPoint3<T>& operator+=(const T& rhs)
		{
			this->_rgCoord[0] += rhs;
			this->_rgCoord[1] += rhs;
			this->_rgCoord[2] += rhs;
			return *this;
		}

		/**
		* @brief Subtraction assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TPoint3<T>& operator-=(const T& rhs)
		{
			this->_rgCoord[0] -= rhs;
			this->_rgCoord[1] -= rhs;
			this->_rgCoord[2] -= rhs;
			return *this;
		}

		/**
		* @brief Multiplication assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TPoint3<T>& operator*=(const T& rhs)
		{
			this->_rgCoord[0] *= rhs;
			this->_rgCoord[1] *= rhs;
			this->_rgCoord[2] *= rhs;
			return *this;
		}

		/**
		* @brief Division assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TPoint3<T>& operator/=(const T& rhs)
		{
			this->_rgCoord[0] /= rhs;
			this->_rgCoord[1] /= rhs;
			this->_rgCoord[2] /= rhs;
			return *this;
		}

		/**
		* @brief Addition operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator+(TPoint3<T> lhs, const TPoint3<T>& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		/**
		* @brief Subtraction operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator-(TPoint3<T> lhs, const TPoint3<T>& rhs)
		{
			lhs -= rhs;
			return lhs;
		}

		/**
		* @brief Multiplication operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator*(TPoint3<T> lhs, const TPoint3<T>& rhs)
		{
			lhs *= rhs;
			return lhs;
		}

		/**
		* @brief Division operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator/(TPoint3<T> lhs, const TPoint3<T>& rhs)
		{
			lhs /= rhs;
			return lhs;
		}

		/**
		* @brief Addition operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator+(TPoint3<T> lhs, const T& rhs)
		{
			lhs += rhs;
			return lhs;;
		}

		/**
		* @brief Subtraction operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator-(TPoint3<T> lhs, const T& rhs)
		{
			lhs -= rhs;
			return lhs;
		}

		/**
		* @brief Multiplication operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator*(TPoint3<T> lhs, const T& rhs)
		{
			lhs *= rhs;
			return lhs;
		}

		/**
		* @brief Division operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TPoint3<T> operator/(TPoint3<T> lhs, const T& rhs)
		{
			lhs /= rhs;
			return lhs;
		}

		/**
		* @brief Subcript operator.
		*
		* Index values 0, 1, 2 are equal to x, y, z, respectively.
		* @param idx array index
		* @return stored coodinate value described by the array index
		*/
		T& operator[](size_type idx)
		{
			return this->_rgCoord[idx];
		}

		/**
		* @brief Subcript operator.
		*
		* Index values 0, 1, 2 are equal to x, y, z, respectively.
		* @param idx array index
		* @return stored coodinate value described by the array index (const)
		*/
		const T& operator[](size_type idx) const
		{
			return this->_rgCoord[idx];
		}

		/**
		* @brief Equality operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs and rhs are equal, FALSE otherwise
		*/
		friend bool operator==(const TPoint3<T>& lhs, const TPoint3<T>& rhs)
		{
			if (std::abs(lhs.x() - rhs.x()) < POINTVECTOR_EQUALITY_TOL &&
				std::abs(lhs.y() - rhs.y()) < POINTVECTOR_EQUALITY_TOL &&
				std::abs(lhs.z() - rhs.z()) < POINTVECTOR_EQUALITY_TOL)
			{
				return true;
			}
			return false;
		}

		/**
		* @brief Inequality operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs and rhs are not equal, FALSE otherwise
		*/
		friend bool operator!=(const TPoint3<T>& lhs, const TPoint3<T>& rhs)
		{
			return !(lhs == rhs);
		}

		/**
		* @brief Getter for x-coordinate.
		* @return coordinate value
		*/
		T x()
		{
			return this->_rgCoord[0];
		}

		/**
		* @brief Getter for x-coordinate (const).
		* @return coordinate value
		*/
		const T x() const
		{
			return this->_rgCoord[0];
		}

		/**
		* @brief Getter for y-coordinate.
		* @return coordinate value
		*/
		T y()
		{
			return this->_rgCoord[1];
		}

		/**
		* @brief Getter for y-coordinate (const).
		* @return coordinate value
		*/
		const T y() const
		{
			return this->_rgCoord[1];
		}

		/**
		* @brief Getter for z-coordinate.
		* @return coordinate value
		*/
		T z()
		{
			return this->_rgCoord[2];
		}

		/**
		* @brief Getter for z-coordinate (const).
		* @return coordinate value
		*/
		const T z() const
		{
			return this->_rgCoord[2];
		}

		/**
		* @brief Setter for x-coordinate.
		* @param value new coordinate value
		*/
		void x(T value)
		{
			this->_rgCoord[0] = value;
		}

		/**
		* @brief Setter for y-coordinate.
		* @param value new coordinate value
		*/
		void y(T value)
		{
			this->_rgCoord[1] = value;
		}

		/**
		* @brief Setter for z-coordinate.
		* @param value new coordinate value
		*/
		void z(T value)
		{
			this->_rgCoord[2] = value;
		}

		/**
		* @brief Setter for x-coordinate.
		* @param value new coordinate value
		*/
		void set_x(T value)
		{
			this->_rgCoord[0] = value;
		}

		/**
		* @brief Setter for y-coordinate.
		* @param value new coordinate value
		*/
		void set_y(T value)
		{
			this->_rgCoord[1] = value;
		}

		/**
		* @brief Setter for z-coordinate.
		* @param value new coordinate value
		*/
		void set_z(T value)
		{
			this->_rgCoord[2] = value;
		}

		/**
		* @brief Returns a pointer to the internally contained data array.
		* @return pointer to the array storing the coordinate values
		*/
		T* data_ptr()
		{
			return this->_rgCoord;
		}

	private:
		T _rgCoord[3]; /**< array which stores the coordinate values */
	};

	template <typename T>
	class TVector3
	{
	public:
		using size_type = unsigned int; /**< Default size type for the TVector3 class */
		using value_type = T; /**< Default value type for the TVector3 class */

		/**
		* @brief Default constructor.
		*
		* Sets start and end points to zero.
		*/
		TVector3()
		{
			this->_mStart = TPoint3<T>(0.0);
			this->_mEnd = TPoint3<T>(0.0);
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets start point to origin and end point to the input value.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param end_pt an array with size of 3 representing the coordinates of the end point
		*/
		explicit TVector3(T end_pt[3])
		{
			this->_mStart = TPoint3<T>(0.0);
			this->_mEnd = TPoint3<T>(end_pt);
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets start point to origin and end point to the input value.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param end_pt end point
		*/
		explicit TVector3(TPoint3<T> end_pt)
		{
			this->_mStart = TPoint3<T>(0.0);
			this->_mEnd = end_pt;
		}

		/**
		* @brief Conversion constructor.
		*
		* Sets start point and end point to the input values.
		* @see http://en.cppreference.com/w/cpp/language/explicit
		* @param start_pt start point of the vector
		* @param end_pt end point of the vector
		*/
		explicit TVector3(TPoint3<T> start_pt, TPoint3<T> end_pt)
		{
			this->_mStart = start_pt;
			this->_mEnd = end_pt;
		}

		/**
		* @brief Copy constructor.
		* @param rhs object to be copied
		*/
		TVector3(const TVector3<T>& rhs)
		{
			this->_mStart = rhs._mStart;
			this->_mEnd = rhs._mEnd;
		}

		/**
		* @brief Default destructor.
		*/
		~TVector3()
		{
			// Empty destructor
		}

		/**
		* @brief Copy assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TVector3<T>& operator=(const TVector3<T>& rhs)
		{
			// Check for self assignment
			if (this != &rhs)
			{
				this->_mStart = rhs._mStart;
				this->_mEnd = rhs._mEnd;
			}
			return *this;
		}

		/**
		* @brief Addition assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TVector3<T>& operator+=(const TVector3<T>& rhs)
		{
			this->_mEnd = rhs._mEnd;
			return *this;
		}

		/**
		* @brief Subtraction assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TVector3<T>& operator-=(const TVector3<T>& rhs)
		{
			this->_mEnd = rhs._mEnd * -1;
			return *this;
		}

		/**
		* @brief Multiplication assignment operator.
		* @param rhs object on the right
		* @return object on the left
		*/
		TVector3<T> operator*=(const TVector3<T>& rhs)
		{
			return this->cross(rhs);
		}

		/**
		* @brief Multiplication assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TVector3<T>& operator*=(const T& rhs)
		{
			this->_mEnd *= rhs;
			return *this;
		}

		/**
		* @brief Division assignment operator.
		* @param rhs primitive data type on the right
		* @return object on the left
		*/
		TVector3<T>& operator/=(const T& rhs)
		{
			this->_mEnd /= rhs;
			return *this;
		}

		/**
		* @brief Addition operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TVector3<T> operator+(TVector3<T> lhs, const TVector3<T>& rhs)
		{
			lhs += rhs;
			return lhs;
		}

		/**
		* @brief Subtraction operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TVector3<T> operator-(TVector3<T> lhs, const TVector3<T>& rhs)
		{
			lhs -= rhs;
			return lhs;
		}

		/**
		* @brief Multiplication operator.
		* @param lhs object on the left (to be updated)
		* @param rhs object on the right
		* @return updated object on the left
		*/
		friend TVector3<T> operator*(TVector3<T> lhs, const TVector3<T>& rhs)
		{
			return lhs.cross(rhs);
		}

		/**
		* @brief Multiplication operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TVector3<T> operator*(TVector3<T> lhs, const T& rhs)
		{
			lhs *= rhs;
			return lhs;
		}

		/**
		* @brief Division operator.
		* @param lhs object on the left (to be updated)
		* @param rhs primitive data type on the right
		* @return updated object on the left
		*/
		friend TVector3<T> operator/(TVector3<T> lhs, const T& rhs)
		{
			lhs /= rhs;
			return lhs;
		}

		/**
		* @brief Equality operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if the magnitudes of lhs and rhs are equal in the resolution tolerance, FALSE otherwise
		*/
		friend bool operator==(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			if (std::abs(lhs.magnitude() - rhs.magnitude()) < POINTVECTOR_EQUALITY_TOL)
				return true;
			return false;
		}

		/**
		* @brief Inequality operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs and rhs are not equal, FALSE otherwise
		*/
		friend bool operator!=(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			return !(lhs == rhs);
		}

		/**
		* @brief Less than operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs is less than rhs, FALSE otherwise
		*/
		friend bool operator<(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			if (lhs.magnitude() < rhs.magnitude())
				return true;
			return false;
		}

		/**
		* @brief Greater than operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs is greater than rhs, FALSE otherwise
		*/
		friend bool operator>(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			return (rhs < lhs);
		}

		/**
		* @brief Less than and equal to operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs is less than and equal to rhs, FALSE otherwise
		*/
		friend bool operator<=(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			return !(lhs > lhs);
		}

		/**
		* @brief Greater than and equal to operator.
		* @param lhs object on the left side of the operator
		* @param rhs object on the right side of the operator
		* @return TRUE if lhs is greater than and equal to rhs, FALSE otherwise
		*/
		friend bool operator>=(const TVector3<T>& lhs, const TVector3<T>& rhs)
		{
			return !(lhs < lhs);
		}

		/**
		* @brief Getter for x-coordinate.
		* @return coordinate value
		*/
		T x()
		{
			return (this->_mEnd.x() - this->_mStart.x());
		}

		/**
		* @brief Getter for x-coordinate (const).
		* @return coordinate value
		*/
		const T x() const
		{
			return (this->_mEnd.x() - this->_mStart.x());
		}

		/**
		* @brief Getter for y-coordinate.
		* @return coordinate value
		*/
		T y()
		{
			return (this->_mEnd.y() - this->_mStart.y());
		}

		/**
		* @brief Getter for y-coordinate (const).
		* @return coordinate value
		*/
		const T y() const
		{
			return (this->_mEnd.y() - this->_mStart.y());
		}

		/**
		* @brief Getter for z-coordinate.
		* @return coordinate value
		*/
		T z()
		{
			return (this->_mEnd.z() - this->_mStart.z());
		}

		/**
		* @brief Getter for z-coordinate (const).
		* @return coordinate value
		*/
		const T z() const
		{
			return (this->_mEnd.z() - this->_mStart.z());
		}

		/**
		* @brief Setter for start point.
		* @param value a 3D point
		*/
		void start_pt(TPoint3<T> value)
		{
			this->_mStart = value;
		}

		/**
		* @brief Getter for start point.
		* @return reference to the start point
		*/
		TPoint3<T>* start_pt()
		{
			return &this->_mStart;
		}

		/**
		* @brief Setter for end point.
		* @param value a 3D point
		*/
		void end_pt(TPoint3<T> value)
		{
			this->_mEnd = value;
		}

		/**
		* @brief Getter for end point.
		* @return reference to the end point
		*/
		TPoint3<T>* end_pt()
		{
			return &this->_mEnd;
		}

		/**
		* @brief Cross product.
		* @param vect vector to cross product
		* @return cross product of this vector and the input vector
		*/
		TVector3<T> cross(const TVector3<T>& vect)
		{
			T s[3];
			s[0] = ((this->y() * vect.z()) - (this->z() * vect.y()));
			s[1] = ((this->z() * vect.x()) - (this->x() * vect.z()));
			s[2] = ((this->x() * vect.y()) - (this->y() * vect.x()));
			return TVector3<T>(s);
		}

		/**
		* @brief Dot product.
		* @param vect vector to dot product
		* @return dot product of this vector and the input vector
		*/
		T dot(const TVector3<T>& vect)
		{
			T prod = (this->x() * vect.x()) + (this->y() * vect.y()) + (this->z() * vect.z());
			return prod;
		}

		/**
		* @brief Distance between two vectors.
		* @param vect vector to measure distance
		* @return the distance between this vector and the input vector
		*/
		T distance(const TVector3<T>& vect)
		{
			T dist = std::sqrt(std::pow(vect.x() - this->x(), 2) + std::pow(vect.y() - this->y(), 2) + std::pow(vect.z() - this->z(), 2));
			return dist;
		}

		/**
		* @brief Normalizes the vector and returns a new vector.
		* @return normalized vector
		*/
		TVector3<T> normalize()
		{
			T magn = this->magnitude();
			T s[3]; s[0] = T(0.0); s[1] = T(0.0); s[2] = T(0.0);
			if (magn != 0)
			{
				s[0] = this->x() / magn;
				s[1] = this->y() / magn;
				s[2] = this->z() / magn;
			}
			return TVector3<T>(s);
		}

		/**
		* @brief Calculates the magnitude of the vector.
		* @return the magnitude of the current vector object
		*/
		T magnitude()
		{
			T magn = std::sqrt(std::pow(this->x(), 2) + std::pow(this->y(), 2) + std::pow(this->z(), 2));
			return magn;
		}

		/**
		* @brief Checks if this vector is parallel to the input vector.
		* @param vect input vector
		*/
		bool parallel(const TVector3<T>& vect)
		{
			if (std::abs(this->dot(vect) - 0.0) < POINTVECTOR_EQUALITY_TOL)
				return true;
			return false;
		}

	private:
		TPoint3<T> _mStart; /**< Start point of the vector */
		TPoint3<T> _mEnd; /**< End point of the vector */
	};

}

#endif // !POINTVECTOR_HXX
