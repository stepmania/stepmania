local t = Def.ActorFrame {};
-- Header
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(x,SCREEN_LEFT;y,SCREEN_TOP;draworder,100);
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      zoomto,SCREEN_WIDTH,64;
      diffuse,ThemeColor.Secondary);
  };
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      zoomto,SCREEN_WIDTH,48;
      diffuse,ThemeColor.Background);
  };
};
-- Footer
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(x,SCREEN_LEFT;y,SCREEN_BOTTOM;draworder,100);
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      zoomto,SCREEN_WIDTH,64;
      diffuse,ThemeColor.Secondary);
  };
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      zoomto,SCREEN_WIDTH,32;
      diffuse,ThemeColor.Background);
  };
};

return t;