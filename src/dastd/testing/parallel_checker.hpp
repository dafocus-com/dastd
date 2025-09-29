/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 26-SEP-2025
**/
#pragma once
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <limits>
#include <cassert>
#include "../fmt.hpp"
#include "../ostream_indent.hpp"

/**

# Parallel checker

The goal of the parallel checker is to to make sure that a parallel sequence of actions
has been executed correctly.


Example:

		// Fill the sequence
		parcheck::events_list<test_event_type> events;
		events.push_back("HELLO");
		events.push_back("WORLD");
		events.push_back("First");
		events.push_back("PIPPO");
		events.push_back("Second");
		events.push_back("FOOBAR");

		// Create the check sequence
		parcheck::check_item_sequence<test_event_type> seq;
		seq.expect("HELLO");
		seq.expect("WORLD");
	
		auto par1 = seq.expect_parallel();
		par1->expect("PIPPO");

		auto seq1 = par1->expect_sequence();
		seq1->expect("First");
		seq1->expect("Second");

		seq.expect("FOOBAR");

		// Create a `check_runtime` object that will run the test
		parcheck::check_runtime<test_event_type> runtime(seq, events);
		bool ok = runtime.run_check();

		// Print the result in debug readable form
		std::cout << runtime;


*/
namespace dastd::testing::parallel_checker {

/// @brief Value returned by the @ref match_item method when no match was possible
static constexpr size_t BAD_POSITION = std::numeric_limits<size_t>::max();


/// @brief This concept defines the characteristics that a type used as events must have
///
/// The check items, i.e. the values inserted and compared, must be printable on a `std::ostream`
/// with the `<<` operator and support the `==` equality operator.
template <class T>
concept event_concept = requires(const T& a, std::ostream& o) {
	{o << a} -> std::convertible_to<std::ostream&>;
	{a == a} -> std::same_as<bool>;
};

template<event_concept EVENT_TYPE> class check_item;


/// @brief Implements a list of events to be recorded by the code being executed
/// @note This list is mutex protected
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class events_list {
	private:
		/// @brief Type of the semaphore used in this context
		using mutex_t = std::mutex;
		
		/// @brief Type of the semaphore lock guard used in this context
		using mutex_lock_t = std::lock_guard<mutex_t>;

		/// @brief Mutex object to access this list
		mutex_t m_mutex;

		/// @brief List of events
		std::vector<EVENT_TYPE> m_events;

	public:
		/// @brief Add an event to the list
		/// @param ev Event to be added to the list
		void push_back(const EVENT_TYPE& ev) {mutex_lock_t lock(m_mutex); m_events.push_back(ev);}

		/// @brief Get the number of events in the events list
		/// @return Returns the number of events in the events list
		size_t size() const {return m_events.size();}

		/// @brief Access the event at the indicated position
		/// @param pos Position of the event being read (from 0 to `size()-1`)
		/// @return Returns a pointer to the event
		const EVENT_TYPE* operator[](const size_t pos) const {return &(m_events[pos]);}

		/// @brief Print the events in a form readable for debugging
		/// @param o Output stream
		void print(std::ostream& o) const;
};

/// @brief Runtime state
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class check_runtime {
	public:
		/// @brief Condirion of a @ref check_item when checking has been done
		enum class check_item_involvement_t {
			MATCHED,       ///< The @ref check_item was correctly matched
			MATCH_FAILED   ///< The @ref check_item match was attempted but failed
		};

	private:
		/// @brief Contains information about a @ref check_item to be used when
		///        logging to have detailed data.
		struct check_item_state {
			/// @brief How the @ref check_item has been involved
			check_item_involvement_t m_check_item_involvement = check_item_involvement_t::MATCH_FAILED;

			/// @brief Position where the match occurred in case of single items
			size_t m_matched_position = BAD_POSITION;

			/// @brief Report a match
			check_item_state(check_item_involvement_t check_item_involvement, size_t matched_position = BAD_POSITION):
				m_check_item_involvement(check_item_involvement), m_matched_position(matched_position) {}
		};

		/// @brief Describes the state of each event
		struct event_state {
			/// @brief Pointer to the event object
			const EVENT_TYPE* m_event_ptr = nullptr;

			/// @brief True if already used by someone
			bool m_consumed = false;

			/// @brief Constructor
			/// @param event_ptr Pointer to the event object
			event_state(const EVENT_TYPE* event_ptr): m_event_ptr(event_ptr), m_consumed(false) {}
		};

		/// @brief Array of event states
		std::vector<event_state> m_event_states;

		/// @brief State of the items that have been matched
		std::map<const check_item<EVENT_TYPE>*, check_item_state> m_item_states;

		/// @brief Load an events list into the runtime object
		/// @param events List of events
		void load_events_list(const events_list<EVENT_TYPE>& events);

		/// @brief Root of the expected items
		const check_item<EVENT_TYPE>& m_expected;

	public:
		/// @brief Constructor
		/// @param expected  Expected3
		/// @param events    List of events being checked
		check_runtime(const check_item<EVENT_TYPE>& expected, const events_list<EVENT_TYPE>& events);

		/// @brief Destructor
		virtual ~check_runtime() = default;

		/// @brief Check the indicated `check_item` against the `events`; return `true` if ok, false in case of failure
		/// @return Returns `true` if ok, `false` if failed
		bool run_check();

		/// @brief Print the events in a form readable for debugging
		/// @param o Output stream
		void print(std::ostream& o) const;

		/// @brief Number of events available
		/// @return Returns the number of events that are available
		size_t events_size() const {return m_event_states.size();}

		/// @brief Access the event at the given position
		/// @param pos Position in the events sequence
		/// @return Returns the event object
		const EVENT_TYPE& get_event(size_t pos) const {return *m_event_states[pos].m_event_ptr;}

		/// @brief Check if the event at the given position has been consumed
		/// @param pos Position in the events sequence
		/// @return Returns `true` if the event has been matched already
		bool event_already_consumed(size_t pos) const {return m_event_states[pos].m_consumed;}

		/// @brief Mark the indicated event as consumed
		/// @param pos Position in the events sequence
		void consume(size_t pos);

		/// @brief Report the result of a rule
		/// @param consuming_rule Rule involved
		/// @param How the @ref check_item has been involved
		/// @param Position where the match occurred in case of single items
		void report_matching_result(const check_item<EVENT_TYPE>* consuming_rule, check_item_involvement_t check_item_involvement, size_t matched_position);

		/// @brief Prints the information it knows about this item
		/// @param o The output stream
		/// @param item The @ref check_item to be prefixed
		void debug_print_check_item_header(std::ostream& o, const check_item<EVENT_TYPE>* item) const;

		/// @brief Starting from a position, find the first available usable event
		/// @param starting_pos Position it should start from
		/// @return Returns the position of the first available one (that could be `starting_pos`) or @ref BAD_POSITION
		///         in case no more elements are available.
		size_t get_first_unused(size_t starting_pos) const;

		/// @brief The `check_item` objects will call this method when they encounter an error
		/// @param expected Contains the expected `check_item` that was not matched
		/// @param started_from Position at which it started searching
		/// 
		/// Redefine this method to execute custom code when a rule could not
		/// match an event.
		virtual void report_missing_match([[maybe_unused]] const check_item<EVENT_TYPE>& expected, [[maybe_unused]] size_t started_from) {}

		/// @brief Called for every event that no rule expected
		/// @param position Position of the unmatched event
		/// 
		/// Redefine this method to execute custom code every time an unmatched
		/// event is detected.
		virtual void report_unmatched_events([[maybe_unused]] size_t position) {}
};

/// @brief Base class for the items to be checked
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class check_item {
	public:
		/// @brief Virtual destructor
		virtual ~check_item() = default;

		/// @brief Starting from event at position `first`, tries to match its contents.
		/// 
		/// This method will receive a position named `first` in the sequence of events.
		/// The event can be accessed by calling `runtime.get_event(i)`; the internal code
		/// should make sure that the event has not been already matched by some other rule
		/// by calling `runtime.event_already_consumed(i)`.
		/// 
		/// Once used, call `runtime.consume(i)` to mark the event as used.
		/// 
		/// @param first    Position of the first item to be matched
		/// @param runtime  Runtime object containing the events
		/// @return Returns the position of the first event after the matched ones or @ref BAD_POSITION in case of match error
		virtual size_t match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const = 0;

		/// @brief Print the rule with the decoding details
		/// @param o        The output stream
		/// @param runtime  The runtime object that knows what happened
		virtual void debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const = 0;
};

/// @brief Concept the includes only the classes derived from @ref check_item
template <class T, class EVENT_TYPE>
concept check_item_concept = std::derived_from<T, check_item<EVENT_TYPE>>;

template<event_concept EVENT_TYPE>
class check_item_sequence;


/// @brief Check item that verifies a single event
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class check_item_event: public check_item<EVENT_TYPE> {
	private:
		/// @brief Event expected for this step
		EVENT_TYPE m_expected_event;

	public:
		/// @brief Constructor
		/// @param expected_event Expected event
		check_item_event(const EVENT_TYPE& expected_event): m_expected_event(expected_event) {}

		/// @brief Starting from event at position `first`, tries to match its contents.
		/// @param first    Position of the first item to be matched
		/// @param runtime  Runtime object containing the events
		/// @return Returns the position of the first event after the matched ones or @ref BAD_POSITION in case of match error
		virtual size_t match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const override;

		/// @brief Print the rule with the decoding details
		/// @param o        The output stream
		/// @param runtime  The runtime object that knows what happened
		virtual void debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const override;
};

/// @brief Check item that a set of parallel actions
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class check_item_parallel: public check_item<EVENT_TYPE> {
	private:
		/// @brief Check items that must be matched in parallel
		std::vector<std::shared_ptr<check_item<EVENT_TYPE>>> m_check_items;

	public:
		/// @brief Add a single event
		/// @param expected_event Expected event
		void expect(const EVENT_TYPE& expected_event) {m_check_items.push_back(std::make_shared<check_item_event<EVENT_TYPE>>(expected_event));}

		/// @brief Add a serial sequence to the parallel item
		std::shared_ptr<check_item_sequence<EVENT_TYPE>> expect_sequence();

		/// @brief Starting from event at position `first`, tries to match its contents.
		/// @param first    Position of the first item to be matched
		/// @param runtime  Runtime object containing the events
		/// @return Returns the position of the first event after the matched ones or @ref BAD_POSITION in case of match error
		virtual size_t match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const override;

		/// @brief Print the rule with the decoding details
		/// @param o        The output stream
		/// @param runtime  The runtime object that knows what happened
		virtual void debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const override;
};

/// @brief Check item that verifies a `lane`, i.e. a sequence of event items that must occur in the exact sequence
/// @tparam EVENT_TYPE Type of the event
template<event_concept EVENT_TYPE>
class check_item_sequence: public check_item<EVENT_TYPE> {
	private:
		/// @brief Check items that must be matched in sequence
		std::vector<std::shared_ptr<check_item<EVENT_TYPE>>> m_check_items;

