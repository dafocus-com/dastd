/**
* @author Davide Achilli
* @copyright Apache 2.0 License
* @date 11-AUG-2023
**/

///   @class dastd::args_parser
///   @brief Manage command line arguments
///
///   Supports POSIX.1-2017-like parameters.
///   See https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
///
///   * Arguments are options if they begin with a hyphen delimiter (`-`).
///   * Multiple options may follow a hyphen delimiter in a single token if the
///     options do not take arguments. Thus, `-abc` is equivalent to `-a -b -c`.
///   * Option names are single alphanumeric characters (as for isalnum).
///   * Certain options require an argument. For example, the `-o` option of the ld
///     command requires an argument and an output file name.
///   * An option and its argument may or may not appear as separate tokens.
///     In other words, the whitespace separating them is optional.
///     Thus, `-o foo` and `-ofoo` are equivalent.
///   * Options typically precede other non-option arguments.
///   * The argument `--` terminates all options; any following arguments are treated as
///     non-option arguments, even if they begin with a hyphen.
///   * A token consisting of a single hyphen character is interpreted as an ordinary
///     non-option argument. By convention, it is used to specify input from or output
///     to the standard input and output streams.
///   * Options may be supplied in any order, or appear multiple times. The
///     interpretation is left up to the particular application program.
///
///   EXAMPLE:
///
///        std::shared_ptr<args_parser> ap = args_parser::make();
///
///        // Simple one letter flag `-a' with it explanation
///        auto p_a = ap->insert<arg_opt_flag>("a", "Explanation of parameter -a");
///
///        // Simple multiple letter flag `--alpha` with it explanation
///        auto p_alpha = ap->insert<arg_opt_flag>("alpha", "Explanation of parameter -alpha");
///
///        // Flag with validation code
///        auto p_b = ap->insert<arg_opt_flag>("b", "Parameter 'b'");
///        p_b->add_validation([](const arg_opt_flag& o) {cout << "Validate flag " << o.key() << endl; return true;});
///
///        // This adds a `--count` additional aka (i.e. `-c' and '--count` are the same)
///        auto p_c = ap->insert<arg_opt_value>("c", "Explanation");
///        p_c->add_aka("count");
///
///        // Parameter with value (--number=<n>) with a standard validator that
///        // makes sure the `--number` parameter is specified at most once.
///        auto p_number = ap->insert<arg_opt_value>("number", "Parameter 'number'");
///        p_number->add_validation(arg::at_most_once);
///
///        // Parse the parameters
///        bool ok = ap->parse(argc, argv);
///
///        // If 'ok', use the values
///        std::cout << "p_number contains " << p_number->count() << " elements" << std::endl;
///
#pragma once
#include "char32string.hpp"
#include "char32parametric.hpp"
#include "container.hpp"
#include "flooder_ch32.hpp"
#include "flooder_ch32_conststr.hpp"
#include "fmt.hpp"
#include "fmt_string.hpp"
#include "fmt_bin.hpp"
#include "strtointegral.hpp"
#include "flooder_ch32_set.hpp"
#include "flooder_ch32_relay.hpp"
#include "sink_ch32_ostream.hpp"
#include "sink_ch32_indent.hpp"
#include "wordwrappable.hpp"
#include "optval.hpp"
#include <iostream>
#include <functional>
#include <string>

namespace dastd {

class arg_opt_aka;
class args_parser;

inline static constexpr string_id_with_default sid_error_invalid_option = {0x21FDB8EB, U"Option '" DASTD_P0 U"' is invalid"};
inline static constexpr string_id_with_default sid_error_missing_value = {0x407385BB, U"Option '" DASTD_P0 U"' must be followed by its value"};
inline static constexpr string_id_with_default sid_error_unexpected_value = {0xB19CA57F, U"Option '" DASTD_P0 U"' can not be followed by a value"};

/// @brief Base class for a named option
class arg_opt: public std::enable_shared_from_this<arg_opt> {
	friend class args_parser;
	public:
		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt(std::string_view name): m_name(name) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Description of the argument, shown in help
		arg_opt(std::string_view name, std::shared_ptr<flooder_ch32> desc): m_name(name), m_desc(desc) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt(std::string_view name, const char32_t* desc): m_name(name) {set_desc(desc);}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt(std::string_view name, std::string_view desc): m_name(name) {set_desc(desc);}

		/// @brief Destructor
		virtual ~arg_opt() {}

		/// @brief Reset to empty
		virtual void reset() = 0;

		/// @brief Returns true if requires a parameter
		virtual bool is_parametric() const = 0;

