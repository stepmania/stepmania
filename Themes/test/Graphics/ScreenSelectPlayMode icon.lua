local gc = Var("GameCommand");
local index = gc:GetIndex();
local name  = gc:GetName();
local text  = gc:GetText();

local itemY = SCREEN_CENTER_Y*0.5 + (50 * index);

local t = Def.ActorFrame {
  InitCommand=cmd(x,SCREEN_CENTER_X;y,itemY);
  OffUnfocusedCommand=cmd(sleep,0.95/2;bounceend,0.3;zoomy,0);
--[[   OffFocusedCommand=function(self)
	local bIsFirst = (index == 0) and true or false;
	self:playcommand( (bIsFirst) and "LeftPick" or "RightPick");
  end; --]]
  OffFocusedCommand=cmd(sleep,0.5;linear,0.75;x,SCREEN_CENTER_X;zoom,1.25;sleep,0.75;decelerate,0.15;zoomy,0);
  LeftPickCommand=cmd(sleep,1.25;linear,0.25;x,-SCREEN_WIDTH);
  RightPickCommand=cmd(sleep,1.25;linear,0.25;x,SCREEN_WIDTH*2);

	Def.Quad {
		InitCommand=cmd(zoomto,274,42);
		OnCommand=cmd();
	};
	
	-- Under
	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(zoomto,274-4,42-4);
			OnCommand=cmd(diffuse,ThemeColor("Primary"));
		};
	};
	
	-- Over
	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(zoomto,274-4,42-4);
			OnCommand=cmd(diffuse,ThemeColor("Secondary");
				diffuseshift;effectcolor2,ThemeColor("Secondary");effectcolor1,GenericColor("cStealth");
				effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5);
				-- Explanation:
				-- 4 seconds for the each card to show on/off
				-- 0.25 seconds of each card to transition inbetween.
				-- 4.5 seconds to offset the transition of EffectColor2
				-- therefore
				-- EffectOffset = Time + TransitionTime*2
		};
		LoadFont("Common Stroke") .. {
			Text=name;
			InitCommand=cmd(shadowlength,0;zoomy,1.1);
			OnCommand=cmd(y,-1;diffuse,ThemeColor("Secondary");
				diffuseshift;effectcolor2,ThemeColor("Primary");effectcolor1,GenericColor("cStealth");
				effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5
				diffusealpha,0.5);
		};
		--[[
		Def.ActorFrame {
		  OnCommand=cmd(vibrate;effectmagnitude,0,4,0);
			LoadFont("Common Stroke") .. {
				Text=name;
				InitCommand=cmd(shadowlength,0;zoomy,1);
				OnCommand=cmd(y,-1;diffuse,ThemeColor("Secondary");
					diffuseshift;effectcolor2,ThemeColor("Primary");effectcolor1,GenericColor("cStealth");
					effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5);
			};
		};
		]]
		LoadFont("Common Normal") .. {
			Text=name;
			InitCommand=cmd(shadowlength,0);
			OnCommand=cmd(y,-1;diffuse,ThemeColor("Secondary");
				diffuseshift;effectcolor2,GenericColor("cWhite");effectcolor1,GenericColor("cStealth");
				effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5);
		};
	};

	-- Choose
	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(zoomto,274-4,42-4);
			OnCommand=cmd(
				diffuseshift;effectcolor1,ThemeColor("Secondary");effectcolor2,ThemeColor("Primary");
				effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5);
			GainFocusCommand=cmd(stoptweening;linear,0.1;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;linear,0.25;diffusealpha,0);
			OffFocusedCommand=cmd(sleep,0.25;linear,0.5;);
		};
 		LoadFont("Common Normal") .. {
			Text=name;
			InitCommand=cmd(shadowlength,1);
			OnCommand=cmd(y,-1;
				diffuseshift;effectcolor2,ThemeColor("Secondary");effectcolor1,ThemeColor("Primary");
				effectoffset,4.5;effecttiming,0.25,3.75,0.5,3.5);
			GainFocusCommand=cmd(stoptweening;zoom,1.2;decelerate,0.2;zoom,1;diffusealpha,1);
			LoseFocusCommand=cmd(stoptweening;zoom,1;decelerate,0.1;zoom,1;diffusealpha,0);
			OffFocusedCommand=cmd(settext,"OK!";);
		}; 
	};
};

return t;