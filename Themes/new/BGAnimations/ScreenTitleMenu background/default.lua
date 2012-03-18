local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  Def.Quad {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT);
    OnCommand=cmd(diffuse,ThemeColor.Background);
  };
};
--
return t