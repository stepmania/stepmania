-- from scratch, with various references.
local Noteskin = ... or {};

-- element redirs
Noteskin.ElementRedirs = {
	-- ["element"] = "redir_element";
	-- Instead of separate hold heads, use the tap note graphics.
	["Hold Head Inactive"] = "Tap Note";
	["Hold Head Active"] = "Tap Note";
	["Roll Head Inactive"] = "Tap Note";
	["Roll Head Active"] = "Tap Note";
};

-- button redirs
-- o2jam uses this order:
-- [white][blue][white][yellow][white][blue][white]
Noteskin.ButtonRedirs = {
	-- dance
	Left		= "White";
	Up			= "White";
	UpLeft		= "Blue";
	Down		= "White";
	UpRight		= "Blue";
	Right		= "White";
	-- kb7
	Key1		= "White";
	Key2		= "Blue";
	Key3		= "White";
	Key4		= "Yellow";
	Key5		= "White";
	Key6		= "Blue";
	Key7		= "White";
};

-- things to blank out
Noteskin.Hide = {
	-- ["element"] = true/false;
};

-- rotations
Noteskin.BaseRotX = {
	Left	= 0;
	UpLeft	= 0;
	Up		= 0;
	Down	= 0;
	UpRight	= 0;
	Right	= 0;
};
Noteskin.BaseRotY = {
	Left	= 0;
	UpLeft	= 0;
	Up		= 0;
	Down	= 0;
	UpRight	= 0;
	Right	= 0;
};

local function NoteskinLoader()
	local Button = Var "Button"
	local Element = Var "Element"

	if Noteskin.Hide[Element] then
		-- Return a blank element. If SpriteOnly is set, we need to return a
		-- sprite; otherwise just return a dummy actor.
		local t;
		if Var "SpriteOnly" then
			t = LoadActor( "_blank" );
		else
			t = Def.Actor {};
		end
		return t .. {
			cmd(visible,false);
		};
	end;

	-- load element and button, using redirs
	local LoadElement = Noteskin.ElementRedirs[Element]
	if not LoadElement then
		LoadElement = Element;
	end;

	local LoadButton = Noteskin.ButtonRedirs[Button]
	if not LoadButton then
		LoadButton = Button;
	end;

	-- get path to thing
	local sPath = NOTESKIN:GetPath( LoadButton, LoadElement );

	-- make actor
	local t = LoadActor( sPath );

	-- apply rotation
	t.BaseRotationX=Noteskin.BaseRotX[sButton]
	t.BaseRotationY=Noteskin.BaseRotY[sButton]

	return t;
end

Noteskin.Load = NoteskinLoader;
Noteskin.CommonLoad = NoteskinLoader;
return Noteskin;