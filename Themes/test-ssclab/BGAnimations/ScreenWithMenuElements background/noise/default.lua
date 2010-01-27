local t = Def.ActorFrame {
	LoadActor("noise") .. {
		Name="Noise";
		InitCommand=cmd(texturewrapping,true;SetTextureFiltering,false;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;zoom,2);
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
