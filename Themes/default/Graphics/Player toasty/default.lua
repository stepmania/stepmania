local Player = ...
assert(Player);
local HasToasty = false;
local fWidth = ( GAMESTATE:GetCurrentStyle():GetStyleType() == 'StyleType_OnePlayerTwoSides' ) and 600 or 256+16;
return Def.ActorFrame {
	ToastyAchievedMessageCommand=function(self,params)
		if params.PlayerNumber == Player then
			self:thump(1);
			self:effectclock('beat');
			self:effectmagnitude(1, 1, 1);
			self:effectcolor1(color("1,1.125,1,1"));
			self:effectcolor2(color("1,1,1,1"));
		end
	end;
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(fWidth, SCREEN_HEIGHT);
			self:diffuse(PlayerColor(Player));
			self:diffusealpha(0);
			self:fadeleft(32 / (256 + 16));
			self:faderight(32 / (256 + 16));
		end;
		ToastyAchievedMessageCommand=function(self,params)
			if params.PlayerNumber == Player then
				self:stoptweening();
				self:linear(2.125);
				self:diffuse(Color.Alpha( PlayerColor(Player), 0.345 ));
				self:glow(color("1,1,1,0.5"));
				self:decelerate(3);
				self:glow(Color.Alpha( ColorDarkTone( PlayerColor(Player) ), 0 ));
				self:diffuseramp();
				self:effectcolor1(ColorLightTone( PlayerColor(Player) ));
				self:effectcolor2(PlayerColor(Player));
				self:effectclock('beat');
				self:effectperiod(2);
				HasToasty = true;
			end
		end;
		ToastyDroppedMessageCommand=function(self,params)
			if params.PlayerNumber == Player then
				if HasToasty then
					self:finishtweening();
					self:stopeffect();
					self:glow(color("1,1,1,0.5"));
					self:decelerate(0.35);
					self:diffuse(Color.Alpha( Color("Black"), 0.25 ));
					self:glow(color("1,1,1,0"));
					self:linear(0.35 * 0.25);
					self:diffusealpha(0);
					HasToasty = false;
				else
					return
				end
			end
		end;
	};
};