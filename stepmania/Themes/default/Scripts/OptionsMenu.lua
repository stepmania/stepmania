-- Sample options menu item.  
function OptionsRowTest()
	local function Set(self,list,pn)
		if list[1] then
			Trace("FOO: 1")
		end
		if list[2] then
			Trace("FOO: 2")
		end
	end
	return 
	{
		-- Name is used to retrieve the header and explanation text.
		Name = "Foo",

		-- Flags for this row.  Note that as this table only defines
		-- a row, not a menu, only row settings can be set here, not
		-- OptionMenuFlags.
		LayoutType = "ShowAllInRow",
		SelectType = "SelectMultiple",
		OneChoiceForAllPlayers = false,
		ExportOnChange = false,
		
		-- Choices are not resolved as metrics, since they might
		-- be dynamic.  Add THEME Lua hooks if we want to translate
		-- these.
		Choices = { "Option1", "Option2" },
		-- Or:
		-- for i = 1,20 do Choices[i] = "Option " .. i end

		-- Set list[1] to true if Option1 should be selected, and
		-- list[2] if Option2 should be selected.  This will be
		-- called once per enabled player.
		LoadSelections = (function(self,list,pn) list[1] = true; end),
		SaveSelections = Set,
	}
end

-- This option row loads and saves the results to a table.  For example, if you
-- have options "fast" and "slow", and the name of the option is "run", then the
-- table will be set to table["run"] = "fast" (or "slow"), and loaded appropriately.
-- (This could handle SelectMultiple, by saving the result to a table, eg.
-- table["run"]["fast"] = true.)

OptionRowTable =
{
	SaveTo = nil, -- set this
	Default = nil, -- set this

	LoadSelections = function(self, list, pn)
		local Sort = self.SaveTo[self.Name] or self.Default
		-- Find the index of the current sort.
		local Index = FindValue(self.RawChoices, Sort) or 1
		list[Index] = true
	end,

	SaveSelections = function(self, list, pn)
		local Selection = FindSelection( list )
		self.SaveTo[self.Name] = self.RawChoices[Selection]
	end
}

-- (c) 2005 Glenn Maynard
-- All rights reserved.
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.