		/// @brief Returns true if it is an aka
		virtual bool is_aka() const {return false;}

		/// @brief Returns true if it is short form (-c); false if long form (--long-param)
		virtual bool is_short_form() const {return m_name.length() == 1;}

		/// @brief Called by the arg_parser when it is matched.
		///
		/// @param value Parameter string; empty if the parameter declared is_parametric()==false.
		virtual void matched(std::string_view value) = 0;

		/// @brief Called by the arg_parser when done to validate the contents
		///
		/// @return Return true if ok, false in case of error.
		virtual bool validate() const {return true;}

		/// @brief Print the description of the parameter
		///
		/// Can be overridden if some custom printing is required.
		virtual void print_desc(sink_ch32& sink) const {m_desc->write_to_sink(sink);}

		/// @brief Print the option name with its required dashes and parameter if required
		virtual void print_name(sink_ch32& out) const;

		/// @brief Return the option name
		std::string name() const {std::ostringstream ret; sink_ch32_ostream sink(ret); print_name(sink); return ret.str();}

		/// @brief Print the parameters after the option
		///
		/// @param out The output sink
		/// @param short_form True if the parameter is short (like -c) or false if long (like --long-param)
		virtual void print_param(sink_ch32& out, bool short_form) const {DASTD_NOWARN_UNUSED(out); DASTD_NOWARN_UNUSED(short_form);}

		/// @brief Print the option debug data
		virtual void print_debug_data(sink_ch32& out) const = 0;

		/// @brief Compare with another arg for sorting
		bool operator<(const arg_opt& o) const {return key()<o.key();}

		/// @brief Return the key (i.e. the parameter name)
		const std::string key() const {return m_name;}

		/// @brief Set the description with a generic flooder_ch32
		void set_desc(std::shared_ptr<flooder_ch32> desc) {m_desc = desc;}

		/// @brief Set the description with a static string
		void set_desc(const char32_t* desc) {m_desc = std::make_unique<char32string>(desc);}

		/// @brief Set the description with a static string
		void set_desc(std::string_view desc) {m_desc = std::make_unique<char32string>(desc);}

		/// @brief Set the description with a static string
		void set_desc(const char* desc) {m_desc = std::make_unique<char32string>(desc);}

		/// @brief Return the associated parser
		std::shared_ptr<args_parser> parser() const {return m_parser.lock();}

		/// @brief Add an 'aka', i.e. an alternative form
		///
		/// @return Returns 'this'
		arg_opt& add_aka(std::string_view name);

		/// @brief Print the help containing it in the given number of columns
		///
		/// This method prints all the options with their explanation using wordwrapping.
		///
		/// @param sink Where the text will be printed
		/// @param columns Maximum number of columns used
		/// @param indent Indent of the lines following the first one (that contains the parameters)
		virtual void print_options_help(sink_ch32& sink, uint32_t columns, uint32_t indent=10) const;

		/// @brief Print information about one non-optional parameter
		///
		/// This methiod can be used to print the information about one non-optional
		/// parameter with the same layout of the `print_options_help` method.
		///
		/// @param sink Where the text will be printed
		/// @param columns Maximum number of columns used
		/// @param indent Indent of the lines following the first one (that contains the parameters)
		static void print_nonoption_help(const flooder_ch32& param_source, const flooder_ch32& desc_source, sink_ch32& sink, uint32_t columns, uint32_t indent=10);

	protected:
		/// @brief Single or multiple letter argument name (without - or --)
		std::string m_name;

		/// @brief List of 'aka', i.e. alternative names
		std::vector<std::weak_ptr<const arg_opt_aka>> akas;

		/// @brief Description to be shown in help
		std::shared_ptr<flooder_ch32> m_desc;

		/// @brief Associated parser
		std::weak_ptr<args_parser> m_parser;
};


/// @brief Flag parser
///
/// This object matches a simple flag with no parameters
class arg_opt_flag: public arg_opt {
	public:
		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt_flag(std::string_view name): arg_opt(name) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Description of the argument, shown in help
		arg_opt_flag(std::string_view name, std::shared_ptr<flooder_ch32> desc): arg_opt(name, desc) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_flag(std::string_view name, const char32_t* desc): arg_opt(name, desc) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_flag(std::string_view name, std::string_view desc): arg_opt(name, desc) {}

		/// @brief Add an 'aka', i.e. an alternative form
		///
		/// @return Returns 'this'
		arg_opt_flag& add_aka(std::string_view name) {arg_opt::add_aka(name); return *this;}

