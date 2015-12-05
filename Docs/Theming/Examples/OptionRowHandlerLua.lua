-- This will discuss how to create an option row in lua for use on a normal options screen.

-- To try out this example, copy this file to Scripts/ and add the following
-- line to metrics.ini, on one of the options screens.  Change the "1" to an
-- appropriate line name and make sure it's in the LineNames list for that
-- screen.
-- Line1="lua,FooMods()"

-- Make sure you test this with Player 2, Player 1 can't interact with this
-- option row as part of the example.
-- When on the screen testing the example, flush the log and read it when
-- interacting with the option row.  This example doesn't actually apply any
-- modifiers to the player, it just prints messages to the log file so you can
-- see what functions are being called.

-- Comments explaining an element come before the element they explain.

-- The function "FooMods" returns a table containing all the information the
-- option row handler needs to build the option row.
function FooMods()
	return {
		-- A string with the name of the row.  This name will be localized using
		-- the entry in the "OptionTitles" section of the language file.
		Name= "Foo",

		-- A boolean that controls whether this row goes to the "next row"
		-- element when NavigationMode for the screen is "toggle" and the
		-- ArcadeOptionsNavigation preference is 1.
		GoToFirstOnStart= false,

		-- A boolean controlling whether the choice affects all players.
		OneChoiceForAllPlayers= false,

		-- A boolean controlling whether SaveSelections is called after every
		-- change.  If this is true, SaveSelections will be called every time
		-- the player moves the cursor on the row.
		ExportOnChange= false,

		-- A LayoutType enum value.  "ShowAllInRow" shows all items in the row,
		-- "ShowOneInRow" shows only the choice with focus is shown.
		-- "ShowOneInRow" is forced if there are enough choices that they would go
		-- off screen.
		LayoutType= "ShowAllInRow",

		-- A SelectType enum value.  "SelectOne" allows only one choice to be
		-- selected.  "SelectMultiple" allows multiple to be selected.
		-- "SelectNone" allows none to be selected.
		SelectType= "SelectMultiple",

		-- Optional function.  If non-nil, this function must return a table of
		-- PlayerNumbers that are allowed to use the row.
		-- A row that can't be used by one player can be confusing for the players
		-- so consider carefully before using this.
		-- This function will be called an extra time during loading to ensure
		-- it returns a table.
		EnabledForPlayers= function(self)
			Trace("FooMods:EnabledForPlayers() called.")
			-- Leave out PLAYER_1 just for example.
			return {PLAYER_2}
		end,

		-- A table of strings that are the names of choices.  Choice names are not
		-- localized.
		Choices= {"a", "b", "c", "d"},

		-- Optional table.  If non-nil, this table must contain a list of messages
		-- this row should listen for.  If one of the messages is recieved, the
		-- row is reloaded (and the EnabledForPlayers function is called if it is
		-- non-nil).
		ReloadRowMessages= {"ReloadFooMods"},

		-- LoadSelections should examine the player and figure out which options
		-- on the row the player has selected.
		-- self is the table returned by the original function used to create the
		-- option row. (the table being created right now).
		-- list is a table of bools, all initially false.  Set them to true to
		-- indicate which options are on.
		-- pn is the PlayerNumber of the player the selections are for.
		LoadSelections= function(self, list, pn)
			Trace("FooMods:LoadSelections(" .. pn .. ")")
			for i, choice in ipairs(self.Choices) do
				-- Randomly set some to true just for an example.
				if math.random(0, 1) == 1 then
					Trace(choice .. " (" .. i .. ")" .. " set to true.")
					list[i]= true
				end
			end
		end,

		-- SaveSelections should examine the list of what the player has selected
		-- and apply the appropriate modifiers to the player.
		-- Same args as LoadSelections.
		SaveSelections= function(self, list, pn)
			Trace("FooMods:SaveSelections(" .. pn .. ")")
			for i, choice in ipairs(self.Choices) do
				if list[i] then
					Trace(choice .. " (" .. i .. ")" .. " set to true.")
				end
			end
		end,

		-- Optional function.  If non-nil, this function must take 3 parameters
		-- (self, pn, choice), and return a bool.  It is called when a player
		-- selects an item in the row by pressing start.
		-- self is the same as for LoadSelections.
		-- pn is the PlayerNumber of the player that made the selection.
		-- choice is the choice the player's cursor is on.
		-- The return value should be true if the Choices table is changed.  If it
		-- is true, then LoadSelections will be called to update which choices are
		-- underlined for each player.
		-- This function is meant to provide a way for a menu to change the text of
		-- its choices.  If it returns true, LoadSelections will be called for each
		-- player.  If OneChoiceForAllPlayers is true, this function will be called
		-- for each player, which means LoadSelections will be called twice for
		-- each player.  Well written code shouldn't have a problem with this.
		NotifyOfSelection= function(self, pn, choice)
			Trace("FooMods:NotifyOfSelection(" .. pn .. ", " .. choice .. ")")
			-- Randomly decide whether to change, as an example.
			local change= math.random(0, 3)
			-- No change half the time, lengthen or clip strings the other half.
			if change < 2 then
				Trace("Not changing choices.")
			else
				Trace("Changing choices.")
			end
			if change == 2 then
				for i, choice in ipairs(self.Choices) do
					self.Choices[i]= choice .. choice
				end
			elseif change == 3 then
				for i, choice in ipairs(self.Choices) do
					self.Choices[i]= choice:sub(1, 1)
				end
			end
			-- Don't actually broadcast a reload message from here, do it from some
			-- other place that actually has a reason to trigger a reload.  This is
			-- just here so that nothing needs to be added to a screen's lua files
			-- for this example.
			if math.random(0, 5) == 0 then
				Trace("Broadcasting reload message.")
				MESSAGEMAN:Broadcast("ReloadFooMods")
			end
			return change >= 2
		end
	}
end
