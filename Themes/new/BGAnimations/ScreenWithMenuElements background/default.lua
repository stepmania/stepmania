local t = Def.ActorFrame {};
-- Background Color
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  --
  Def.Quad {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color("#005185"));
  };
};
-- Additive Tint
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  --
  Def.Quad {
	InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;fadetop,1;blend,"BlendMode_Add";diffusealpha,0.2);
  }
};
-- Textures Frame
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  -- Scanline
  LoadActor("_texture scanline") .. {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;
      customtexturerect,0,0,SCREEN_WIDTH/16,SCREEN_HEIGHT/32;
      diffuse,Color.Black;
      diffusealpha,0.25;
    );
  };
  -- Checkerboard
  LoadActor("_texture checkerboard") .. {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;
      customtexturerect,0,0,SCREEN_WIDTH/64,SCREEN_HEIGHT/64;
      texcoordvelocity,0.5,0;
      diffuse,Color.Black;
      diffusealpha,0.25;
    );
  };
};

--
return t;