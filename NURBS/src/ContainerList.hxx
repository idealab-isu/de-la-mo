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

#ifndef CONTAINERLIST_HXX
#define CONTAINERLIST_HXX

// CPP includes
#include <cstddef>
#include <iostream>
#include <memory>
#include <cstdlib>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>

#define CONTAINER_DEFAULT_ALLOC_SZ 8 /**< Default allocation size for the container */

namespace delamo
{
	template <typename T>
	class List
	{
	public:
		using size_type = unsigned int; /**< Default size type for the container class */
		using value_type = T; /**< Default value type for the container class */
		using iterator = T*; /**< Iterator type for the container class */
		using const_iterator = const T*; /**< Const Iterator type for the container class */

		/**
		 * @brief Default constructor.
		 *
		 * Creates an empty container.
		 */
		List()
		{
			this->init_vars(0, T());
		}

		/**
		 * @brief Creates a container from an initializer list.
		 * @param lst a list, i.e. {1.0, 2.3, 3.7, 4.5}
		 */
		List(std::initializer_list<T> lst)
		{
			this->_mSize = size_type(lst.size());
			this->_mSpace = this->_mSize;
			this->_pElem = new T[this->_mSize];
			// Copy list elements to the internal pointer array
			std::copy(lst.begin(), lst.end(), this->_pElem);
		}

		/**
		 * @brief Creates a new container with "s" elements.
		 * @param s number of elements inside the container
		 * @param value default value of the elements
		 */
		explicit List(size_type s, T value)
		{
			this->init_vars(s, value);
		}

		/**
		 * @brief Creates a container from the pointer array.
		 * @param ptr_elem the pointer array
		 * @param ptr_elem_size size of this pointer array
		 */
		explicit List(T* ptr_elem, size_type ptr_elem_size)
		{
			this->_mSize = ptr_elem_size;
			this->_mSpace = this->_mSize;
			this->_pElem = new T[this->_mSize];
			std::copy(ptr_elem, ptr_elem + ptr_elem_size, this->_pElem);
		}

	  /** WARNING DANGEROUS: Create a list that shadows 
              a preexisting array... only applicable if T is a pointer type */
	      List(T first_elem, size_type num_elems)
	        {
		  this->_mSize=num_elems;
		  this->_mSpace=this->_mSize;
		  this->_pElem=new T[this->_mSize];
		  for (size_type cnt=0;cnt < num_elems;cnt++) {
		    this->_pElem[cnt]=first_elem+cnt;
		  }
		}
	  
		/**
		 * @brief Copy constructor.
		 * @param rhs object to be copied
		 */
		List(const List<T>& rhs)
		{
			// Deallocate old memory
			if (this->_pElem != nullptr)
			{
				delete[] this->_pElem;
				this->_pElem = nullptr;
			}

			// Start copying
			this->_mSize = rhs._mSize;
			this->_mSpace = rhs._mSize;
			if (this->_mSize != 0)
			{
				this->_pElem = new T[this->_mSize];
				std::copy(rhs._pElem, rhs._pElem + rhs._mSize, this->_pElem);
			}
		}

		/**
		 * @brief Move constructor.
		 * @param rhs object to be moved
		 */
		List(List<T>&& rhs)
		{
			this->_mSize = rhs._mSize;
			this->_mSpace = rhs._mSpace;
			this->_pElem = rhs._pElem;
			rhs._pElem = nullptr;
			rhs._mSize = 0;
			rhs._mSpace = 0;
		}

		/**
		 * @brief Default destructor.
		 */
		~List()
		{
			if (this->_pElem != nullptr)
				delete[] this->_pElem;
			this->_pElem = nullptr;
		}

		/**
		 * @brief Copy assignment operator.
		 * @param rhs object on the right
		 * @return object on the left
		 */
		List<T>& operator=(const List<T>& rhs)
		{
			// Check for self assignment
			if (this != &rhs)
			{
				// Deallocate old memory
				if (this->_pElem != nullptr)
					delete[] this->_pElem;

				// Start copying
				this->_mSize = rhs._mSize;
				this->_mSpace = rhs._mSize;
				this->_pElem = new T[this->_mSize];
				std::copy(rhs._pElem, rhs._pElem + rhs._mSize, this->_pElem);
			}
			return *this;
		}

