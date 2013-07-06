return Def.ActorFrame {
	CurrentSongChangedMessageCommand=function(self)
		local song = GAMESTATE:GetCurrentSong(); 
		if song then
			self:finishtweening();
			self:playcommand("TweenOn");
		elseif not song and self:GetZoomX() == 1 then
			self:finishtweening();
			self:playcommand("TweenOff");
		end;
	end;
	Def.Quad {
		InitCommand=function(self)
			self:y(-14);
			self:zoomto(164, 2);
			self:fadeleft(8 / 164);
			self:faderight(8 / 164);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:diffusealpha(0);
			self:linear(0.35);
			self:diffusealpha(0.5);
		end;
	};
	Def.Quad {
		InitCommand=function(self)
			self:y(24 * 5 - 10);
			self:zoomto(164, 2);
			self:fadeleft(8 / 164);
			self:faderight(8 / 164);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:diffusealpha(0);
			self:linear(0.35);
			self:diffusealpha(0.5);
		end;
	};
	Def.StepsDisplayList {
		Name="StepsDisplayListRow";

		CursorP1 = Def.ActorFrame {
			InitCommand=function(self)
				self:x(-128 + 16);
				self:player(PLAYER_1);
			end;
			PlayerJoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_1 then
					self:visible(true);
					self:zoom(0);
					self:bounceend(1);
					self:zoom(1);
				end;
			end;
			PlayerUnjoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_1 then
					self:visible(true);
					self:bouncebegin(1);
					self:zoom(0);
				end;
			end;
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
				InitCommand=function(self)
					self:diffuse(PlayerColor(PLAYER_1));
					self:x(8);
					self:zoom(0.75);
				end;
			};
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand=function(self)
					self:x(20);
					self:diffuse(PlayerColor(PLAYER_1));
				end;
				OnCommand=function(self)
					self:thump(1);
					self:effectmagnitude(1, 1.25, 1);
					self:effectclock('beat');
				end;
			};
			LoadFont("Common Normal") .. {
				Text="P1";
				InitCommand=function(self)
					self:x(2);
					self:diffuse(PlayerColor(PLAYER_1));
					self:shadowlength(1);
				end;
				OnCommand=function(self)
					self:zoom(0.75);
				end;
			};
		};
		CursorP2 = Def.ActorFrame {
			InitCommand=function(self)
				self:x(128 - 16);
				self:player(PLAYER_2);
			end;
			PlayerJoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_2 then
					self:visible(true);
					self:zoom(0);
					self:bounceend(1);
					self:zoom(1);
				end;
			end;
			PlayerUnjoinedMessageCommand=function(self, params)
				if params.Player == PLAYER_2 then
					self:visible(true);
					self:bouncebegin(1);
					self:zoom(0);
				end;
			end;
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
				InitCommand=function(self)
					self:diffuse(PlayerColor(PLAYER_2));
					self:x(-8);
					self:zoom(0.75);
					self:zoomx(-0.75);
				end;
			};
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand=function(self)
					self:x(-20);
					self:diffuse(PlayerColor(PLAYER_2));
					self:zoomx(-1);
				end;
				OnCommand=function(self)
					self:thump(1);
					self:effectmagnitude(1, 1.25, 1);
					self:effectclock('beat');
				end;
			};
			LoadFont("Common Normal") .. {
				Text="P2";
				InitCommand=function(self)
					self:x(-2);
					self:diffuse(PlayerColor(PLAYER_2));
					self:shadowlength(1);
				end;
				OnCommand=function(self)
					self:zoom(0.75);
				end;
			};
		};
		CursorP1Frame = Def.Actor{
			ChangeCommand=function(self)
				self:stoptweening();
				self:decelerate(0.05);
			end;
		};
		CursorP2Frame = Def.Actor{
			ChangeCommand=function(self)
				self:stoptweening();
				self:decelerate(0.05);
			end;
		};
	};
};