#include "RageMath.hpp"

#include <array>
#include <cmath>

float Rage::FastSin(float x)
{
	// These tables go from 0 to PI.
	// Utilize 1024 entries: 1024 * 4 == 4096 == one page of memory in many OSes.
	// TODO: Find a way to generate this during compilation time.
	static std::array<float, 1024> table;
	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;
		// range for is not appropriate: need the index.
		for (unsigned i = 0; i < table.size(); ++i)
		{
			float z = Rage::scale( i + 0.f, 0.f, table.size() + 0.f, 0.f, Rage::PI );
			table[i] = std::sin(z);
		}
	}
	
	if (x == 0)
	{
		return 0;
	}
	
	float index = Rage::scale(x, 0.f, Rage::PI * 2, 0.f, table.size() * 2.f);
	
	// lerp using the samples from the table.
	// Warning to the future: Changing sampleIndex to store size_t will break
	// this function.  Not sure why. -Kyz
	std::array<int, 2> sampleIndex;
	sampleIndex[0] = static_cast<int>(std::floor(index));
	sampleIndex[1] = sampleIndex[0] + 1;
	
	float remainder = index - sampleIndex[0];
	for (auto &sample: sampleIndex)
	{
		sample %= table.size() * 2;
	}
	
	// TODO: Use a library like the GSL to ensure the remainder is within [0,1].
	
	std::array<float, 2> values;
	for (unsigned i = 0; i < values.size(); ++i)
	{
		int &sample = sampleIndex[i];
		float &val = values[i];
		
		// If Rage::PI <= sample < 2 * PI
		if (static_cast<size_t>(sample) >= table.size())
		{
			sample -= table.size();
			// TODO: Use a library like the GSL to ensure the sample is within our range.
			val = -table[sample];
		}
		else
		{
			val = table[sample];
		}
	}
	
	return Rage::scale(remainder, 0.f, 1.f, values[0], values[1]);
}

float Rage::FastCos(float x)
{
	return Rage::FastSin( x + (Rage::PI * .5f) );
}

float Rage::FastTan(float x)
{
	return Rage::FastSin( x ) / Rage::FastCos( x );
}

float Rage::FastCsc(float x)
{
	return 1 / Rage::FastSin( x );
}

float Rage::TriangleWave(float x)
{
	float fAngle= fmod(x, PI * 2.0f);
	if(fAngle < 0.0f)
	{
		fAngle+= PI * 2.0;
	}
	double result= fAngle * (1 / PI);
	if(result < .5)
	{
		return result * 2.0;
	}
	else if(result < 1.5)
	{
		return 1.0 - ((result - .5) * 2.0);
	}
	else
	{
		return -4.0 + (result * 2.0);
	}
}

float Rage::SquareWave(float x)
{
	float fAngle = fmod( x , (PI * 2.0f) );
		//Hack: This ensures the hold notes don't flicker right before they're hit.
		if(fAngle < 0.01)
		{
		    fAngle+= PI * 2.0;
		}
	return fAngle >= PI ? -1.0 : 1.0;
}
