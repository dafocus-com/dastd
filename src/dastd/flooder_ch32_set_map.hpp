/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 11-AUG-2023
**/
#pragma once
#include "flooder_ch32_set.hpp"
#include <map>

namespace dastd {
	/// @brief Implementation of a `flooder_ch32_set` with an internal map
	class flooder_ch32_set_map: public flooder_ch32_set {
		public:
			/// @brief Given a string_id, returns the related string
			///
			/// @param strid String-id of the desired string
			///
			/// @return Returned string; the returned object is `valid()' if the requested `strid` has been found.
			virtual flooder_ch32_relay get(string_id strid) const;

			/// @brief Register a string_id/flooder_ch32 association
			///
			/// @param strid String-id of the desired string
			/// @param txt Associated text
			void set(string_id strid, flooder_ch32_relay txt) {m_string_map.insert({strid, txt});}

			/// @brief Register a string_id/flooder_ch32 association
			///
			/// @param strid String-id of the desired string
			/// @param txt Associated text
			void set(string_id strid, const char32_t* string_z) {m_string_map.insert({strid, flooder_ch32_relay(std::make_unique<flooder_ch32_conststr<char32_t>>(string_z))});}

		private:
			/// @brief Map that connects a string_id to a flooder_ch32
			std::map<string_id, flooder_ch32_relay> m_string_map;
};

/// Given a string_id, returns the related string
inline flooder_ch32_relay flooder_ch32_set_map::get(string_id strid) const
{
	auto iter = m_string_map.find(strid);
	if (iter == m_string_map.end()) return flooder_ch32_relay();
	return iter->second;
}

} // namespace dastd
