local t = Def.ActorFrame{
	LoadActor("noise")..{
		Name="Noise";
		InitCommand=cmd(Center;texturewrapping,true;SetTextureFiltering,false;zoom,2;diffusealpha,0;);
		OnCommand=cmd(linear,1;diffusealpha,1);
	};
};

local function Update(self)
	local c = self:GetChild("Noise");
	local x = math.random();
	local y = math.random();
	c:customtexturerect(0+x,0+y,2+x,2+y);
end;
t.InitCommand = cmd(SetUpdateFunction,Update);

return t;
