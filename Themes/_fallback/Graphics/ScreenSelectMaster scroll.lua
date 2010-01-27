local gc = Var("GameCommand");

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
    Text=gc:GetName() .. "\n" .. gc:GetText();
};

t.GainFocusCommand=cmd(visible,true);
t.LoseFocusCommand=cmd(visible,false);
return t;