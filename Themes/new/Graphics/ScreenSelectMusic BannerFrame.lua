return Def.ActorFrame {
  Def.Quad {
    InitCommand=cmd(zoomto,320+16,80+16);
    OnCommand=cmd(diffuse,ThemeColor.Secondary);
  };
  Def.Quad {
    InitCommand=cmd(zoomto,320+16,24;y,-48-12);
    OnCommand=cmd(diffuse,ThemeColor.Primary;diffuserightedge,ColorMidTone(ThemeColor.Primary));
  };
};