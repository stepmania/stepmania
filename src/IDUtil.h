#ifndef ID_UTIL_H
#define ID_UTIL_H

// This is a specialization of the std::hash template for handling the various
// types that have ToString functions, so they can be used in unordered_map.
// It is based on this example: http://en.cppreference.com/w/cpp/utility/hash
// I haven't figured out how to use templates right yet, so I copy pasted to
// cover all our custom types. -Kyz
namespace std
{
	template<>
    struct hash<SongID>
	{
		std::size_t operator()(SongID const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<CourseID>
	{
		std::size_t operator()(CourseID const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<StepsID>
	{
		std::size_t operator()(StepsID const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<TrailID>
	{
		std::size_t operator()(TrailID const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
}

#endif
