local gc = Var("GameCommand")
local itemName = gc:GetName()

local t = Def.ActorFrame{
	-- focused frame
	LoadActor( THEME:GetPathG("ReferenceBase","ItemFocused") )..{
		GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0);
	};
	-- unfocused
	LoadActor( THEME:GetPathG("SectionMenu","ItemUnfocused") )..{
		GainFocusCommand=cmd(decelerate,0.25;diffusealpha,0);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,1);
	};

	Def.ActorFrame{
		LoadFont("_frutiger roman 24px")..{
			Name="SectionName";
			Text=ScreenString(itemName);
			InitCommand=cmd(x,-250;y,-6;halign,0;diffuse,color("#222222FF"));
			GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.5);
		};
		LoadFont("_frutiger roman 24px")..{
			Name="SectionDesc";
			Text=ScreenString(itemName.."Desc");
			InitCommand=cmd(x,-248;y,10;align,0,0;diffuse,color("#222222FF");zoom,0.5);
			GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.5);
		};
	};
};

return t;