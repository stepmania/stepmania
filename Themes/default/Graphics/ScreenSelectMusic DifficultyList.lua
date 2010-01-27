return Def.StepsDisplayList {
	Name="StepsDisplayListRow";
	CurrentSongChangedMessageCommand=function(self)
		local song = GAMESTATE:GetCurrentSong();
		if song and self:GetZoomX() == 0 then
			self:playcommand("Show");
		elseif not song and self:GetZoomX() == 1 then
			self:playcommand("Hide");
		end;
	end;

	CursorP1 = Def.ActorFrame {
		InitCommand=cmd(x,-128+16;player,PLAYER_1);
		PlayerJoinedMessageCommand=function(self, params)
			if params.Player == PLAYER_1 then
				self:visible(true);
				(cmd(zoom,0;bounceend,0.3;zoom,1))(self);
			end;
	 	end;
	 	PlayerUnjoinedMessageCommand=function(self, params)
			if params.Player == PLAYER_1 then
				self:visible(true);
				(cmd(bouncebegin,0.3;zoom,0))(self);
			end;
		end;
		LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
			InitCommand=cmd(diffuse,PlayerColor(PLAYER_1));
		};
		LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
			InitCommand=cmd(x,20;diffuse,PlayerColor(PLAYER_1));
			OnCommand=cmd(thump,1;effectmagnitude,1,1.25,1;effectclock,'beat';);
		};
		LoadFont("Common Normal") .. {
			Text="P1";
			InitCommand=cmd(x,-4;diffuse,PlayerColor(PLAYER_1));
		};
	};
	CursorP2 = Def.ActorFrame {
		InitCommand=cmd(x,128-16;player,PLAYER_2);
		PlayerJoinedMessageCommand=function(self, params)
			if params.Player == PLAYER_2 then
				self:visible(true);
				(cmd(zoom,0;bounceend,0.3;zoom,1))(self);
			end;
		end;
		PlayerUnjoinedMessageCommand=function(self, params)
			if params.Player == PLAYER_2 then
				self:visible(true);
				(cmd(bouncebegin,0.3;zoom,0))(self);
			end;
		end;
		LoadActor(THEME:GetPathG("_StepsDisplayListRow","Cursor")) .. {
			InitCommand=cmd(diffuse,PlayerColor(PLAYER_2);zoomx,-1);
		};
		LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
			InitCommand=cmd(x,-20;diffuse,PlayerColor(PLAYER_2);zoomx,-1);
			OnCommand=cmd(thump,1;effectmagnitude,1,1.25,1;effectclock,'beat';);
		};
		LoadFont("Common Normal") .. {
			Text="P2";
			InitCommand=cmd(x,8;diffuse,PlayerColor(PLAYER_2));
		};
	};
	CursorP1Frame = Def.Actor{
		ChangeCommand=cmd(stoptweening;decelerate,0.125);
	};
	CursorP2Frame = Def.Actor{
		ChangeCommand=cmd(stoptweening;decelerate,0.125);
	};
};