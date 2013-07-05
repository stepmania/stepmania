-- how does installed song??? let's find out

local t = Def.ActorFrame{
	LoadFont("Common Normal")..{
		Name="Header";
		InitCommand=function(self)
			self:x(SCREEN_LEFT + 24);
			self:y(SCREEN_TOP + 24);
			self:halign(0);
			self:diffuse(color("#CCCCCC"));
			self:settext(Screen.String("BodyHeader"));
			self:shadowlength(1);
			self:shadowcolor(HSV(40,0,0.6));
			self:diffusetopedge(color("#FFFFFF"));
		end;
		OnCommand=function(self)
			self:queuecommand("Anim");
		end;
		AnimCommand=function(self)
			self:cropright(1);
			self:faderight(1);
			self:addx(96);
			self:decelerate(1);
			self:addx(-96);
			self:skewx(-0.1);
			self:cropright(0);
			self:faderight(0);
		end;
	};
	-- todo: add explantion paragraph here (above the scroller)
};

return t;