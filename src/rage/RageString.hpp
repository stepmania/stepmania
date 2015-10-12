#ifndef RAGE_STRING_HPP_
#define RAGE_STRING_HPP_

#include <string>

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

	/** @brief Offer a clean way of hexify-ing a wide character. Primarily used for fonts.
	 *
	 * See http://stackoverflow.com/a/8024386 */
	std::string hexify(wchar_t const src, unsigned int dstlen);
}

#endif
