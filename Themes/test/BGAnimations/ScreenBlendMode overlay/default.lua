local blendModes = {
	'BlendMode_Normal',
	'BlendMode_Add',
	'BlendMode_Modulate',
	'BlendMode_WeightedMultiply',
	'BlendMode_InvertDest',
};

local t = Def.ActorFrame{};

for i=1,#blendModes do
	--diffuse,color("0.45,0.45,0.45,0.9")
	local thisX = (i > 3) and ((i-4)*72) or ((i-1)*72)
	local thisY = (i > 3) and 72 or 0;
	t[#t+1] = Def.Quad{
		InitCommand=cmd(zoomto,64,64;x,(SCREEN_CENTER_X*0.25)+thisX;y,SCREEN_CENTER_Y+thisY;blend,blendModes[i];diffuse,color("1,1,1,0.4"););
		--OnCommand=cmd(diffuse,color("0,0.7,1,0.25"););
	};
end;

t[#t+1] = LoadFont("Common normal")..{
	InitCommand=cmd(CenterX;y,SCREEN_CENTER_Y*0.75;maxwidth,SCREEN_WIDTH*0.95);
	BeginCommand=function(self)
		local driver = PREFSMAN:GetPreference('VideoRenderers');
		local possibleRenderers = split(",", driver);
		driver = possibleRenderers[1];
		local blends = "";
		for i=1,#blendModes do
			blends = blends..blendModes[i];
			if i < #blendModes then
				blends = blends..", ";
			end;
		end;
		self:settext(driver.."\n"..blends);
	end;
};

t[#t+1] = Def.Quad{
	InitCommand=cmd(zoomto,512,128;x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*1.65;diffuse,color("0,0,0,0.1"););
	OnCommand=cmd(diffuseleftedge,color("0,0,0,0.25");diffuserightedge,color("0,0,0,0.125"););
};

return t;