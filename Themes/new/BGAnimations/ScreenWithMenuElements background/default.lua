local t = Def.ActorFrame {};
--
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
--[[   LoadActor("_wallpaper") .. {
    InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
  };
   LoadActor("EV01439N") .. {
    InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
    OnCommand=cmd(diffusealpha,0.35);
  }; ]]
  LoadActor("_raise G") .. {
    InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffusealpha,0.35);
  };
  LoadActor(THEME:GetPathG("","_textures/paper")) .. {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,500/SCREEN_WIDTH,500/SCREEN_HEIGHT);
    OnCommand=cmd(diffusealpha,0.35);
  };
  LoadActor("_particle-Vert") .. {
    InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y);
  };
  LoadActor("_particle-Horiz") .. {
    InitCommand=cmd(x,-SCREEN_CENTER_X;y,-SCREEN_CENTER_Y);
  };
  LoadActor(THEME:GetPathG("","_textures/paper")) .. {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT;customtexturerect,0,0,500/SCREEN_WIDTH,500/SCREEN_HEIGHT);
    OnCommand=cmd(diffusealpha,0.35;blend,Blend.Add);
  };
};
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  Def.Quad {
    InitCommand=cmd(zoomto,SCREEN_WIDTH,SCREEN_HEIGHT);
    OnCommand=cmd(diffuse,ThemeColor.Primary
      diffusealpha,0.45);
  };
};
--
return t