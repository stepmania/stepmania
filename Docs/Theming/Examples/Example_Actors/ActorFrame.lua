-- A simple ActorFrame example with a couple Actor children and attributes.
-- An ActorFrame is used to contain other actors, giving them a common origin
--   and organizing them.

Def.ActorFrame{
	Name= "Simple frame",
	-- Amount to multiply the delta time by when updating.
	UpdateRate= 2,

	-- The angular width of the field of view used to render children.
	-- This will ruin your frame rate, do not use.  -1 means no effect.
	--FOV= -1,
	-- The position of the vanishing point used for the FOV rendering.
	--VanishX= _screen.cx,
	--VanishY= _screen.cy,

	-- Whether to apply lighting.  Incomplete implementation, do not use.
	--Lighting= true,
	-- Different color values used when lighting is applied.
	--AmbientColor= ".75,.75,.75",
	--DiffuseColor= "0,0,.25",
	--SpecularColor= ".25,0,0",

	-- A couple children, just to have something to list later.
	Def.Actor{Name= "Simple child"},
	Def.Actor{Name= "Other child"},

	InitCommand= function(self)
		local children= self:GetChildren()
		for name, child in pairs(children) do
			Trace("Has child " .. name)
		end
	end
}
