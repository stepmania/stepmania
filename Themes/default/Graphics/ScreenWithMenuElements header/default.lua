local t = Def.ActorFrame {}

-- Base bar diffuse,color("#1C1C1B");diffusebottomedge,color("#333230");
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(vertalign,top),
	OnCommand=function(self)
		self:addy(-104):decelerate(0.5):addy(104)
	end,
	OffCommand=cmd(sleep,0.175;decelerate,0.4;addy,-105),
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,92),
		OnCommand=function(self)
			self:diffuse(ScreenColor(SCREENMAN:GetTopScreen():GetName())):diffusebottomedge(ColorDarkTone(ScreenColor(SCREENMAN:GetTopScreen():GetName()))):diffusealpha(1)
		end
	},
	LoadActor("_shade") .. {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,92),
		OnCommand=function(self)
			self:diffuse(ScreenColor(SCREENMAN:GetTopScreen():GetName())):diffusebottomedge(ColorDarkTone(ScreenColor(SCREENMAN:GetTopScreen():GetName()))):diffusealpha(0.8):faderight(1)
		end
	},
	-- Shadow
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH,4;y,92),
		OnCommand=cmd(diffuse,Color("Black");fadebottom,1;diffusealpha,0.2)
	}
}

-- Diamond
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,-SCREEN_CENTER_X+76;y,SCREEN_TOP+30),
	OnCommand=cmd(addx,-110;sleep,0.3;decelerate,0.7;addx,110),
	OffCommand=cmd(decelerate,0.175;addx,-110),

	-- Diamond BG
	Def.Quad {
		InitCommand=cmd(vertalign,top;zoomto,54,54;rotationz,45),
		OnCommand=function(self)
			self:diffuse(ColorLightTone(ScreenColor(SCREENMAN:GetTopScreen():GetName())))
		end
	},
	-- Symbol selector
	Def.Sprite {
		Name="HeaderDiamondIcon",
		InitCommand=cmd(horizalign,center;y,18;x,-20;diffusealpha,0.8),
		OnCommand=function(self)
			local screen = SCREENMAN:GetTopScreen():GetName()
			if FILEMAN:DoesFileExist("Themes/"..THEME:GetCurThemeName().."/Graphics/ScreenWithMenuElements header/"..screen.." icon.png") then
				self:Load(THEME:GetPathG("","ScreenWithMenuElements header/"..screen.." icon"))
			-- Little workaround so not every other options menu has the "graphic missing" icon.
			elseif string.find(screen, "Options") then
				self:Load(THEME:GetPathG("","ScreenWithMenuElements header/ScreenOptionsService icon"))
			else
				print("iconerror: file does not exist")
				self:Load(THEME:GetPathG("","ScreenWithMenuElements header/Generic icon"))
			end
		end
	}
}

-- Text
t[#t+1] = LoadFont("Common Header") .. {
	Name="HeaderTitle",
	Text=Screen.String("HeaderText"),
	InitCommand=cmd(zoom,1.0;x,-SCREEN_CENTER_X+110;y,49;horizalign,left;diffuse,color("#ffffff");shadowlength,1),
	OnCommand=cmd(diffusealpha,0;sleep,0.5;smooth,0.3;diffusealpha,0.8),
	UpdateScreenHeaderMessageCommand=function(self,param)
		self:settext(param.Header)
	end,
	OffCommand=cmd(smooth,0.175;diffusealpha,0)
}

-- t[#t+1] = LoadFont("Common Condensed") .. {
	-- Name="HeaderSubTitle";
	-- Text=Screen.String("HeaderSubText");
	-- InitCommand=cmd(zoom,0.8;x,-SCREEN_CENTER_X+110;y,70;horizalign,left;diffuse,color("#ffffff");shadowlength,1;skewx,-0.1);
	-- OnCommand=cmd(diffusealpha,0;sleep,0.55;smooth,0.3;diffusealpha,0.75);
	-- UpdateScreenHeaderMessageCommand=function(self,param)
		-- self:settext(param.Header);
	-- end;
	-- OffCommand=cmd(smooth,0.3;diffusealpha,0);
-- };

return t