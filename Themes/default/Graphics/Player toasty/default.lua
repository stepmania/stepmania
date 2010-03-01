local Player = ...
assert(Player);
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
				(cmd(linear,2.125;diffuse,Colors.Alpha( PlayerColor(Player), 0.345 );glow,color("1,1,1,0.5");decelerate,3;glow,Colors.Alpha( ColorDarkTone( PlayerColor(Player) ), 0 );diffuseramp;
				effectcolor1,ColorLightTone( PlayerColor(Player) );effectcolor2,PlayerColor(Player);
				effectclock,'beat';effectperiod,2;
				))(self);
			end
		end;
	};
};