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
}

#endif
