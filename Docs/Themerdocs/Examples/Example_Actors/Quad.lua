-- A simple quad example.
-- Quads are used to create… well, quads.

Def.Quad{
	-- Quads have no special attributes, but they do have a few special functions.
	OnCommand=function(self)
		self:SetWidth(40) -- scales the quad's width to 40
		self:SetHeight(40) -- scales the quad's height to 40
		-- SetWidth and SetHeight must be capitalized to work.
		-- SetSize (or setsize) is a shorthand for SetWidth and SetHeight. 
		-- The syntax is setsize(x, y), where x is the width and y is the height.
		-- The default width and height is 1, which is less than 1 pixel on some displays

		self:diffuse(Color.Red)
		-- Sets the color to red. 
		-- A list of all possible names for the colors can be found in
		-- Themes/_fallback/Scripts/02 Colors.lua
		
		self:Center()
		-- places the quad in the middle of the screen
	end
}
