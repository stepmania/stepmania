local gc = Var("GameCommand");
local index = gc:GetIndex();
local name  = gc:GetName();
local text  = gc:GetText();

local itemColors = {
	HSV(128, 0.6, 0.85),	-- play
	HSV(192, 0.6, 0.85),	-- options
	HSV(160, 0.6, 0.85),	-- customization
	HSV( 64, 0.6, 0.85),	-- edit
	HSV(  0, 0.6, 0.85),	-- exit
};

local t = Def.ActorFrame{
	LoadFont("Common Normal")..{
		Text=Screen.String(text);
		InitCommand=function(self)
			self:diffuse(color("1,1,1,1"));
		end;
		DisabledCommand=function(self)
			self:diffuse(color("0.5,0.5,0.5,0.85"));
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.25);
			self:diffuse(color("1,0.2,0.2,1"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:linear(0.25);
			self:diffuse(color("1,1,1,1"));
		end;
	};
	--[[ begin triangle ]]
	Def.Quad{
		InitCommand=function(self)
			self:x(-SCREEN_WIDTH * 0.105);
			self:zoomto(8, 24);
			self:zwrite(true);
			self:blend('BlendMode_NoEffect');
		end;
	};
	Def.Quad{
		InitCommand=function(self)
			self:x(-SCREEN_WIDTH * 0.1);
			self:zoomto(12, 12);
			self:rotationz(45);
			self:diffuselowerleft(color("0,0,0,0"));
			self:ztest(true);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:visible(true);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:visible(false);
		end;
	};
	--[[ end triangle ]]
};

return t;