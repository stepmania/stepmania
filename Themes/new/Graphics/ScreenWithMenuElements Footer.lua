return Def.ActorFrame {
  -- Base
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      zoomto,SCREEN_WIDTH,80;
      diffuse,ThemeColor.Secondary);
  };
  -- System Layer
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      zoomto,SCREEN_WIDTH,32;
      diffuse,ThemeColor.Background);
  };
  -- Inner Shadow
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      y,-80;
      zoomto,SCREEN_WIDTH,5;
      diffuse,Color('Black');diffusealpha,0.25;
      fadebottom,1);
  };
  -- Inner Hard Line
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      y,-80;
      zoomto,SCREEN_WIDTH,1;
      diffuse,Color('Black');diffusealpha,0.125;);
  };
  -- Outer White Fade
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      y,-80;
      zoomto,SCREEN_WIDTH,8;
      diffuse,Color('White');diffusealpha,0.125;
      fadetop,1);
  };
  -- Outer White Line
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      y,-80;
      zoomto,SCREEN_WIDTH,1;
      diffuse,Color('White');diffusealpha,0.125;);
  };
};