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
		typedef SongID argument_type;
		typedef std::size_t result_type;
 
		result_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<CourseID>
	{
		typedef CourseID argument_type;
		typedef std::size_t result_type;
 
		result_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<StepsID>
	{
		typedef StepsID argument_type;
		typedef std::size_t result_type;
 
		result_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
	template<>
    struct hash<TrailID>
	{
		typedef TrailID argument_type;
		typedef std::size_t result_type;
 
		result_type operator()(argument_type const& s) const
		{
			return std::hash<std::string>()(std::string(s.ToString()));
		}
	};
}

#endif
