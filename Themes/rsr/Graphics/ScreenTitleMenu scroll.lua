local gc = Var("GameCommand");

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
    Text=gc:GetText();
};

t.GainFocusCommand=function(self)
	self:diffusealpha(1);
end;
t.LoseFocusCommand=function(self)
	self:diffusealpha(0.5);
end;
return t;