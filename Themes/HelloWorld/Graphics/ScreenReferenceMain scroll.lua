local gc = Var("GameCommand")
local itemName = gc:GetName()
local url = gc:GetUrl()

local t = Def.ActorFrame{
	-- focused frame
	LoadActor( THEME:GetPathG("SectionMenu","ItemFocused") )..{
		GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0);
	};
	-- unfocused
	LoadActor( THEME:GetPathG("SectionMenu","ItemUnfocused") )..{
		GainFocusCommand=cmd(decelerate,0.25;diffusealpha,0);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,1);
	};

	LoadActor( THEME:GetPathG("_section", "Reference") )..{
		InitCommand=cmd(x,-240;y,-23;Real;halign,0);
		GainFocusCommand=cmd(decelerate,0.25;diffusealpha,0.9);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.45);
	};

	Def.ActorFrame{
		LoadFont("_frutiger roman 24px")..{
			Name="SectionName";
			Text=THEME:GetString("ScreenReferenceMain",itemName);
			InitCommand=cmd(x,-246;y,-6;halign,0;diffuse,color("#222222FF"));
			GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.5);
		};
		LoadFont("_frutiger roman 24px")..{
			Name="SectionDesc";
			Text=THEME:GetString("ScreenReferenceMain",itemName.."Desc");
			InitCommand=cmd(x,-244;y,10;align,0,0;diffuse,color("#222222FF");zoom,0.5);
			GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.5);
		};
		LoadActor(THEME:GetPathG("_tango","webbrowser"))..{
			Name="UrlIcon";
			InitCommand=cmd(x,224;y,4;zoom,0.75;visible,url~="");
			GainFocusCommand=cmd(decelerate,0.25;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;decelerate,0.25;diffusealpha,0.5);
		};
	};
};

return t;