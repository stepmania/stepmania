#ifndef ARCH_INTERNAL
#define ARCH_INTERNAL 1

#include <map>

template <class Driver>
class DriverList
{
public:
	typedef Driver *(*Generator)();
	static void AddDriver(int priority, Generator gen)
	{
		drivers.insert(pair<int, Generator>(priority, gen));
	}
	
	static Driver *Generate()
	{
		Driver *ret = NULL;

		for(multimap<int, Generator>::iterator i = drivers.begin();
			i != drivers.end(); ++i)
		{
			try {
				ret = i->second();
			} catch(RageException e) {
				/* XXX */
			}
		}

		return ret;
	}

	static multimap<int, Generator> drivers;
};

template <class Driver, class Type>
class DriverEntry
{
public:
	static Driver *Generate() { return new Type; }

	DriverEntry()
	{
		DriverList<RageSoundDriver>::AddDriver(1, 
			DriverEntry<Driver,Type>::Generate);
	}
};

#endif
