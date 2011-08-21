-- Alternating files being played back at once
local t = Def.ActorFrame {
    LoadActor(Var "File1") .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;rate,0.25) };
    LoadActor(Var "File2") .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(1,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;rate,0.25) };
    LoadActor(Var "File2") .. { OnCommand=cmd(x,scale(1,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;rate,0.25) };
    LoadActor(Var "File1") .. { OnCommand=cmd(x,scale(3,0,4,SCREEN_LEFT,SCREEN_RIGHT);y,scale(3,0,4,SCREEN_TOP,SCREEN_BOTTOM);cropto,SCREEN_WIDTH/2,SCREEN_HEIGHT/2;rate,0.25) };
};

return t