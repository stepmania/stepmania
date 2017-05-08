#ifndef RAGE_UNICODE_HPP_
#define RAGE_UNICODE_HPP_

#include <string>

namespace Rage
{
	/** @brief The unicode replacement character, FFFD. */
	extern wchar_t const invalid_char;

	/** @brief Sanitize the UTF-8 string. */
	void utf8_sanitize( std::string &s );

	/** @brief Given a UTF-8 byte, return the length of the codepoint. */
	int utf8_get_char_len( char p );

	/** @brief Determine if the byte is an official continuation byte. */
	bool is_utf8_continuation_byte( char p );

	/** @brief Decode a code point and place within the output variable.
	 *
	 * @return true if a valid encoded string was found. */
	bool utf8_to_wchar_ec( std::string const &s, unsigned &start, wchar_t &ch );

	/** @brief Decode a code point and place within the output variable.
	 *
	 * This variant only does enough error checking to prevent crashing.
	 *
	 * @return true if a valid encoded string was found. */
	bool utf8_to_wchar( char const *s, size_t iLength, unsigned &start, wchar_t &ch );

	/** @brief Analyze the wide character and turn into a UTF-8 string. */
	void wchar_to_utf8( wchar_t ch, std::string &out );

	/** @brief Attempt to get a wide character from the UTF-8 string.
	 *
	 * @return the appropriate wide character if one exists, or an appropiate default character if one does not. */
	wchar_t utf8_get_char( std::string const &s );

	/** @brief Determine if the string passed in is a valid UTF-8 string.
	 *
	 * @return true if it's a valid UTF-8 string. */
	bool utf8_is_valid( const std::string &s );

	/** @brief Remove the starting Byte Order Mark (BOM) form the string if it exists. */
	void utf8_remove_bom( std::string &s );

	/** @brief Convert a wide string to its lowercase variant. */
	std::wstring make_lower( std::wstring const &s );

	/** @brief Convert a wide string to its lowercase variant. */
	wchar_t make_lower( wchar_t const &s );

	/** @brief Convert a wide string to its uppercase variant. */
	std::wstring make_upper( std::wstring const &s );

	/** @brief Convert a wide string to its uppercase variant. */
	wchar_t make_upper( wchar_t const &s );

	/** @brief A mapping to allow for quick lowercase conversion. */
	extern unsigned char lowerCase[256];

	/** @brief A mapping to allow for quick uppercase conversion. */
	extern unsigned char upperCase[256];
}

#endif
