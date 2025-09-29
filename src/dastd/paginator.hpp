/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 28-SEP-2023
**/
#pragma once
#include "defs.hpp"
#include "char32string.hpp"
#include "container.hpp"
#include "wordwrappable.hpp"
#include <sstream>

namespace dastd {

/// @brief Range of characters within a paginator
class paginator_range {
	public:
		/// @brief Constructor
		/// @param true_line_number Position of this line in the m_true_lines array
		/// @param sub_line_number  Line number within the true line
		/// @param range          First character of the range and length of the range
		paginator_range(size_t true_line_number, size_t sub_line_number, const wordwrap_range& range):
			m_true_line_number(true_line_number),
			m_sub_line_number(sub_line_number),
			m_range(range) {}

		/// @brief Constructor
		paginator_range() {}

		/// @brief Return the range
		const wordwrap_range& get_range() const {return m_range;}

		/// @brief Return the position of this line in the m_true_lines array
		///
		/// This would be the line number if the lines were never broken.
		/// 0 is the first line.
		size_t get_true_line_number() const {return m_true_line_number;}

		/// @brief Line number within the true line
		///
		/// The true line might have been split in multiple lines to
		/// fit in the available with. This tells the line number within
		/// the true line: 0 is the first line generated, 1 the second and
		/// so on.
		size_t get_sub_line_number() const {return m_sub_line_number;}

	private:
		/// @brief Position of this line in the m_true_lines array
		size_t m_true_line_number;

		/// @brief Line number within the true line
		///
		/// The true line might have been split in multiple lines to
		/// fit in the available with. This tells the line number within
		/// the true line: 0 is the first line generated, 1 the second and
		/// so on.
		size_t m_sub_line_number;

		/// @brief First character of the range
		wordwrap_range m_range;
};

/// @brief Implements a container made of lines.
///
/// The lines are wrapped to fit the indicated width
class paginator {
	public:
		/// @brief Add one or more lines, separated by '\n'
		void add_line(std::shared_ptr<wordwrappable> wrappable_text_line);

		/// @brief Set the number of columns and rebuild
		void set_columns(uint32_t columns);

		/// @brief Return the number of lines
		size_t get_split_lines_count() const {return m_split_lines.size();}

		/// @brief Return the i-th split line
		const paginator_range& get_split_line(size_t pos) const {return m_split_lines[pos];}

		/// @brief Get the number of columns
		uint32_t get_columns() const {return m_columns;}

		/// @brief Geth the wordwrappable object at given position
		std::shared_ptr<const wordwrappable> get_word_wrappable(size_t true_line_number) const {return m_true_lines[true_line_number];}

		/// @brief Clear the contents
		void clear() {m_split_lines.clear(); m_true_lines.clear();}

	private:
		/// @brief Rebuild all wrapped lines
		void rebuild();

		/// @brief Append a single line
		///
		/// Split the n-th line into the split lines
		void generate_split_line(size_t true_line_no);

		/// @brief Array of strings, one per true line
		///
		/// This contains the lines, no matter how long they are.
		std::vector<std::shared_ptr<wordwrappable>> m_true_lines;

		/// @brief Array of strings, one per split line
		///
		/// This contains the lines that originate from the true lines.
		/// This might contain more lines depending on wrapping.
		std::vector<paginator_range> m_split_lines;

		/// @brief Text width for word wrapping
		uint32_t m_columns = 0;
};

//------------------------------------------------------------------------------
// (brief) Add one or more lines, separated by '\n'
//------------------------------------------------------------------------------
inline void paginator::add_line(std::shared_ptr<wordwrappable> wrappable_text_line)
{
	m_true_lines.push_back(wrappable_text_line);
	generate_split_line(m_true_lines.size() - 1);
}

//------------------------------------------------------------------------------
// (brief) Append a single line
//
// Split the n-th line into the split lines
//------------------------------------------------------------------------------
inline void paginator::generate_split_line(size_t true_line_no)
{
	if (m_columns == 0) return;
	std::vector<wordwrap_range> ranges;
	m_true_lines[true_line_no]->do_wordwrap(m_columns, ranges);

	size_t sub_line_no = 0;
	for (auto & range: ranges) {
		m_split_lines.emplace_back(true_line_no, sub_line_no, range);
		sub_line_no++;
	}
}


//------------------------------------------------------------------------------
// (brief) Rebuild all wrapped lines
//------------------------------------------------------------------------------
inline void paginator::rebuild()
{
	m_split_lines.clear();
	size_t i;
	for (i=0; i<m_true_lines.size(); i++) {
		generate_split_line(i);
	}
}

//------------------------------------------------------------------------------
// (brief) Set the number of columns and rebuild
//------------------------------------------------------------------------------
inline void paginator::set_columns(uint32_t columns)
{
	if (columns == m_columns) return;
	m_columns = columns;
	rebuild();
}

}
