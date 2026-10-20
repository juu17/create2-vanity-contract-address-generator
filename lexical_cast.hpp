#ifndef HPP_LEXICALCAST
#define HPP_LEXICALCAST

#include <sstream>
#include <string>
#include <stdexcept>

namespace lexical_cast {
	template<typename T>
	std::string write(const T & o) {
		std::ostringstream s;
		s << o;
		return s.str();
	}

	// Parse a numeric value, rejecting malformed input instead of silently
	// returning a partial/zero/uninitialized result. `t` is value-initialized
	// (no uninitialized read on empty input), and the whole string must be
	// consumed apart from trailing whitespace, so "16M", "5x", "" and non-numeric
	// input throw rather than yielding 16 / 5 / garbage.
	template <typename T>
	T read(const std::string s) {
		std::istringstream ss(s);
		T t = T();
		ss >> t;
		if (ss.fail()) {
			// extraction itself failed: empty or non-numeric input
			throw std::runtime_error("invalid value for numeric argument: '" + s + "'");
		}
		// `>> char` skips leading whitespace; if any non-whitespace remains the
		// value had trailing garbage (e.g. "16M", "5x"). End-of-stream / trailing
		// whitespace leaves nothing to extract, so a clean value passes.
		char trailing;
		if (ss >> trailing) {
			throw std::runtime_error("invalid value for numeric argument: '" + s + "'");
		}
		return t;
	}

	// Strings are taken verbatim: no whitespace-delimited truncation and no
	// numeric validation (an empty value is a legitimate "not supplied" default).
	template <>
	inline std::string read<std::string>(const std::string s) {
		return s;
	}
}

#endif /* HPP_LEXICALCAST */