return Def.ActorFrame {
  -- Base
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      zoomto,SCREEN_WIDTH,80;
      diffuse,ThemeColor.Background;diffusealpha,0.5);
  };
  -- Inner Shadow
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      y,-80;
      zoomto,SCREEN_WIDTH,2;
      diffuse,ThemeColor.Primary;diffusealpha,1;);
  };
};