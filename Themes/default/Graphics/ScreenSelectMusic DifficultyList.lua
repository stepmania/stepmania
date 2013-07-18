return Def.ActorFrame {
	CurrentSongChangedMessageCommand=function(self)
		local song = GAMESTATE:GetCurrentSong(); 
		if song then
-- 			self:setaux(0);
			self:finishtweening();
			self:playcommand("TweenOn");
		elseif not song and self:GetZoomX() == 1 then
-- 			self:setaux(1);
			self:finishtweening();
			self:playcommand("TweenOff");
		end;
	end;
	Def.Quad {
		InitCommand=cmd(y,-14;zoomto,164,2;fadeleft,8/164;faderight,8/164);
		OnCommand=cmd(diffuse,Color("Black");diffusealpha,0;linear,0.35;diffusealpha,0.5);
	};
	Def.Quad {
		InitCommand=cmd(y,24*(5)-10;zoomto,164,2;fadeleft,8/164;faderight,8/164);
		OnCommand=cmd(diffuse,Color("Black");diffusealpha,0;linear,0.35;diffusealpha,0.5);
	};
	Def.StepsDisplayList {
		Name="StepsDisplayListRow";

		CursorP1 = Def.ActorFrame {
			InitCommand=cmd(x,-128+16;player,PLAYER_1);
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
				InitCommand=cmd(diffuse,PlayerColor(PLAYER_1);x,8;zoom,0.75);
			};
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand=cmd(x,20;diffuse,PlayerColor(PLAYER_1));
				OnCommand=cmd(thump,1;effectmagnitude,1,1.25,1;effectclock,'beat';);
			};
			LoadFont("Common Normal") .. {
				Text="P1";
				InitCommand=cmd(x,2;diffuse,PlayerColor(PLAYER_1);shadowlength,1);
				OnCommand=cmd(zoom,0.75);
			};
		};
		CursorP2 = Def.ActorFrame {
			InitCommand=cmd(x,128-16;player,PLAYER_2);
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
				InitCommand=cmd(diffuse,PlayerColor(PLAYER_2);x,-8;zoom,0.75;zoomx,-0.75;);
			};
			LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
				InitCommand=cmd(x,-20;diffuse,PlayerColor(PLAYER_2);zoomx,-1);
				OnCommand=cmd(thump,1;effectmagnitude,1,1.25,1;effectclock,'beat';);
			};
			LoadFont("Common Normal") .. {
				Text="P2";
				InitCommand=cmd(x,-2;diffuse,PlayerColor(PLAYER_2);shadowlength,1);
				OnCommand=cmd(zoom,0.75);
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