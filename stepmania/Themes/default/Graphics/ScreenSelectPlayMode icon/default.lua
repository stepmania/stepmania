local gc = Var("GameCommand");
local master_pn = GAMESTATE:GetMasterPlayerNumber();
local pad_file = "";
local index = gc:GetIndex();
local c = color("#000000");
if index == 0 then
	c = color("#00e110");
elseif index == 1 then
	c = color("#e4e100");
elseif index == 2 then
	c = color("#fba500");
elseif index == 3 then
	c = color("#ff0000");
elseif index == 4 then
	c = color("#1cd8ff");
end

local t = Def.ActorFrame {
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+140;y,SCREEN_CENTER_Y;);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
		LoadActor( "preview " .. gc:GetName() ) .. {
			InitCommand=cmd(y,170;vertalign,bottom;);
		};
		LoadFont( "_terminator two stroke 92" ) .. {
			InitCommand=cmd(x,94;settext,string.upper(gc:GetText());shadowlength,0;zoom,0.7;zoomtowidth,300;rotationz,-90;diffuse,color("#000000");diffusealpha,0.35);
		};
		LoadFont( "_terminator two 92" ) .. {
			InitCommand=cmd(x,94;settext,string.upper(gc:GetText());shadowlength,0;zoom,0.7;zoomtowidth,300;rotationz,-90;diffuse,c;shadowlengthx,4;shadowlengthy,0;);
		};
		LoadFont( "_euromode 44" ) .. {
			InitCommand=cmd(horizalign,right;vertalign,bottom;x,72;y,150;settext,ScreenString(gc:GetName().."Explanation");shadowlengthx,0;shadowlengthy,3;maxwidth,500;diffuse,c;strokecolor,color("#000000"););
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-150;y,SCREEN_CENTER_Y-104+gc:GetIndex()*50;);
		LoadActor( "cursor" ) .. {
			InitCommand=cmd(x,20;y,2;);
			GainFocusCommand=cmd(visible,true);
			LoseFocusCommand=cmd(visible,false);
		};
		LoadActor( "icon " .. gc:GetName() ) .. {
			InitCommand=cmd(x,124;y,2;);
			GainFocusCommand=cmd(visible,true);
			LoseFocusCommand=cmd(visible,false);
		};
		LoadFont( "_terminator two stroke 60" ) .. {
			InitCommand=cmd(y,-6;settext,string.upper(gc:GetText());shadowlength,0;maxwidth,350;diffuse,color("#000000");diffusealpha,0.35;);
		};
		LoadFont( "_terminator two 60" ) .. {
			InitCommand=cmd(y,-6;settext,string.upper(gc:GetText());shadowlength,0;maxwidth,350;diffuse,c;shadowlengthx,0;shadowlengthy,4;);
		};
	};
};

return t;