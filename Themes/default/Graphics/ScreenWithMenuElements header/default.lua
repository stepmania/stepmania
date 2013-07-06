local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=function(self)
		self:vertalign(top);
		self:zoomto(SCREEN_WIDTH+1,50);
		self:diffuse(Color.Black);
	end;
}

t[#t+1] = LoadFont("Common Bold") .. {
	Name="HeaderText";
	Text=Screen.String("HeaderText");
	InitCommand=function(self)
		self:x(-SCREEN_CENTER_X+24);
		self:y(24);
		self:zoom(1);
		self:horizalign(left);
		self:shadowlength(0);
		self:maxwidth(200);
	end;
	OnCommand=function(self)
		self:strokecolor(Color.Invisible);
		self:diffusebottomedge(color("0.75,0.75,0.75"));
	end;
	UpdateScreenHeaderMessageCommand=function(self,param)
		self:settext(param.Header);
	end;
};


return t