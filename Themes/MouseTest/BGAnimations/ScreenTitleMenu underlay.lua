local t = Def.ActorFrame{};

local mou = Def.ActorFrame{
	LoadFont("common normal")..{
		Name="Coords";
		InitCommand=function(self)
			self:align(0, 0);
			self:x(SCREEN_LEFT + 8);
			self:y(SCREEN_BOTTOM - 48);
		end;
	};
};
local function UpdateMouse(self)
	local coords = self:GetChild("Coords")
	local mouseX = INPUTFILTER:GetMouseX()
	local mouseY = INPUTFILTER:GetMouseY()
	local text = "[Mouse] X: ".. mouseX .." Y: ".. mouseY;
	coords:settext(text)
end
mou.InitCommand=function(self)
	self:SetUpdateFunction();
	UpdateMouse();
end;
t[#t+1] = mou;

t[#t+1] = Def.ActorFrame{
	LoadFont("Common normal")..{
		Name="Title";
		Text=THEME:GetString("MouseTest","Title");
		InitCommand=function(self)
			self:CenterX();
			self:y(SCREEN_TOP + 28);
			self:diffuse(color("#333333"));
		end;
	};
	LoadFont("Common normal")..{
		Name="Instructions";
		Text=THEME:GetString("MouseTest","Instructions");
		InitCommand=function(self)
			self:x(SCREEN_LEFT + 12);
			self:align(0, 0);
			self:y(SCREEN_TOP + 42);
			self:diffuse(color("#333333"));
			self:zoom(2 / 3);
			self:wrapwidthpixels(SCREEN_WIDTH * 1.5);
		end;
	};
};

return t;