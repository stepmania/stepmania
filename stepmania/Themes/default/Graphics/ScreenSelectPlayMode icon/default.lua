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
			OnCommand=cmd(rotationx,90;diffusealpha,0;linear,.11;diffusealpha,.5;rotationx,0;linear,.11;diffusealpha,1);
			OffCommand=cmd(rotationy,90;diffusealpha,1;linear,.2;rotationy,90;diffusealpha,0);
		};
		LoadFont( "_terminator two 40px" ) .. {
			InitCommand=cmd(x,94;settext,string.upper(gc:GetText());shadowlength,0;zoom,1.3;zoomtowidth,300;rotationz,-90;diffuse,c;strokecolor,color("#00000044");shadowlengthx,4;shadowlengthy,0;);
			OnCommand=cmd(diffusealpha,0;addy,-50;decelerate,.2;diffusealpha,1;addy,50);
			OffCommand=cmd(diffusealpha,1;accelerate,.2;diffusealpha,0;addy,-50);
		};
		LoadFont( "_venacti Bold 24px" ) .. {
			InitCommand=cmd(horizalign,right;vertalign,bottom;x,70;y,150;settext,ScreenString(gc:GetName().."Explanation");shadowlengthx,0;shadowlengthy,3;maxwidth,500;diffuse,c;strokecolor,color("#00000044"););
			OnCommand=cmd(zoomy,0;diffusealpha,.5;linear,.18;zoomy,1;diffusealpha,1);
			OffCommand=cmd(zoomy,1;diffusealpha,1;linear,.18;zoomy,.2;diffusealpha,0);
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-150;y,SCREEN_CENTER_Y-104+gc:GetIndex()*50;);
		LoadActor( "cursor" ) .. {
			InitCommand=cmd(x,20;y,2;);
			GainFocusCommand=cmd(visible,true);
			LoseFocusCommand=cmd(visible,false);
			OnCommand=cmd(cropright,1;faderight,1;linear,.2;cropright,0;faderight,0);
			OffCommand=cmd(cropleft,0;fadeleft,0;linear,.2;cropleft,1;fadeleft,1);
		};
		LoadActor( "icon " .. gc:GetName() ) .. {
			InitCommand=cmd(x,124;y,2;);
			GainFocusCommand=cmd(visible,true);
			LoseFocusCommand=cmd(visible,false);
			OnCommand=cmd(addx,-90;diffusealpha,0;decelerate,.12;diffusealpha,.5;addx,90;linear,.12;diffusealpha,1);
			OffCommand=cmd(sleep,.18;bouncebegin,.12;zoom,0;diffusealpha,0);
		};
		LoadFont( "_terminator two 30" ) .. {
			InitCommand=cmd(y,-6;settext,string.upper(gc:GetText());maxwidth,350;diffuse,c;strokecolor,color("#0000004F");shadowlengthx,0;shadowlengthy,4;shadowcolor,color("#00000044"););
			OnCommand=cmd(diffusealpha,0;linear,.2;diffusealpha,1);
			OffCommand=cmd(diffusealpha,1;sleep,index/16;linear,.1;diffusealpha,0);
		};
	};
};

return t;