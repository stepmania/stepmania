#ifndef RAGE_CONFIG_HPP_
#define RAGE_CONFIG_HPP_

#if defined(_MSC_VER) && _MSC_VER <= 1800
/** @brief A cross platform way of indicating a constant expression variable if the compiler was intelligent enough. */
#define CONSTEXPR_VARIABLE const
/** @brief A cross platform way of indicating a constant expression function if the compiler was intelligent enough. */
#define CONSTEXPR_FUNCTION
#else
/** @brief A cross platform way of indiciating a constant expression variable. */
#define CONSTEXPR_VARIABLE constexpr
/** @brief A cross platform way of indicating a constant expression function. */
#define CONSTEXPR_FUNCTION constexpr
#endif

#endif