		/**
		 * @brief Move assignment operator.
		 * @param rhs object on the right
		 * @return object on the left
		 */
		List<T>& operator=(List<T>&& rhs)
		{
			// Check for self assignment
			if (this != &rhs)
			{
				// Deallocate existing memory
				if (this->_pElem != nullptr)
					delete[] this->_pElem;

				// Start moving
				this->_mSize = rhs._mSize;
				this->_mSpace = rhs._mSpace;
				this->_pElem = rhs._pElem;
				rhs._pElem = nullptr;
				rhs._mSize = 0;
				rhs._mSpace = 0;
			}
			return *this;
		}

		/**
		 * @brief Subcript operator for non-const values.
		 * @param idx array index
		 * @return stored value in the Container described by the array index
		 */
		T& operator[](size_type idx)
		{
			return this->_pElem[idx];
		}

		/**
		 * @brief Subcript operator for const values.
		 * @param idx array index
		 * @return stored value in the Container described by the array index
		 */
		const T& operator[](size_type idx) const
		{
			return this->_pElem[idx];
		}

		/**
		 * @brief Returns the pointer to the underlying element array.
		 * @return pointer to the underlying element strotage
		 */
		T* data()
		{
			return this->_pElem;
		}

		/**
		 * @brief Returns the const pointer to the underlying element array.
		 * @return pointer to the underlying element strotage (const)
		 */
		const T* data() const
		{
			return this->_pElem;
		}

		/**
		 * @brief Returns the size of the container, a.k.a. number of elements stored inside.
		 * @return number of elements stored inside the container
		 */
		size_type size()
		{
			return this->_mSize;
		}

		/**
		 * @brief Deletes the data inside the pointer and set the counters to zero.
		 */
		void clear()
		{
			if (this->_pElem != nullptr)
			{
				delete[] this->_pElem;
				this->_pElem = nullptr;
			}
			this->_mSize = 0;
			this->_mSpace = 0;
		}

		/**
		 * @brief Checks whether the container is empty or not.
		 * return TRUE if empty, FALSE otherwise.
		 */
		bool empty()
		{
			if (this->_mSize == 0)
				return true;
			else
				return false;
		}

		/**
		* @brief Adds the input element at the front by copying it to the container.
		* @param elem element to be added
		*/
		void push_front(const T& elem)
		{
			// Minimize the allocated memory for the elements pointer array
			this->reset_alloc();

			// Use a temporary array for memory reallocation
			T* new_elems = new T[this->_mSize + 1];
			if (this->_pElem != nullptr)
			{
				for (size_type i = 1; i < this->_mSize + 1; i++)
				{
					new_elems[i] = this->_pElem[i - 1];
				}
			}

			// Set the first element
			new_elems[0] = elem;
			this->_mSize++;

			// Copy the temporarpy array back to class
			if (this->_pElem != nullptr)
			{
				delete[] this->_pElem;
				this->_pElem = nullptr;
			}
			this->_pElem = new T[this->_mSize];
			std::copy(new_elems, new_elems + this->_mSize, this->_pElem);

			// Set the allocated space
			this->_mSpace = this->_mSize;
		}

		/**
		* @brief Adds the input element at the front by copying it to the container.
		* @param elem_list elements to be added
		*/
		void push_front(List<T>& elem_list)
		{
			// Minimize the allocated memory for the elements pointer array
			this->reset_alloc();

			size_type list_size = elem_list.size();

			size_type ic = 0;

			// Use a temporary array for memory reallocation
			T* new_elems = new T[this->_mSize + list_size];
			if (this->_pElem != nullptr)
			{
				for (size_type i = list_size; i < this->_mSize + list_size; i++)
				{
					new_elems[i] = this->_pElem[ic];
					ic++;
				}
			}

			// Set the first elements
			ic = 0;
			for (auto el : elem_list)
			{
				new_elems[ic] = el;
				ic++;
			}
			this->_mSize += list_size;

			// Copy the temporarpy array back to class
			if (this->_pElem != nullptr)
			{
				delete[] this->_pElem;
				this->_pElem = nullptr;
			}
			this->_pElem = new T[this->_mSize];
			std::copy(new_elems, new_elems + this->_mSize, this->_pElem);

			// Set the allocated space
			this->_mSpace = this->_mSize;
		}

		/**
		 * @brief Adds the input element at the end by copying it to the container.
		 * @param elem element to be added
		 */
		void push_back(const T& elem)
		{
			// Allocate some additional memory, if necessary
			this->allocate();

			// Add the new element to the pointer array 
			this->_pElem[this->_mSize] = elem;
			this->_mSize++;
		}

