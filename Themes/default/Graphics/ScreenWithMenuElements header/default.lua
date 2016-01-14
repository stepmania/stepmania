local t = Def.ActorFrame {};

local text_width = 0;

local function IsHeaderTextVisible()
	local r = Screen.String("HeaderText");
	return string.len(r) > 0 and true or false
end

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH+1,48;diffuse,color("#161616"));
}
t[#t+1] = LoadActor("_texture stripe") .. {
	Name="TextureStripe";
	InitCommand=cmd(x,-SCREEN_CENTER_X-8;y,-2;horizalign,left;vertalign,top;zoomto,320,48;customtexturerect,0,0,(320/2)/8,50/32);
	OnCommand=cmd(texcoordvelocity,1,0;skewx,-0.0575;diffuse,color("#594300");diffuserightedge,color("#59430000"));
}


t[#t+1] = Def.Quad {
	Name="Underline",
	InitCommand=cmd(x,-SCREEN_CENTER_X+24;y,36;horizalign,left),
	OnCommand=cmd(stoptweening;visible,IsHeaderTextVisible();diffuse,color("#ffd400");linear,0.25;zoomtowidth,text_width),
};

t[#t+1] = LoadFont("Common Bold") .. {
	Name="HeaderText",
	Text=Screen.String("HeaderText"),
	InitCommand=cmd(x,-SCREEN_CENTER_X+24;y,26;zoom,1;horizalign,left;shadowlength,0;maxwidth,200;queuecommand,"Set"),
	OnCommand=cmd(visible,IsHeaderTextVisible();diffuse,color("#ffd400")),
	BeginCommand=function(self)
		text_width = self:GetWidth();
	end,
	UpdateScreenHeaderMessageCommand=function(self,param)
		self:settext(param.Header);
	end;
};
return t
