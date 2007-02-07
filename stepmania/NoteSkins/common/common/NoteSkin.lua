local ret = ... or {}

ret.Redir = function(sButton, sElement)
	-- To redirect files for Up to Down:
	-- if sButton == "Up" then sButton = "Down"; end
	return sButton, sElement;
end

ret.Rotate =
{
};

ret.PartsToRotate =
{
};

ret.Blank =
{
-- To blank tap notes:
-- ["tap note"] = true,
};

local function func()
	local sButton = Var "Button";
	local sElement = Var "Element";

	if ret.Blank[sElement] then
		-- Return a blank element.  If SpriteOnly is set,
		-- we need to return a sprite; otherwise just return
		-- a dummy actor.
		if Var "SpriteOnly" then
			local t = LoadActor( "_blank" );
			return t;
		else
			return Def.Actor {};
		end
	end

	local sButtonToLoad, sElementToLoad = ret.Redir(sButton, sElement);
	assert( sButtonToLoad );
	assert( sElementToLoad );

	local sPath = NOTESKIN:GetPath( sButtonToLoad, sElementToLoad );

	local t = LoadActor( sPath );

	if ret.PartsToRotate[sElement] then
		t.BaseRotationZ = ret.Rotate[sButton];
	end

	return t;
end

-- This is the only required function.
ret.Load = func;

return ret;
