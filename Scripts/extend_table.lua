-- Utilities for better table manipulation

-- split a string into a table
-- http://lua-users.org/wiki/SplitJoin (but with error messages)
function string.split(self, sSeparator, nMax, bRegexp)
	assert(sSeparator ~= '', "empty separator is not allowed.")
	assert(nMax == nil or nMax >= 1, "max must be a positive number.")

	local aRecord = {}

	if self:len() > 0 then
		local bPlain = not bRegexp
		nMax = nMax or -1

		local nField=1 nStart=1
		local nFirst,nLast = self:find(sSeparator, nStart, bPlain)
		while nFirst and nMax ~= 0 do
			aRecord[nField] = self:sub(nStart, nFirst-1)
			nField = nField+1
			nStart = nLast+1
			nFirst,nLast = self:find(sSeparator, nStart, bPlain)
			nMax = nMax-1
		end
		aRecord[nField] = self:sub(nStart)
	end

	return aRecord
end

-- table.concat alias for convenience.
table.join = table.concat

-- insert multiple elements into a table at once
function table.push(self, ...)
	for _, v in ipairs({...}) do
		table.insert(self, v)
	end
end
