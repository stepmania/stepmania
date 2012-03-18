return Def.ActorFrame {
  -- Base
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      zoomto,SCREEN_WIDTH,48;
      diffuse,ThemeColor.Background;diffusealpha,0.5);
  };
  -- Line
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      y,48;
      zoomto,SCREEN_WIDTH,2;
      diffuse,ThemeColor.Primary;diffusealpha,1;
    );
  };
};