		/**
		* @brief Adds the input element at the end by copying it to the container.
		* @param elem_list elements to be added
		*/
		void push_back(List<T>& elem_list)
		{
			// Allocate some more memory, at least the size of the list
			this->reserve(this->_mSpace + elem_list._mSize);

			// Add new elements to the pointer array
			for (auto elem : elem_list)
			{
				this->_pElem[this->_mSize] = elem;
				this->_mSize++;
			}
		}

		/**
		 * @brief Adds the input element at the end by copying it to the container.
		 *
		 * Alias of push_back()
		 * @param elem element to be added
		 */
		void add(const T& elem)
		{
			this->push_back(elem);
		}

		/**
		 * @brief Adds the input element at the end by copying it to the container.
		 *
		 * Alias of push_back()
		 * @param elem element to be added
		 */
		void append(const T& elem)
		{
			this->push_back(elem);
		}


		/**
		* @brief Adds the input element at the front by copying it to the container.
		* @@return first element of the list
		*/
		T pop_front()
		{
			// Minimize the allocated memory for the elements pointer array
			this->reset_alloc();

			// Use a temporary array for memory reallocation
			T* new_elems = new T[this->_mSize - 1];
			if (this->_pElem != nullptr)
				std::copy(this->_pElem + 1, this->_pElem + this->_mSize, new_elems);

			// Get the return value
			T retval = this->_pElem[0];
			this->_mSize--;

			// Copy the temporarpy array back to class
			if (this->_pElem != nullptr)
			{
				delete[] this->_pElem;
				this->_pElem = nullptr;
			}
			this->_pElem = new T[this->_mSize];
			std::copy(new_elems, new_elems + this->_mSize, this->_pElem);

			// Set the allocated space
			this->_mSpace = this->_mSize;

			// Retrun the last element
			return retval;
		}

		/**
		 * @brief Returns and deletes the last element.
		 * @return last element of the list
		 */
		T pop_back()
		{
			// Remove the last element from the elements array
			T retval = this->_pElem[this->_mSize - 1];
			this->_pElem[this->_mSize - 1] = T();
			this->_mSize--;

			// Deallocate some of the allocated memory inside the container, if necessary
			this->deallocate();

			// Retrun the last element
			return retval;
		}

		/**
		 * @brief Returns and deletes the last element.
		 *
		 * Alias of pop_back()
		 * @return last element of the list
		 */
		T remove()
		{
			return this->pop_back();
		}

		/**
		 * @brief Returns the element reference at the input index.
		 * @param idx array index
		 * @return stored value in the Container described by the array index
		 */
		T& at(size_type idx)
		{
			if (idx < 0 || idx >= this->_mSize)
				throw std::out_of_range("Index value exceeds container size");
			return this->_pElem[idx];
		}

		/**
		 * @brief Reserves some additional memory for future push backs.
		 * @param newalloc number of elements to be reserved in the memory
		 * @return boolean value to check if the reserve is successful or not
		 */
		bool reserve(size_type newalloc)
		{
			// Never decresase the allocated memory
			if (newalloc <= this->capacity())
				return false;

			// Allocate new memory
			T* temp_ptr = new T[newalloc];

			// Copy old elements into the new memory
			for (size_type i = 0; i < this->_mSize; i++)
			{
				temp_ptr[i] = this->_pElem[i];
			}

			// Delete old memory
			delete[] this->_pElem;

			// Set new memory and reserved space
			this->_pElem = temp_ptr;
			this->_mSpace = newalloc;

			return true;
		}

		/**
		 * @brief Returns the allocated capacity of this container from the memory.
		 * @return the capacity of the container as an integer
		 */
		size_type capacity() const
		{
			return this->_mSpace;
		}

		/**
		 * @brief Resizes this container.
		 * @param newsize number of elements to be allocated inside the container
		 */
		void resize(size_type newsize)
		{
			if (this->reserve(newsize))
			{
				for (size_type i = this->_mSize; i < newsize; i++)
					this->_pElem[i] = T();
			}
		}

		/**
		 * @brief Indicates the start of the pointer array stored inside the container.
		 *
		 * According to the C++11 Standard, begin() must return the first element of the pointer array.
		 * @return the first element stored inside the container
		 */
		iterator begin()
		{
			return &this->_pElem[0];
		}

		/**
		 * @brief Indicates the start of the pointer array stored inside the container.
		 *
		 * According to the C++11 Standard, begin() must return the first element of the pointer array.
		 * @return the first element stored inside the container
		 */
		const_iterator begin() const
		{
			return &this->_pElem[0];
		}

		/**
		 * @brief Indicates the end of the pointer array by returning a "not-found" value.
		 *
		 * According to the C++11 Standard, end() must return a not-found value.
		 * @return a not-found value
		 */
		iterator end()
		{
			return &this->_pElem[this->_mSize];
		}