		/// @brief Reset to defaults
		virtual void reset() {m_count = 0;}

		/// @brief Returns true if requires a parameter
		virtual bool is_parametric() const {return false;}

		/// @brief Returns the number of instances of this flag
		uint32_t count() const {return m_count;}

		/// @brief True if count()>0
		bool is_set() const {return m_count>0;}

		/// @brief Called by the arg_parser when it is matched.
		///
		/// @param param Parameter string; empty if the parameter declared is_parametric()==false.
		virtual void matched(std::string_view) {m_count++;}

		/// @brief Print the option debug data
		virtual void print_debug_data(sink_ch32& out) const;

		/// @brief Prototype for the validation function
		typedef std::function<bool (const arg_opt_flag& flag)> validation_function;

		/// @brief Add a validation function
		arg_opt_flag& add_validation(validation_function f) {m_validation_funcs.push_back(f); return *this;}

		/// @brief Called by the arg_parser when done to validate the contents
		///
		/// @return Return true if ok, false in case of error.
		virtual bool validate() const {bool v=true; for (auto f: m_validation_funcs) if (!f(*this)) v=false; return v;}

		/// @brief Set an optval<bool> to true if the parameter is present
		void set(optval<bool>& opt) const {if (is_set()) opt = true;}

	protected:
		/// @brief Counts the number of matches
		uint32_t m_count = 0;

		/// @brief Vector of validation functions. Called in order by the `validate` method.
		std::vector<validation_function> m_validation_funcs;
};

/// @brief Value parser
///
/// This object matches an option with value.
/// For example: -p10, -p 10 or --foobar=10
class arg_opt_value: public arg_opt {
	public:
		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt_value(std::string_view name): arg_opt(name) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Description of the argument, shown in help
		arg_opt_value(std::string_view name, std::shared_ptr<flooder_ch32> desc): arg_opt(name, desc) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value(std::string_view name, const char32_t* desc): arg_opt(name, desc) {}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value(std::string_view name, std::string_view desc): arg_opt(name, desc) {}

		/// @brief Add an 'aka', i.e. an alternative form
		///
		/// @return Returns 'this'
		arg_opt_value& add_aka(std::string_view name) {arg_opt::add_aka(name); return *this;}

		/// @brief Reset to defaults
		virtual void reset() {m_values.clear();}

		/// @brief Returns true if requires a parameter
		virtual bool is_parametric() const {return true;}

		/// @brief Called by the arg_parser when it is matched.
		///
		/// @param value Parameter string; empty if the parameter declared is_parametric()==false.
		virtual void matched(std::string_view value) {m_values.emplace_back(value);}

		/// @brief Print the parameters after the option
		///
		/// @param out The output sink
		/// @param short_form False if the parameter is short (like -c) or true if long (like --long-param)
		virtual void print_param(sink_ch32& out, bool short_form) const;

		/// @brief Print the option debug data
		virtual void print_debug_data(sink_ch32& out) const;

		/// @brief Print a value placemark (for example, "<v>") to be shown in help
		virtual void print_value_placemark(sink_ch32& out) const;

		/// @brief Prototype for the validation function
		typedef std::function<bool (const arg_opt_value& opt)> validation_function;

		/// @brief Add a validation function
		arg_opt_value& add_validation(validation_function f) {m_validation_funcs.push_back(f); return *this;}

		/// @brief Called by the arg_parser when done to validate the contents
		///
		/// @return Return true if ok, false in case of error.
		virtual bool validate() const {bool v=true; for (auto f: m_validation_funcs) if (!f(*this)) v=false; return v;}

		/// @brief Access the values
		const std::vector<std::string>& get_values() const {return m_values;}

		/// @brief Access the values as a simple string
		///
		/// @param pos Position of the value in the series
		const std::string& str(uint32_t pos=0) const {return m_values[pos];}

		/// @brief Returns the number of instances of this flag
		uint32_t count() const {return (uint32_t)m_values.size();}

		/// @brief Set the value placemark instead of the default "<v>" string
		void set_value_placemark(std::string_view value_placemark) {m_value_placemark = value_placemark;}

		/// @brief Set an optval<NUMTYPE> to true if the parameter is present
		void set(std::string& tgt) const {if (count()>0) tgt = str(0);}

	protected:
		/// @brief List of values
		///
		/// This can contain multiple entries if the same option has been
		/// specified multiple times.
		std::vector<std::string> m_values;

		/// @brief Vector of validation functions. Called in order by the `validate` method.
		std::vector<validation_function> m_validation_funcs;

