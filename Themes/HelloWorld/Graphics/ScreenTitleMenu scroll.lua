-- This file is used to define how the items should look on ScreenTitleMenu.
-- The syntax is different because I took this code from moonlight. :)

--[[
Concepts used:
* THEME:GetString(screen,string)
* zoom
* shadowlength
* diffuse
* halign (0 = left, 0.5 = middle, 1 = right)
* strokecolor
* stoptweening
* accelerate
--]]

-- this is used as a shorthand for GameCommand.
local gc = Var("GameCommand")

local t = Def.ActorFrame{
	LoadFont( "_frutiger roman" ) ..{
		-- This line tries to find a corresponding line in the current language
		-- ini file; If it doesn't, (and you haven't disabled error messages,)
		-- then an error will pop up telling you where you need to add the
		-- string.
		Text=THEME:GetString( 'ScreenTitleMenu', gc:GetText() );
		InitCommand=cmd(zoom,0.85;y,-1;shadowlength,0;diffuse,color("#000000");halign,1;strokecolor,color("0,0,0,0"););
		DisabledCommand=cmd( diffuse,color("0.45,0,0,1") );
		GainFocusCommand=cmd(stoptweening;accelerate,0.15;diffuse,color("#000000");strokecolor,color("0.85,0.9,1,0.75"););
		LoseFocusCommand=cmd(stoptweening;accelerate,0.2;diffuse,color("#888888");strokecolor,color("0,0,0,0"););
		OffFocusedCommand=cmd(sleep,0.45;decelerate,1;addx,-32;diffuse,color("#FF000000"););
		OffUnfocusedCommand=cmd(sleep,0.125*gc:GetIndex();linear,0.5;addx,SCREEN_CENTER_X;);
	};
};

return t;