		/**
		 * @brief Indicates the end of the pointer array by returning a "not-found" value.
		 *
		 * According to the C++11 Standard, end() must return a not-found value.
		 * @return a not-found value
		 */
		const_iterator end() const
		{
			return &this->_pElem[this->_mSize];
		}

		/**
		 * @brief Returns the first element of the container.
		 * @return the element stored inside the container
		 */
		iterator first()
		{
			return &this->_pElem[0];
		}

		/**
		 * @brief Returns the last element of the container.
		 * @return the element stored inside the container
		 */
		iterator last()
		{
			return &this->_pElem[this->_mSize - 1];
		}

		/**
		 * @brief Reverses the order of the elements of the container.
		 *
		 * Flips the pointer array upside down, so that the last element becomes the first element and so.
		 */
		void reverse()
		{
			if (this->_pElem != nullptr)
			{
				T* rev_elems = new T[this->_mSize];
				for (size_type i = 0; i < this->_mSize; i++)
					rev_elems[i] = this->_pElem[this->_mSize - 1 - i];

				delete[] this->_pElem; this->_pElem = nullptr;
				this->_pElem = new T[this->_mSize];

				std::copy(rev_elems, rev_elems + this->_mSize, this->_pElem);
				delete[] rev_elems;	rev_elems = nullptr;
			}
		}

		/**
		 * @brief Reduces the amount of memory allocated by the container.
		 */
		void reset_alloc()
		{
			if (this->_pElem != nullptr)
			{
				T* temp_ptr = new T[this->_mSize];
				std::copy(this->_pElem, this->_pElem + this->_mSize, temp_ptr);

				delete[] this->_pElem; this->_pElem = nullptr;
				this->_pElem = new T[this->_mSize];

				std::copy(temp_ptr, temp_ptr + this->_mSize, this->_pElem);
				delete[] temp_ptr; temp_ptr = nullptr;

				this->_mSpace = this->_mSize;
			}
		}

	protected:

		/**
		 * @brief Initializes the class variables, helper for the constructors.
		 *
		 * Similar functionality of std::allocator<T>::construct()
		 * @param s number of initial elements in the pointer array
		 * @param value default value of the elements
		 */
		void init_vars(size_type s, T value)
		{
			this->_mSize = s;
			this->_mSpace = s;
			// Check we are initializing an empty container
			if (s == 0)
			{
				// If an empty container, initialize elements array to a null pointer
				this->_pElem = nullptr;
			}
			else
			{
				// If not an empty container, create a new instance of the elements array and initialize to zero
				this->_pElem = new T[s];
				for (size_type i = 0; i < this->_mSize; i++)
					this->_pElem[i] = value;
			}
		}

		/**
		 * @brief Allocates memory for the container.
		 *
		 * Similar functionality of std::allocator<T>::allocate()
		 */
		void allocate()
		{
			// If there is no space, allocate some
			if (this->_mSpace == 0)
				this->reserve(CONTAINER_DEFAULT_ALLOC_SZ);
			else if (this->_mSize == this->_mSpace)
				this->reserve(this->_mSpace + CONTAINER_DEFAULT_ALLOC_SZ);
		}

		/**
		 * @brief Deallocates memory from the container.
		 *
		 * Similar functionality of std::allocator<T>::deallocate()
		 */
		void deallocate()
		{
			// Check allocated space is way too bigger than the container size
			if (this->_mSpace > 2 * this->_mSize)
			{
				// Allocate a temporary pointer array having the container size
				T* temp_ptr = new T[this->_mSize];
				// Copy old elements into the temporary array
				std::copy(this->_pElem, this->_pElem + this->_mSize, temp_ptr);
				// Delete old elements
				delete[] this->_pElem;
				// Allocate new memory for the smaller elements array
				this->_pElem = new T[this->_mSize];
				// Copy old values into the smaller elements array
				std::copy(temp_ptr, temp_ptr + this->_mSize, this->_pElem);
				// Delete temporary pointer array
				delete[] temp_ptr;
				temp_ptr = nullptr;
				// The container capacity, now, should be equal to the container size
				this->_mSpace = this->_mSize;
			}
		}

	private:
		T* _pElem = nullptr; /**< The pointer array which stores the elements inside the container */
		size_type _mSize = 0; /**< The size of the pointer array */
		size_type _mSpace = 0; /**< The capacity of the container object */
	};
}

#endif // !CONTAINERLIST_HXX
