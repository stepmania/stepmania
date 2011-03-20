-- from scratch, with various references.
local Noteskin = ... or {};

-- element redirs
Noteskin.ElementRedirs = {
	-- ["element"] = "redir_element";
};

-- button redirs (since this is a bar noteskin, it's all the same)
Noteskin.ButtonRedirs = {
	-- dance
	Left		= "Red";
	UpLeft		= "Red";
	Up			= "Red";
	Down		= "Red";
	UpRight		= "Red";
	Right		= "Red";
	-- kb7
	Key1		= "Red";
	Key2		= "Red";
	Key3		= "Red";
	Key4		= "Yellow";
	Key5		= "Red";
	Key6		= "Red";
	Key7		= "Red";
	-- pump
	DownLeft	= "Bar";
	DownRight	= "Bar";
	Center		= "Bar";
};

-- things to blank out
Noteskin.Hide = {
	-- ["element"] = true/false;
	["Hold Topcap Active"] = true,
	["Hold Topcap Inactive"] = true,
	["Roll Topcap Active"] = true,
	["Roll Topcap Inactive"] = true,
	["Hold Tail Active"] = true,
	["Hold Tail Inactive"] = true,
	["Roll Tail Active"] = true,
	["Roll Tail Inactive"] = true
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