		/// @brief Value placemark string
		std::string m_value_placemark = "<v>";
};

/// @brief Value parser
///
/// This object matches an option with an integral value.
/// For example: -p10, -p 10 or --foobar=10
template<class NUMTYPE>
class arg_opt_value_integral: public arg_opt_value {
	public:
		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt_value_integral(std::string_view name): arg_opt_value(name) {init_value_integral();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Description of the argument, shown in help
		arg_opt_value_integral(std::string_view name, std::shared_ptr<flooder_ch32> desc): arg_opt_value(name, desc) {init_value_integral();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value_integral(std::string_view name, const char32_t* desc): arg_opt_value(name, desc) {init_value_integral();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value_integral(std::string_view name, std::string_view desc): arg_opt_value(name, desc) {init_value_integral();}

		/// @brief Add a validation function
		void add_validation(validation_function f) {arg_opt_value::add_validation(f);}

		/// @brief Get the value converted to the template integral type
		///
		/// @param pos Instance
		/// @param default_val Value returned if the parameter format is invalid
		NUMTYPE number(size_t pos=0, NUMTYPE default_val=0) const;

		/// @brief Set an optval<NUMTYPE> to true if the parameter is present
		void set(optval<NUMTYPE>& opt) const {if (count()>0) opt = number(0,0);}

	protected:
		/// @brief Common initializer
		void init_value_integral();
};

/// @brief Value parser
///
/// This object matches an option with an integral value.
/// For example: -p10.4, -p 10 or --foobar=-10.7
class arg_opt_value_double: public arg_opt_value {
	public:
		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt_value_double(std::string_view name): arg_opt_value(name) {init_value_double();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Description of the argument, shown in help
		arg_opt_value_double(std::string_view name, std::shared_ptr<flooder_ch32> desc): arg_opt_value(name, desc) {init_value_double();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value_double(std::string_view name, const char32_t* desc): arg_opt_value(name, desc) {init_value_double();}

		/// @brief Constructor
		///
		/// @param name Single or multiple letter argument name (without - or --)
		/// @param desc Simple string description of the argument, shown in help
		arg_opt_value_double(std::string_view name, std::string_view desc): arg_opt_value(name, desc) {init_value_double();}

		/// @brief Add a validation function
		arg_opt_value_double& add_validation(validation_function f) {arg_opt_value::add_validation(f); return *this;}

		/// @brief Get the value converted to the template integral type
		///
		/// @param pos Instance
		/// @param default_val Value returned if the parameter format is invalid
		double number(size_t pos=0, double default_val=0) const;

	protected:
		/// @brief Common initializer
		void init_value_double();
};

/// @brief Alternative flag
///
/// This object simply relies the match to another option. Used
/// when there is a short and a long version of a parameter, like
/// "-p" and "--long-version"
class arg_opt_aka: public arg_opt {
	friend class arg_opt;
	public:
		/// @brief Constructor
		///
		/// Can be instanced only by the 'add_aka` method.
		///
		/// @param name Single or multiple letter argument name (without - or --)
		arg_opt_aka(std::string_view name, std::weak_ptr<arg_opt> master_opt): arg_opt(name), m_master_opt(master_opt) {}

		/// @brief Returns true if requires a parameter
		virtual bool is_parametric() const {return m_master_opt.lock()->is_parametric();}

		/// @brief Returns true if it is an aka
		virtual bool is_aka() const {return true;}

		/// @brief Called by the arg_parser when it is matched.
		///
		/// @param param Parameter string; empty if the parameter declared is_parametric()==false.
		virtual void matched(std::string_view val) {m_master_opt.lock()->matched(val);}

		/// @brief Print the option debug data
		virtual void print_debug_data(sink_ch32& out) const {m_master_opt.lock()->print_debug_data(out);}

		/// @brief Reset to empty
		virtual void reset() {}

	protected:
		/// @brief Option of which this is an aka
		std::weak_ptr<arg_opt> m_master_opt;
};

// See file header
class args_parser: public std::enable_shared_from_this<args_parser> {
	protected:
		/// @brief Protected constructor; use the public `make` factory methods that create a dynamic instance in a shared pointer; use args_parser::make() to create an instance
		args_parser() {}

	public:
		/// @brief Static instance creator
		static std::shared_ptr<args_parser> make() {args_parser* p=new args_parser; return std::shared_ptr<args_parser>(std::move(p));}

		/// @brief Virtual destructor
		virtual ~args_parser() {}

		/// @brief Add an option allocated with new; asserts in case of duplications.
		template<class OPT_CLS, typename... Targs>
		std::shared_ptr<OPT_CLS> insert(Targs... Fargs);

		/// @brief Reset to empty values
		///
		/// This methods prepares the parser for a new parameters parsing.
		/// It will not delete the argument definitions, just the values that
		/// have been loaded on the previous run.
		void reset();

		/// @brief Execute a full parsing of a argc/argv pair
		///
		/// @param argc Number of arguments
		/// @param argv Argument
		/// @param first_is_executable true if the first argument is the executable name (thus, it has to be skipped)
		///
		/// @return Returns true if ok, false in case of error
		bool parse(int argc, const char* argv[], bool first_is_executable=true);
		bool parse(int argc, char* argv[], bool first_is_executable=true) {return parse(argc, (const char**)argv, first_is_executable);}

		/// @brief Print the option debug data
		void print_debug_data(sink_ch32& out) const;

		/// @brief Called when an option is unknown to report an error
		virtual void error_invalid_option(std::string_view opt);

		/// @brief Called when a parametric option is missing the value
		virtual void error_missing_value(const arg_opt& arg);

		/// @brief Called when a non-parametric option is given a value
		virtual void error_unexpected_value(const arg_opt& arg);

		/// @brief Print the help containing it in the given number of columns
		///
		/// @param sink Where the text will be printed
		/// @param columns Maximum number of columns used
		/// @param indent Indent of the lines following the first one (that contains the parameters)
		void print_options_help(sink_ch32& sink, uint32_t columns, uint32_t indent=10) const {internal_print_options_help(sink, columns, indent);}
		void print_options_help(sink_ch32&& sink, uint32_t columns, uint32_t indent=10) const {internal_print_options_help((sink_ch32&)sink, columns, indent);}

		/// @brief Print the help containing it in the given number of columns
		///
		/// This method prints all the options with their explanation using wordwrapping.
		///
		/// @param out Where the text will be printed using the indicated format
		/// @param columns Maximum number of columns used
		/// @param indent Indent of the lines following the first one (that contains the parameters)
		template<fmt_string_format_t FORMAT=fmt_string_f::UTF8>
		void print_options_help(std::ostream& out, uint32_t columns=79, uint32_t indent=10) const {sink_ch32_ostream<FORMAT> sink(out); print_options_help(sink, columns, indent);}

		/// @brief Resolve a string_id into a flooder_ch32 for error reporting
		///
		/// This method is used by the various arguments to report error messages.
		/// Each error message has its own string_id. This method tries to relay
		/// the request to the given `m_messages_set` object. If it is not
		/// available or unable to resolve the message string_id, it will
		/// return the default value.
		virtual flooder_ch32_relay get_message(const string_id_with_default& msgid) const;

		/// @brief Print an error message
		///
		/// @param msg Short message not terminated by newline.
		virtual void print_error_message(const flooder_ch32& msg) {std::cerr << fmt32_utf8(msg) << std::endl;}

		/// @brief Set the messages `flooder_ch32_set` to be used to retrieve the error messages
		void set_messages_set(const flooder_ch32_set* messages_set) {m_messages_set = messages_set;}

		/// @brief Array of the non-options parameters
		const std::vector<std::string>& get_non_options() const {return m_non_options;}

	protected:
		/// @brief Print the help containing it in the given number of columns
		///
		/// @param sink Where the text will be printed
		/// @param columns Maximum number of columns used
		/// @param indent Indent of the lines following the first one (that contains the parameters)
		virtual void internal_print_options_help(sink_ch32& sink, uint32_t columns, uint32_t indent=10) const;

		/// @brief Execute the parsing of the arguments in the m_args_vals array
		///
		/// @return Returns true if ok, false in case of error
		bool execute();

		/// @brief Registered arguments
		set_key_shared_ptr<const std::string, arg_opt> m_options;

		/// @brief Array of elements from the argument list
		std::vector<std::string> m_args_vals;

		/// @brief Array of non-option string (i.e. those not beginning with -)
		std::vector<std::string> m_non_options;

		/// @brief True if the -- delimiter has been matched
		///
		/// All the following parameters are considered "non-option" even if
		/// they begin with -
		bool m_double_dash = false;

		/// @brief `flooder_ch32_set` used to resolve the error strings
		///
		/// This pointer can be nullptr if no `flooder_ch32_set` is available
		const flooder_ch32_set* m_messages_set = nullptr;
};

// Add an argument allocated with new; asserts in case of duplications.
template<class OPT_CLS, typename... Targs>
std::shared_ptr<OPT_CLS> args_parser::insert(Targs... Fargs)
{
	std::shared_ptr<OPT_CLS> new_arg = std::make_unique<OPT_CLS>(Fargs...);
	new_arg->m_parser = weak_from_this();
	DASTD_DEBUG(auto insert_result =) m_options.insert(new_arg);
	assert(insert_result.second); // If fails, the option is dupe
	return new_arg;
}

inline static constexpr string_id_with_default sid_arg_is_integral = {0x9D5ED42D, U"Option '" DASTD_P0 U"' value '" DASTD_P1 U"' is not a valid integral number"};
inline static constexpr string_id_with_default sid_arg_is_double = {0x49376FD2, U"Option '" DASTD_P0 U"' value '" DASTD_P1 U"' is not a valid floating point number"};
inline static constexpr string_id_with_default sid_at_most_once = {0xCF55BFEF, U"Option '" DASTD_P0 U"' can be speficied at most once"};
inline static constexpr string_id_with_default sid_exactly_once = {0x7468ADCA, U"Option '" DASTD_P0 U"' must be specified once"};
/// @brief Trait class that contains some helper functions
class arg {
	public:
		/// @brief Validation function for `arg_opt_value` that limits the number of instances to zero or one
		static bool at_most_once(const arg_opt_value& o);

		/// @brief Validation function for `arg_opt_value` that limits the number of instances to exactly one
		static bool exactly_once(const arg_opt_value& o);

		/// @brief Validation function for `arg_opt_value` that checks it can be converted into an integral type NUMTYPE
		template<class NUMTYPE>
		static bool is_integral(const arg_opt_value& a) {
			static_assert(std::is_integral<NUMTYPE>::value, "Integral required");
			bool ok=true;
			for(auto & str: a.get_values()) {
				auto pair = strtointegral<NUMTYPE,char>(str);
				if (!pair.second) {
					char32parametric c32p(a.parser()->get_message(sid_arg_is_integral));
					c32p.set_param(0) << a.name();
					c32p.set_param(1) << fmt_string(str);
					a.parser()->print_error_message(c32p);

					ok=false;
				}
			}
			return ok;
		}

		/// @brief Validation function for `arg_opt_value` that checks it can be converted into a 'double'
		static bool is_double(const arg_opt_value& a) {
			bool ok=true;
			for(auto & str: a.get_values()) {
				try {
					(void)std::stod(str);
				}
				catch(const std::exception&) {
					char32parametric c32p(a.parser()->get_message(sid_arg_is_integral));
					c32p.set_param(0) << a.name();
					c32p.set_param(1) << fmt_string(str);
					a.parser()->print_error_message(c32p);
					ok=false;
				}
			}
			return ok;
		}
};


// Common initializer
template<class NUMTYPE>
void arg_opt_value_integral<NUMTYPE>::init_value_integral() {
	static_assert(std::is_integral<NUMTYPE>::value, "Integral required");
	add_validation(arg::is_integral<NUMTYPE>);
}

// Get the value converted to the template integral type
template<class NUMTYPE>
NUMTYPE arg_opt_value_integral<NUMTYPE>::number(size_t pos, NUMTYPE default_val) const {
	if (pos >= count()) return default_val;
	auto pair = strtointegral<NUMTYPE,char>(get_values()[pos], default_val);
	return pair.first;
}

// Common initializer
inline void arg_opt_value_double::init_value_double() {
	add_validation(arg::is_double);
}

// Get the value converted to the template integral type
inline double arg_opt_value_double::number(size_t pos, double default_val) const {
	if (get_values().size() <= pos) return default_val;
	std::string value_string = get_values()[pos];
	if (value_string.length() == 0) return default_val;
	try {
		return std::stod(get_values()[pos]);
	}
	catch(...) {
		return default_val;
	}
}

// Print information about one non-optional parameter
// This methiod can be used to print the information about one non-optional
// parameter with the same layout of the `print_options_help` method.
inline void arg_opt::print_nonoption_help(const flooder_ch32& param_source, const flooder_ch32& desc_source, sink_ch32& sink, uint32_t columns, uint32_t indent)
{
	uint32_t i;
	assert(columns > indent+10);

	char32string param_string;
	param_source.write_to_sink(param_string);

	// String containing the text to be wordwrapped
	char32string text;

	// Case A: the parameter string is short enough to
	// fit in the first column.
	if (param_string.length() < (size_t)indent) {
		// Write the string
		sink << param_string;

		// Write some padding
		for (i=(uint32_t)param_string.length(); i<indent; i++) {
			sink << " ";
		}
	}
	// Case B or C: the parameter string is too long. Write the
	// beginning of the parameter string to the sink and append the rest
	// to the word wrapper
	else {
		// Write the initial part
		for (i=0; i<indent; i++) {
			sink << param_string[i];
		}

		// Append the rest to the padder
		text.append(param_string, indent);
		text << "\n";
	}

	// Append the description
	text << desc_source;

	// Prepare the indented sink
	sink_ch32_indent ind(sink, indent, false, U" ");

	// Wordwrap
	ind << wordwrap_string<char32_t>(text, columns-indent) << "\n";
}

// Print the help containing it in the given number of columns
inline void arg_opt::print_options_help(sink_ch32& sink, uint32_t columns, uint32_t indent) const
{
	char32string param_source;
	char32string desc_source;

	// Print the main parameter
	print_name(param_source);
	print_param(param_source, is_short_form());

	// Append the akas, if any
	for (auto & weak_aka: akas) {
		param_source << ", ";
		auto aka = weak_aka.lock();
		if (aka) {
			aka->print_name(param_source);
			print_param(param_source, aka->is_short_form());
		}
	}
	// String containing the text to be wordwrapped
	print_desc(desc_source);

	print_nonoption_help(param_source, desc_source, sink, columns, indent);
}

// Add an 'aka', i.e. an alternative form
inline arg_opt& arg_opt::add_aka(std::string_view name)
{
	auto pars = parser();
	assert(pars);

	auto opt = pars->insert<arg_opt_aka>(name, weak_from_this());
	akas.push_back(opt);
	return *this;
}

// Print the option debug data
inline void arg_opt::print_name(sink_ch32& out) const
{
	out << (is_short_form() ? "-" : "--") << m_name;
}

// Print the option debug data
inline void arg_opt_flag::print_debug_data(sink_ch32& out) const
{
	arg_opt::print_name(out);
	out << ", " << m_count << " instance(s)";
}

// Print the option debug data
inline void arg_opt_value::print_param(sink_ch32& out, bool short_form) const
{
	if (!short_form) {
		out << "=";
	}
	print_value_placemark(out);
}

// Print a value placemark (for example, "<v>") to be shown in help
inline void arg_opt_value::print_value_placemark(sink_ch32& out) const
{
	out << m_value_placemark;
}

// Print the option debug data
inline void arg_opt_value::print_debug_data(sink_ch32& out) const
{
	arg_opt::print_name(out);
	out << ", values:";
	for(auto & val: m_values) {
		out << " " << fmt_string<char,fmt_string_f::C11_ESCAPED_AUTOREQUOTE_QUOTED>(val);
	}
}

// Print the option debug data
inline void args_parser::print_debug_data(sink_ch32& out) const
{
	for(auto & arg: m_options) {
		if (!arg->is_aka()) {
			out << "OPTION: ";
			arg->print_debug_data(out);
			out << "\n";
		}
	}
	for(auto & non_option: m_non_options) {
		out << "NONOPT: " << fmt_string<char,fmt_string_f::C11_ESCAPED_AUTOREQUOTE_QUOTED>(non_option) << "\n";
	}
}


// Reset to empty values
inline void args_parser::reset()
{
	for (auto & arg: m_options) arg->reset();
	m_args_vals.clear();
	m_non_options.clear();
	m_double_dash = false;
}

// Execute a full parsing of a argc/argv pair
inline bool args_parser::parse(int argc, const char* argv[], bool first_is_executable)
{
	reset();
	int i;
	for (i=(first_is_executable ? 1 : 0); i<argc; i++) {
		m_args_vals.push_back(argv[i]);
	}

	return execute();
}

// Execute the parsing of the arguments in the m_args_vals array
inline bool args_parser::execute()
{
	unsigned i;
	bool valid = true;
	std::string name;

	for (i=0; i<m_args_vals.size(); i++) {
		std::string_view arg = m_args_vals[i];

		if (m_double_dash) {
			m_non_options.emplace_back(arg);
			continue;
		}

		if (arg == "--") {
			m_double_dash = true;
			continue;
		}

		// Current option
		//std::shared_ptr<arg_opt> opt;

		// Special case: '-' followed by multiple letters
		// They might be all flags except for the last one
		if ((arg.length() >= 2) && (arg[0]=='-') && (arg[1]!='-')) {
			size_t j;
			for (j=1; j<arg.length(); j++) {
				name = arg.substr(j, 1);
				std::shared_ptr<arg_opt> opt = m_options.find_or_null(name);
				if (opt) {
					// If there is something remaining, then it is the parameter value
					// The cases are:
					//   -p0     p=0
					//   -p 0    p=0 with two separate arguments
					//   -xyzp0  p=0, where x, y and z are simple flags
					//   -xyzp 0 p=0, where x, y and z are simple flags and p is set with two separate arguments
					if (opt->is_parametric()) {
						// This is the case -p<val>
						if (j+1 < arg.length()) {
							opt->matched(arg.substr(j+1));
							j = arg.length();
						}
						// Otherwise there must be a following string
						else if (i+1 < m_args_vals.size()) {
							i++;
							opt->matched(m_args_vals[i]);
						}
						else {
							error_missing_value(*opt);
						}
					}
					else {
						opt->matched("");
					}
				}
				else {
					error_invalid_option("-"+name);
					valid = false;
				}
			}
			continue;
		}

		// Check for -- argument prefix
		// It can be:
		//   --name
		//   --name=value
		if (arg.compare(0, 2, "--", 2) == 0) {
			// See if it has a '=' in it
			size_t eq_pos = arg.find_first_of('=');
			bool has_value;
			if (eq_pos == std::string::npos) {
				name = arg.substr(2);
				has_value = false;
			}
			else {
				name = arg.substr(2, eq_pos-2);
				has_value = true;
			}

			std::shared_ptr<arg_opt> opt = m_options.find_or_null(name);
			if (opt) {
				if (has_value) {
					if (!opt->is_parametric()) {
						error_unexpected_value(*opt);
						valid = false;
					}
					else {
						opt->matched(arg.substr(eq_pos+1));
					}
				}
				else {
					if (opt->is_parametric()) {
						// See if the user specified "--param value" instead of "--param=value"
						if (i+1 < m_args_vals.size()) {
							i++;
							opt->matched(m_args_vals[i]);
						}
						else {
							error_missing_value(*opt);
							valid = false;
						}
					}
					else {
						opt->matched("");
					}
				}
			}
			else {
				error_invalid_option("--"+name);
				valid = false;
			}
		}
		m_non_options.emplace_back(arg);
	}

	// Execute the validation
	for (auto & opt: m_options) {
		if (!opt->validate()) valid=false;
	}

	return valid;
}


/// Print the help containing it in the given number of columns
inline void args_parser::internal_print_options_help(sink_ch32& sink, uint32_t columns, uint32_t indent) const
{
	for (auto & opt: m_options) {
		if (opt->is_aka()) continue;
		opt->print_options_help(sink, columns, indent);
	}
}
#if 0
/// the help containing it in the given number of columns
inline void args_parser::print_options_help(std::ostream& out, uint32_t columns, uint32_t indent, fmt_string_format_t format) const
{
	sink_ch32_ostream sink(out, format);
	print_options_help(sink, columns, indent);
}
#endif

// Called when an option is unknown to report an error
inline void args_parser::error_invalid_option(std::string_view opt)
{
	char32parametric c32p(get_message(sid_error_invalid_option));
	c32p.set_param(0) << opt;
	print_error_message(c32p);
}

// Called when a parametric option is missing the value
inline void args_parser::error_missing_value(const arg_opt& arg)
{
	char32parametric c32p(get_message(sid_error_missing_value));
	c32p.set_param(0) << arg.name();
	print_error_message(c32p);
}

// Called when a non-parametric option is given a value
inline void args_parser::error_unexpected_value(const arg_opt& arg)
{
	char32parametric c32p(get_message(sid_error_unexpected_value));
	c32p.set_param(0) << arg.name();
	print_error_message(c32p);
}

// Resolve a string_id into a flooder_ch32 for error reporting
inline flooder_ch32_relay args_parser::get_message(const string_id_with_default& msgid) const
{
	if (m_messages_set) return m_messages_set->get(msgid);
	return flooder_ch32_relay(std::make_unique<flooder_ch32_conststr<char32_t>>(msgid.default_text));
}

// Validation function for `arg_opt_value` that limits the number of instances to zero or one
inline bool arg::at_most_once(const arg_opt_value& a) {
	if (a.count() > 1) {
		char32parametric c32p(a.parser()->get_message(sid_at_most_once));
		c32p.set_param(0) << a.name();
		a.parser()->print_error_message(c32p);
		return false;
	}
	return true;
}

// Validation function for `arg_opt_value` that limits the number of instances to zero or one
inline bool arg::exactly_once(const arg_opt_value& a) {
	if (a.count() != 1) {
		char32parametric c32p(a.parser()->get_message(sid_exactly_once));
		c32p.set_param(0) << a.name();
		a.parser()->print_error_message(c32p);
		return false;
	}
	return true;
}

} // namespace dastd
