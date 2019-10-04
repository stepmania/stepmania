local function arrow_bounce(self, x, y)
	self:bounce()
	self:effectmagnitude(x,y,0)
	self:effecttiming(0,0,0.75,0.25)
	self:effectclock('bgm')
end

return Def.ActorFrame {
	CurrentSongChangedMessageCommand=function(self)
		local song = GAMESTATE:GetCurrentSong();
		if song then
-- 			self:setaux(0);
			self:finishtweening();
			self:decelerate(0.3):zoomx(1):diffusealpha(1)
		elseif not song and self:GetZoomX() == 1 then
-- 			self:setaux(1);
			self:finishtweening();
			self:decelerate(0.3):zoomx(0):diffusealpha(0)
		end;
	end;
	Def.StepsDisplayList {
		Name="StepsDisplayListRow";
		OnCommand=function(self)
		self:diffusealpha(0):zoomx(0):decelerate(0.4):zoomx(1):diffusealpha(1)
		end;
		OffCommand=function(self)
		self:decelerate(0.3):zoomx(0):diffusealpha(0)
		end;
		CursorP1 = Def.ActorFrame {
			InitCommand=function(self)
				self:x(-174)
				self:player(PLAYER_1)
				arrow_bounce(self, 5, 0)
			end,
			PlayerJoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_1 then
					self:visible(true);
					(cmd(zoom,0;bounceend,1;zoom,1))(self);
				end;
			end;
			PlayerUnjoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_1 then
					self:visible(true);
					(cmd(bouncebegin,1;zoom,0))(self);
				end;
			end;
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
				InitCommand=cmd(diffuse,ColorLightTone(PlayerColor(PLAYER_1));x,8;zoom,0.75);
			};
			LoadFont("_roboto condensed Bold 48px") .. {
				Text="P1";
				InitCommand=cmd(horizalign,center;x,8;diffuse,ColorDarkTone(PlayerColor(PLAYER_1)));
				OnCommand=cmd(zoom,0.5);
			};
		};
		CursorP2 = Def.ActorFrame {
			InitCommand=function(self)
				self:x(174)
				self:player(PLAYER_2)
				arrow_bounce(self, -5, 0)
			end,
			PlayerJoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_2 then
					self:visible(true);
					(cmd(zoom,0;bounceend,1;zoom,1))(self);
				end;
			end;
			PlayerUnjoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_2 then
					self:visible(true);
					(cmd(bouncebegin,1;zoom,0))(self);
				end;
			end;
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
				InitCommand=cmd(diffuse,ColorLightTone(PlayerColor(PLAYER_2));x,-8;zoom,0.75;zoomx,-0.75);
			};
			LoadFont("_roboto condensed Bold 48px") .. {
				Text="P2";
				InitCommand=cmd(horizalign,center;x,-8;diffuse,ColorDarkTone(PlayerColor(PLAYER_2)));
				OnCommand=cmd(zoom,0.5);
			};
		};
		CursorP1Frame = Def.Actor{
			ChangeCommand=cmd(stoptweening;decelerate,0.05);
		};
		CursorP2Frame = Def.Actor{
			ChangeCommand=cmd(stoptweening;decelerate,0.05);
		};
	};
};