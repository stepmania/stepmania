-- freem, inc. DateTime for StepMania
-- todo: accept format parameters for things

-- reference: http://us.php.net/manual/en/function.date.php
local dateChars = {
	--[[ day ]]
	'd', -- Day of the month, 2 digits with leading zeros (01-31)
	'D', -- A textual representation of a day, three letters ("Mon"-"Sun")
	'j', -- Day of the month without leading zeros (1-31)
	'l', -- A full textual representation of the day of the week
	'N', -- ISO-8601 numeric representation of the day of the week (1=Mon,7=Sun)
	'S', -- English ordinal suffix for the day of the month, 2 characters
	'w', -- Numeric representation of the day of the week (0=Sun,6=Sat)
	'z', -- The day of the year (0-365)
	--[[ week ]]
	'W', -- ISO-8601 week number of year, weeks starting on Monday
	--[[ month ]]
	'F', -- A full textual representation of a month, such as January or March
	'm', -- Numeric representation of a month, with leading zeros
	'M', -- A short textual representation of a month, three letters
	'n', -- Numeric representation of a month, without leading zeros
	't', -- Number of days in the given month
	--[[ year ]]
	'L', -- Whether it's a leap year (1 or 0)
	'o', -- (sux) ISO-8601 year number
	'Y', -- A full numeric representation of a year, 4 digits
	'y', -- (sux) A two digit representation of a year
	--[[ time ]]
	'a', -- Lowercase Ante meridiem and Post meridiem
	'A', -- Uppercase Ante meridiem and Post meridiem
	'B', -- Swatch Beats
	'g', -- 12-hour format of an hour without leading zeros
	'G', -- 24-hour format of an hour without leading zeros
	'h', -- 12-hour format of an hour with leading zeros
	'H', -- 24-hour format of an hour with leading zeros
	'i', -- Minutes with leading zeros
	's', -- Seconds, with leading zeros
	'u', -- Microseconds
	--[[ timezone ]]
	'e', -- Timezone identifier
	'I', -- Whether or not the date is in daylight saving time
	'O', -- Difference to Greenwich time (GMT) in hours
	'P', -- Difference to Greenwich time (GMT) with colon between hours and minutes
	'T', -- Timezone abbreviation
	'Z', -- Timezone offset in seconds.
	--[[ full datetime ]]
	'c', -- ISO 8601 date
	'r', -- RFC 2822 formatted date
	'U' -- Seconds since the Unix Epoch (January 1 1970 00:00:00 GMT)
}

function date(format,...)
	if ... then
		-- convert value
	else
		-- convert current time	
	end
end

Date = {
	Today = function()
		return string.format("%i%02i%02i", Year(), (MonthOfYear()+1), DayOfMonth())
	end
}

Time = {
	Now = function()
		return string.format( "%02i:%02i:%02i", Hour(), Minute(), Second() )
	end
}