	public:
		/// @brief Add a single event
		/// @param expected_event Expected event
		void expect(const EVENT_TYPE& expected_event) {m_check_items.push_back(std::make_shared<check_item_event<EVENT_TYPE>>(expected_event));}

		/// @brief Add a parallel entry to the sequence
		std::shared_ptr<check_item_parallel<EVENT_TYPE>> expect_parallel();

		/// @brief Starting from event at position `first`, tries to match its contents.
		/// @param first    Position of the first item to be matched
		/// @param runtime  Runtime object containing the events
		/// @return Returns the position of the first event after the matched ones or @ref BAD_POSITION in case of match error
		virtual size_t match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const override;

		/// @brief Print the rule with the decoding details
		/// @param o        The output stream
		/// @param runtime  The runtime object that knows what happened
		virtual void debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const override;
};


/// @brief Print operator
/// @tparam EVENT_TYPE Type of the event
/// @param o Output stream
/// @param n The data to be printed
/// @return Returns the same output stream so it can be chained
template<event_concept EVENT_TYPE>
inline std::ostream& operator<< (std::ostream& o, const events_list<EVENT_TYPE>& n) {n.print(o); return o;}

/// @brief Print operator
/// @tparam EVENT_TYPE Type of the event
/// @param o Output stream
/// @param n The data to be printed
/// @return Returns the same output stream so it can be chained
template<event_concept EVENT_TYPE>
inline std::ostream& operator<< (std::ostream& o, const check_runtime<EVENT_TYPE>& n) {n.print(o); return o;}


// Print the events in a form readable for debugging
template<event_concept EVENT_TYPE>
inline void events_list<EVENT_TYPE>::print(std::ostream& o) const
{
	if (size() == 0) {
		o << "NO EVENTS";
		return;
	}
	size_t i;
	for (i=0; i<size(); i++) {
		o << std::endl << fmt(i, 10, 3) << ": " << *(operator[](i));
	}
}

// Print the events in a form readable for debugging
template<event_concept EVENT_TYPE>
inline void check_runtime<EVENT_TYPE>::print(std::ostream& o) const
{
	size_t i;

	if (m_event_states.size() == 0) {
		o << "NO EVENTS" << std::endl;
	}
	else {
		for (i=0; i<m_event_states.size(); i++) {
			o << fmt(i, 10, 3) << ": [" << (m_event_states[i].m_consumed ? "*" : " ") << "] " << *m_event_states[i].m_event_ptr << std::endl;
		}
	}
	m_expected.debug_print(o, *this);
}


// Constructor
template<event_concept EVENT_TYPE>
inline check_runtime<EVENT_TYPE>::check_runtime(const check_item<EVENT_TYPE>& expected, const events_list<EVENT_TYPE>& events): 
	m_expected(expected) 
{
	load_events_list(events);
}

// Load an events list into the runtime object
template<event_concept EVENT_TYPE>
inline void check_runtime<EVENT_TYPE>::load_events_list(const events_list<EVENT_TYPE>& events)
{
	m_event_states.clear();
	m_event_states.reserve(events.size());
	for (size_t i=0; i<events.size(); i++) {
		m_event_states.push_back(events[i]);
	}
}

// Mark the indicated event as consumed
template<event_concept EVENT_TYPE>
void check_runtime<EVENT_TYPE>::consume(size_t pos) 
{
	assert(!m_event_states[pos].m_consumed);
	m_event_states[pos].m_consumed=true;
}

// Report the result of a rule
template<event_concept EVENT_TYPE>
void check_runtime<EVENT_TYPE>::report_matching_result(const check_item<EVENT_TYPE>* consuming_rule, check_item_involvement_t check_item_involvement, size_t matched_position)
{
	auto ret = m_item_states.insert({consuming_rule, check_item_state(check_item_involvement, matched_position)});
	assert(ret.second);
}

// Check the indicated `check_item` against the `events`; return `true` if ok, false in case of failure
template<event_concept EVENT_TYPE>
inline bool check_runtime<EVENT_TYPE>::run_check()
{
	size_t end_pos = m_expected.match_item(0, *this);
	if (end_pos == BAD_POSITION) return false;

	// Make sure there are no unused entries
	bool ok = true;
	for (size_t pos=0; pos<events_size(); pos++) {
		if (!event_already_consumed(pos)) {
			ok = false;
			report_unmatched_events(pos);
		}
	}

	return ok;
}

// Starting from a position, find the first available usable event
template<event_concept EVENT_TYPE>
inline size_t check_runtime<EVENT_TYPE>::get_first_unused(size_t starting_pos) const
{
	for(;;) {
		if (starting_pos >= events_size()) return BAD_POSITION;
		if (!event_already_consumed(starting_pos)) return starting_pos;
		starting_pos++;
	}
}

// Starting from event at position `first`, tries to match its contents.
template<event_concept EVENT_TYPE>
inline size_t check_item_event<EVENT_TYPE>::match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const
{
	for (size_t pos = runtime.get_first_unused(first); pos != BAD_POSITION; pos = runtime.get_first_unused(pos+1)) {
		if (runtime.get_event(pos) == m_expected_event) {
			runtime.consume(pos);
			runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCHED, pos);
			return pos+1;
		}
	}
	runtime.report_missing_match(*this, first);
	runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCH_FAILED, BAD_POSITION);
	return BAD_POSITION;
}

