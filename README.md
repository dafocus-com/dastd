# DASTD

This library contains include-only extensions to the C++20 standard library.

## History

* 1.0.00 - 17-SEP-2025 - First release
* 1.0.01 - 17-SEP-2025 - Added one missing file
* 1.0.02 - 30-SEP-2025 - Added several files that were not yet released in the previous releases. Changed several `const std::string&` in `const std::string_view&`
* 1.0.03 - 18-DEC-2025 - Added support for UUID and multiple formats used to print structured data
* 1.0.04 - 18-DEC-2025 - Fixed incompatiblity with g++ 15.2.1 20251112 in UUID code
* 1.0.05 - 14-AUG-2026 - Several changes and files added
* 1.0.06 - 28-OCT-2026 
	- Now args supports "--param value" and not only "--param=value" as before
	- removed useless `src/dastd/meson.build`
	- `dastd::exception` now supports a `string_view` as an optional constructor
	- `dastd::json_data_decoder` has new method `tokenizer()` to allow some manual processing
	- macro `DASTD_THROW` now requires a `;` as a terminator
	- added new class `dastd::sink_byte_file` that writes on a `std::ostream`
	- added new class `dastd::source_file` that implements a `source_istream` connected to a `std::ifstream`
	- fixed a bug on `dastd::source_istream::tentative_read()` when the stream returns `eof`
	- class `dastd::marshal_dec_json` constructor now expects a less restrictive `std::string_view` instead of the previous `const std::string&`
	- minor cleanup fixes (like some missing `override` statements)

