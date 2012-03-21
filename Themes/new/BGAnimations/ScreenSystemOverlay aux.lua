local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
  Def.Quad {
    InitCommand=cmd(Center;zoomto,SCREEN_WIDTH-_safe.w,SCREEN_HEIGHT-_safe.h);
    ScreenChangedMessageCommand=cmd(playcommand,"On");
    OnCommand=cmd(basealpha,0.4;diffusealpha,0;visible,false;blend,Blend.Add);
    ToggleConsoleDisplayMessageCommand=function(self)   
        self:visible(true);
        self:diffusealpha( 0.4 - self:GetDiffuseAlpha() );
    end;
  };
};

return t;