// Add a serial sequence to the parallel item
template<event_concept EVENT_TYPE>
inline std::shared_ptr<check_item_sequence<EVENT_TYPE>> check_item_parallel<EVENT_TYPE>::expect_sequence()
{
	std::shared_ptr<check_item_sequence<EVENT_TYPE>> item = std::make_shared<check_item_sequence<EVENT_TYPE>>();
	m_check_items.push_back(item);
	return item;
}

// Starting from event at position `first`, tries to match its contents.
template<event_concept EVENT_TYPE>
inline size_t check_item_parallel<EVENT_TYPE>::match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const
{
	size_t highest_pos = first;

	for (auto item: m_check_items) {
		size_t next_pos = item->match_item(first, runtime);
		if (next_pos == BAD_POSITION) {
			runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCH_FAILED, BAD_POSITION);
			return BAD_POSITION;
		}
		assert(next_pos >= first);
		if (next_pos > highest_pos) highest_pos = next_pos;
	}
	runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCHED, BAD_POSITION);
	return highest_pos;
}

// Add a a parallel item to a serial sequence
template<event_concept EVENT_TYPE>
inline std::shared_ptr<check_item_parallel<EVENT_TYPE>> check_item_sequence<EVENT_TYPE>::expect_parallel()
{
	std::shared_ptr<check_item_parallel<EVENT_TYPE>> item = std::make_shared<check_item_parallel<EVENT_TYPE>>();
	m_check_items.push_back(item);
	return item;
}


