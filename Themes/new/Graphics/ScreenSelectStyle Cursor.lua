return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(48, 24);
			self:diffuse(PlayerColor(PLAYER_1));
		end;
	};
	LoadFont("Common Normal") .. {
		Text="P1";
	};
};