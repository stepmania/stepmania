local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
  Def.Quad {
    InitCommand=cmd(Center;zoomto,SCREEN_WIDTH-_safe.w,SCREEN_HEIGHT-_safe.h;queuecommand,"On");
    OnCommand=cmd(basealpha,0.4;diffusealpha,0;visible,false);
    ToggleConsoleDisplayMessageCommand=function(self)   
        self:visible(true);
        self:diffusealpha( 0.4 - self:GetDiffuseAlpha() );
    end;
  };
};

return t;