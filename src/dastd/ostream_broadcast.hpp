/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 09-AUG-2023
**/
#pragma once
#include "ostream_basic.hpp"
#include <utility>
#include <map>
#include <limits>
#include <fstream>

namespace dastd {
	/// @brief Broadcasts the same output on multiple streams
	class ostream_broadcast: public ostream_basic {
		public:
			/// @brief Value that can be used to represent invalid stream IDs
			static constexpr uint32_t INVALID_STREAM_ID = std::numeric_limits<uint32_t>::max();

			/// @brief Write one character to the target stream
			virtual void write_char(char_type c) override;

			/// @brief Register a not-owned stream.
			///
			/// This method is to be used for streams whose life cycle
			/// is managed elsewhere, like for example "std::cout".
			/// These streams will not be deleted by this object
			///
			/// @param stream_not_owned Stream to be added
			/// @return Returns the stream-id assigned to this stream.
			uint32_t add_stream_not_owned(std::ostream& stream_not_owned) {m_max_id++; m_substreams.insert({m_max_id, substream(&stream_not_owned)}); return m_max_id;}

			/// @brief Register an owned stream.
			///
			/// This method is to be used for streams whose life cycle
			/// is managed by this object. This object will delete
			/// these objects when destructed.
			///
			/// @param stream_owned Stream to be added
			/// @return Returns the stream-id assigned to this stream.
			uint32_t add_stream_owned(std::shared_ptr<std::ostream> stream_owned) {m_max_id++; m_substreams.insert({m_max_id, substream(stream_owned)}); return m_max_id;}

			/// @brief Helper function that adds a `std::ofstream` file on disk
			/// @param file_name Name of the file to be created
			/// @param append    Set to `true` if output is to be appended, `false` otherwise
			/// @return Returns the stream-id assigned or INVALID_STREAM_ID in case the file can not be opened
			uint32_t add_stream_file(const std::string& file_name, bool append=true);

			/// @brief Delete the stream given its stream id or INVALID_STREAM_ID in case the file could not be opened
			void del_stream(uint32_t stream_id) {m_substreams.erase(stream_id);}

			/// @brief Remove all streams
			void clear() {m_substreams.clear();}

			/// @brief Implementation of the "flush" action
			///
			/// @return Return `false` in case of error, `true` if ok.
			virtual bool sync() override;

		protected:
			/// @brief Substream descriptor
			struct substream {
				/// @brief Constructor for not-owned stream
				substream(std::ostream* stream_not_owned): m_stream(stream_not_owned) {}

				/// @brief Constructor for owned stream
				substream(std::shared_ptr<std::ostream> stream_owned): m_stream_owned(stream_owned) {m_stream = stream_owned.get();}

				/// @brief Pointer to the substream to be used
				std::ostream* m_stream = nullptr;

				/// @brief Substream smartpointer in case it is owned
				std::shared_ptr<std::ostream> m_stream_owned;

				/// @brief True if enabled
				bool m_enabled = true;
			};

			/// @brief Number of stream ids assigned; used to assign new ids
			uint32_t m_max_id = 0;

			/// @brief Map of stream ids and their related substream descriptor
			std::map<uint32_t, substream> m_substreams;
	};

	// Write one character to the target stream
	inline void ostream_broadcast::write_char(char_type c) {
		for (auto & sub: m_substreams) {
			if (sub.second.m_enabled) sub.second.m_stream->put(c);
		}
	}

	// Implementation of the "flush" action
	inline bool ostream_broadcast::sync() {
		bool ok = true;
		for (auto & sub: m_substreams) {
			if (sub.second.m_enabled) {
				sub.second.m_stream->flush();
				if (sub.second.m_stream->bad()) ok=false;
			}
		}
		return ok;
	}

	// Helper function that adds a `std::ofstream` file on disk
	inline uint32_t ostream_broadcast::add_stream_file(const std::string& file_name, bool append)
	{
		std::shared_ptr<std::ofstream> file = std::make_shared<std::ofstream>(file_name, std::ios::out|(append ? std::ios::app : std::ios::trunc));
		if (file->good()) {
			return add_stream_owned(file);
		}
		return INVALID_STREAM_ID;
	}

} // namespace dastd
