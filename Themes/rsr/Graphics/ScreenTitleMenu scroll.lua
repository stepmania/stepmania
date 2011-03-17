local gc = Var("GameCommand");

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
    Text=gc:GetText();
};

t.GainFocusCommand=cmd(diffusealpha,1);
t.LoseFocusCommand=cmd(diffusealpha,0.5);
return t;