// Starting from event at position `first`, tries to match its contents.
template<event_concept EVENT_TYPE>
inline size_t check_item_sequence<EVENT_TYPE>::match_item(size_t first, check_runtime<EVENT_TYPE>& runtime) const
{
	for (auto item: m_check_items) {
		size_t next_pos = item->match_item(first, runtime);
		if (next_pos == BAD_POSITION) {
			runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCH_FAILED, BAD_POSITION);
			return BAD_POSITION;
		}
		first = next_pos;
	}
	runtime.report_matching_result(this, check_runtime<EVENT_TYPE>::check_item_involvement_t::MATCHED, BAD_POSITION);
	return first;
}

// Prints the information it knows about this item
template<event_concept EVENT_TYPE>
inline void check_runtime<EVENT_TYPE>::debug_print_check_item_header(std::ostream& o, const check_item<EVENT_TYPE>* item) const
{
	auto iter = m_item_states.find(item);
	if (iter == m_item_states.end()) {
		o << "[ABANDONED] ";
	}
	else {
		switch(iter->second.m_check_item_involvement) {
			case check_item_involvement_t::MATCHED: o << "[MATCHED] "; break;
			case check_item_involvement_t::MATCH_FAILED: o << "[MATCH_FAILED] "; break;
			default: assert(0);
		}
		if (iter->second.m_matched_position != BAD_POSITION) o << " event=" << fmt(iter->second.m_matched_position, 10, 3) << " ";
	}
}

// Print the rule with the decoding details
template<event_concept EVENT_TYPE>
inline void check_item_event<EVENT_TYPE>::debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const
{
	runtime.debug_print_check_item_header(o, this);
	o << m_expected_event << std::endl;
}


// Print the rule with the decoding details
template<event_concept EVENT_TYPE>
inline void check_item_parallel<EVENT_TYPE>::debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const
{
	runtime.debug_print_check_item_header(o, this);
	o << "---BEGIN PARALLEL---" << std::endl;
	dastd::ostream_indent ind(o);
	for (auto item: m_check_items) {
		item->debug_print(ind, runtime);
	}

	o << "---END PARALLEL---" << std::endl;
}

// Print the rule with the decoding details
template<event_concept EVENT_TYPE>
inline void check_item_sequence<EVENT_TYPE>::debug_print(std::ostream& o, const check_runtime<EVENT_TYPE>& runtime) const
{
	runtime.debug_print_check_item_header(o, this);
	o << "---BEGIN SEQUENCE---" << std::endl;
	dastd::ostream_indent ind(o);
	for (auto item: m_check_items) {
		item->debug_print(ind, runtime);
	}

	o << "---END SEQUENCE---" << std::endl;
}

} // namespace dastd::testing::parallel_checker
