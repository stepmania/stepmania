return Def.Quad {    
    InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scaletoclipped,SCREEN_WIDTH*2,SCREEN_HEIGHT*2);
    GainFocusCommand=cmd(finishtweening;diffusealpha,1;accelerate,0.6;diffusealpha,0);
};