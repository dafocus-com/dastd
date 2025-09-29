/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 13-GEN-2024
**/
#pragma once
#include "defs.hpp"

namespace dastd {

	/// @brief 	It defines a simple container that holds a pointer to a typed	data buffer and an indication of its size.
	template<class T> struct binary_container {
		// Constructor
		binary_container(): m_data_pointer(NULL), m_number_of_elements(0) {}
		binary_container(T* data_pointer, size_t number_of_elements): m_data_pointer(data_pointer), m_number_of_elements(number_of_elements) {}

		// Set the value
		void Set(T* data_pointer, size_t number_of_elements) {m_data_pointer=data_pointer; m_number_of_elements=number_of_elements;}

		// Internal data
		T* m_data_pointer;
		size_t m_number_of_elements;
	};

	/// @brief 	It defines a simple container that holds a cibst pointer to a typed	data buffer and an indication of its size.
	template<class T> struct binary_container_const {
		// Constructor
		binary_container_const(): m_data_pointer(NULL), m_number_of_elements(0) {}
		binary_container_const(const T* data_pointer, size_t number_of_elements): m_data_pointer(data_pointer), m_number_of_elements(number_of_elements) {}

		// Set the value
		void Set(const T* data_pointer, size_t number_of_elements) {m_data_pointer=data_pointer; m_number_of_elements=number_of_elements;}

		// Internal data
		const T* m_data_pointer;
		size_t m_number_of_elements;
	};

} // namespace dastd
