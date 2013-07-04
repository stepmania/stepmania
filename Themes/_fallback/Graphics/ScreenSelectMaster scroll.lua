local gc = Var("GameCommand");

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
    Text=gc:GetName();
};

t.GainFocusCommand=function(self)
	self:visible(true);
end;
t.LoseFocusCommand=function(self)
	self:visible(false);
end;
return t;