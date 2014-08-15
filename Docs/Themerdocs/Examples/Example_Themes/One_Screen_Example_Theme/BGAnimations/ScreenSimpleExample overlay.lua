-- This screen file returns an ActorFrame with two actors.

-- A screen file must return an ActorFrame.
-- All actors on the screen must be inside this ActorFrame.
return Def.ActorFrame{
	-- An actor is added to a frame by calling a function that returns an actor.
	-- Most of the functions for creating actors are in the Def table.

	-- All of the functions in the Def table take a table with the parameters
	-- to construct the actor.

	-- For convenience, Lua makes the parentheses normally used when calling a
	-- function optional when that function's sole argument is a table.
	-- "foo({...})" and "foo{...}" are equivalent in lua.

	-- Quad is a very simple actor, a colored rectangle.
	Def.Quad{
		-- The parameters for an actor are set through entries in a table.
		-- This is just a simple example, so it only sets the required parts.

		-- Every actor should have a name.
		-- The name can be used to find the actor later.
		-- If you don't set the name, the actor will have a blank name.
		-- The name should be unique within the ActorFrame the actor is inside.
		-- See the entries for ActorFrame:GetChild and ActorFrame:GetChildren in
		-- Lua.xml for details on what happens when two actors in the same
		-- ActorFrame have the same name.
		-- Actors that are inside different ActorFrames can have the same name
		-- without causing any problems.
		Name= "example",

		-- The InitCommand for an actor should initialize any parts of the actor
		-- that should be set before the first time it draws.
		-- If you don't set the InitCommand, the actor will be in its default
		-- state.  Details of the default state of an actor can be found in the
		-- example for that actor.
		InitCommand= function(self)
			-- The details of these functions can be found in Lua.xml.
			-- Position, size, and color should be initialized for a quad.
			self:xy(SCREEN_CENTER_X, SCREEN_CENTER_Y)
			self:setsize(20, 20)
			self:diffuse(color("1,1,1,1"))
		end,
	},
	-- The contents of an ActorFrame are entries in a table, so we separate
	-- them with commas.
	Def.Quad{
		-- This quad is almost identical to the previous one, so there is nothing
		-- new here to explain.
		Name= "example2",
		InitCommand= function(self)
			self:xy(SCREEN_CENTER_X, SCREEN_CENTER_Y-40)
			self:setsize(20, 20)
			self:diffuse(color("0,0,1,1"))
		end,
	},
}
