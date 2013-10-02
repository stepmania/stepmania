local gc = Var("GameCommand");
local colors = {
	Exit = Color.Red;
}

local t = Def.ActorFrame {};

t[#t+1] = LoadFont("Common Normal") .. {
	Text=gc:GetName();
	InitCommand=cmd(horizalign,right;shadowlength,1.5;playcommand,"Set");
	SetCommand=function(self)
		if colors[gc:GetName()] ~= nil then
			self:diffuse(colors[gc:GetName()]);
		end
	end;
	GainFocusCommand=cmd(diffusealpha,1);
	LoseFocusCommand=cmd(diffusealpha,0.5);
};


return t;