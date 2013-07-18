return Def.ActorFrame {
  Def.Quad {
	InitCommand=cmd(zoomto,48,24;diffuse,PlayerColor(PLAYER_1));
  };
  LoadFont("Common Normal") .. {
	Text="P1";
  };
};