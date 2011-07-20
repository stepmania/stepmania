local Player = ...
assert(Player);
local HasToasty = false;
local fWidth = ( GAMESTATE:GetCurrentStyle():GetStyleType() == 'StyleType_OnePlayerTwoSides' ) and 600 or 256+16;
return Def.ActorFrame {
	ToastyAchievedMessageCommand=function(self,params)
		if params.PlayerNumber == Player then
			(cmd(thump,1;effectclock,'beat';effectmagnitude,1,1,1;
			effectcolor1,color("1,1.125,1,1");effectcolor2,color("1,1,1,1")))(self);
		end
	end;
	Def.Quad {
		InitCommand=cmd(zoomto,fWidth,SCREEN_HEIGHT;diffuse,PlayerColor(Player);diffusealpha,0;fadeleft,32/(256+16);faderight,32/(256+16));
		ToastyAchievedMessageCommand=function(self,params)
			if params.PlayerNumber == Player then
				(cmd(stoptweening;linear,2.125;diffuse,Color.Alpha( PlayerColor(Player), 0.345 );glow,color("1,1,1,0.5");decelerate,3;glow,Color.Alpha( ColorDarkTone( PlayerColor(Player) ), 0 );diffuseramp;
				effectcolor1,ColorLightTone( PlayerColor(Player) );effectcolor2,PlayerColor(Player);
				effectclock,'beat';effectperiod,2;
				))(self);
				HasToasty = true;
			end
		end;
		ToastyDroppedMessageCommand=function(self,params)
			if params.PlayerNumber == Player then
				if HasToasty then
					(cmd(finishtweening;stopeffect;glow,color("1,1,1,0.5");decelerate,0.35;diffuse,Color.Alpha( Color("Black"), 0.25 );glow,color("1,1,1,0");linear,0.35*0.25;diffusealpha,0))(self);
					HasToasty = false;
				else
					return
				end
			end
		end;
	};
};