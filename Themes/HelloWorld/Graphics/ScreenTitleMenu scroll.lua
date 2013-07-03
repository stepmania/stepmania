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
		InitCommand=function(self)
			self:zoom(0.85);
			self:y(-1);
			self:shadowlength(0);
			self:diffuse(color("#000000"));
			self:halign(1);
			self:strokecolor(color("0,0,0,0"));
		end;
		DisabledCommand=function(self)
			self:diffuse(color("0.45,0,0,1"));
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.15);
			self:diffuse(color("#000000"));
			self:strokecolor(color("0.85,0.9,1,0.75"));
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.2);
			self:diffuse(color("#888888"));
			self:strokecolor(color("0,0,0,0"));
		end;
		OffFocusedCommand=function(self)
			self:sleep(0.45);
			self:decelerate(1);
			self:addx(-32);
			self:diffuse(color("#FF000000"));
		end;
		OffUnfocusedCommand=function(self)
			self:sleep(0.125 * gc:GetIndex());
			self:linear(0.5);
			self:addx(SCREEN_CENTER_X);
		end;
	};
};

return t;