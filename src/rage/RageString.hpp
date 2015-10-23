#ifndef RAGE_STRING_HPP_
#define RAGE_STRING_HPP_

#include <string>
#include <vector>

namespace Rage
{
    /** @brief Get the first x characters of a string. Allow negative warping.
     *
     * This comes from http://stackoverflow.com/a/7597469/445373
     */
    std::string head(std::string const &source, int32_t const length);
    
    /** @brief Get the last x characters of a string. Allow negative warping.
     *
     * This comes from http://stackoverflow.com/a/7597469/445373
     */
    std::string tail(std::string const &source, int32_t const length);

	/** @brief Determine if the source string begins with the specified content. */
	bool starts_with(std::string const &source, std::string const &target);

	/** @brief Determine if the source string ends with the specified content. */
	bool ends_with(std::string const &source, std::string const &target);

	/** @brief Offer a clean way of hexify-ing a wide character. Primarily used for fonts.
	 *
	 * See http://stackoverflow.com/a/8024386 */
	std::string hexify(wchar_t const src, unsigned int dstlen);

	/** @brief Offer a clean way of replacing all occurances of a character with a different character.
	 *
	 * See http://stackoverflow.com/a/2896627 */
	void replace(std::string &target, char from, char to);

	/** @brief Offer a clean way of replacing all occurances of a string with a different string.
	 *
	 * See http://stackoverflow.com/a/29752943 */
	void replace(std::string &target, std::string const &from, std::string const &to);

	/** @brief Convert the string into its uppercase variant. */
	std::string make_upper(std::string const &source);

	/** @brief Convert the string into its lowercase variant. */
	std::string make_lower(std::string const &source);

	/** @brief Join all of the strings together into one string. */
	std::string join(std::string const &delimiter, std::vector<std::string> const &source);

	/** @brief Join all of the strings together into one string. */
	std::string join(std::string const &delimiter, std::vector<std::string>::const_iterator start, std::vector<std::string>::const_iterator finish );

	/** @brief A trait to allow for case insensitive strings...assuming ASCII. */
	struct ci_ascii_char_traits: public std::char_traits<char>
	{
		/** @brief A helper function to cast to lowercase whenever possible. */
		static unsigned char asciiToLower(char c)
		{
			if (c >= 'A' && c <= 'Z')
			{
				return static_cast<unsigned char>(c + 'a' - 'A');
			}

			return static_cast<unsigned char>(c);
		}

		/** @brief Determine if the two characters match. */
		static bool eq(char c1, char c2)
		{
			return asciiToLower(c1) == asciiToLower(c2);
		}

		/** @brief Determine if the two characters do not match. */
		static bool ne(char c1, char c2)
		{
			return asciiToLower(c1) != asciiToLower(c2);
		}

		/** @brief Determine if the first character is less than the other character. */
		static bool lt(char c1, char c2)
		{
			return asciiToLower(c1) < asciiToLower(c2);
		}

		/** @brief Compare the two characters to see the proper order. */
		static int compare(char const *c1, char const *c2, size_t n)
		{
			while (n-- > 0)
			{
				auto d1 = asciiToLower(*c1++);
				auto d2 = asciiToLower(*c2++);
				if (d1 < d2)
				{
					return -1;
				}
				if (d1 > d2)
				{
					return 1;
				}
			}

			return 0;
		}

		/** @brief Attempt to find the wanted character. */
		static char const * find(char const *s, int n, char a)
		{
			auto d = asciiToLower(a);
			while (n-- > 0 && asciiToLower(*s) != d)
			{
				++s;
			}

			// sanity check in case something strange happened.
			if (asciiToLower(*s) == d)
			{
				return s;
			}
			return nullptr;
		}
	};

	using ci_ascii_string = std::basic_string<char, ci_ascii_char_traits>;
}

inline bool operator==(Rage::ci_ascii_string const &lhs, std::string const &rhs)
{
	return lhs == rhs.c_str();
}

inline bool operator!=(Rage::ci_ascii_string const &lhs, std::string const &rhs)
{
	return !operator==(lhs, rhs);
}

#endif
