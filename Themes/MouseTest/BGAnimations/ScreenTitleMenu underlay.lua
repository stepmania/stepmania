local t = Def.ActorFrame{};

local mou = Def.ActorFrame{
	LoadFont("common normal")..{
		Name="Coords";
		InitCommand=cmd(align,0,0;x,SCREEN_LEFT+8;y,SCREEN_BOTTOM-48);
	};
};
local function UpdateMouse(self)
	local coords = self:GetChild("Coords")
	local mouseX = INPUTFILTER:GetMouseX()
	local mouseY = INPUTFILTER:GetMouseY()
	local text = "[Mouse] X: ".. mouseX .." Y: ".. mouseY;
	coords:settext(text)
end
mou.InitCommand=cmd(SetUpdateFunction,UpdateMouse);
t[#t+1] = mou;

t[#t+1] = Def.ActorFrame{
	LoadFont("Common normal")..{
		Name="Title";
		Text=THEME:GetString("MouseTest","Title");
		InitCommand=cmd(CenterX;y,SCREEN_TOP+28;diffuse,color("#333333"));
	};
	LoadFont("Common normal")..{
		Name="Instructions";
		Text=THEME:GetString("MouseTest","Instructions");
		InitCommand=cmd(x,SCREEN_LEFT+12;align,0,0;y,SCREEN_TOP+42;diffuse,color("#333333");zoom,16/24;wrapwidthpixels,SCREEN_WIDTH*1.5);
	};
};

return t;