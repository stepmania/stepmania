local t = Def.ActorFrame {};

local function IsVisible()
	local r = Screen.String("HeaderText");
	return string.len(r) > 0 and true or false
end

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH+1,50;diffuse,color("#161616"));
}
t[#t+1] = LoadActor("_texture stripe") .. {
	InitCommand=cmd(x,-SCREEN_CENTER_X;y,-2;horizalign,left;vertalign,top;zoomto,320,50;customtexturerect,0,0,(320/1.5)/8,50/32);
	OnCommand=cmd(faderight,0.5;texcoordvelocity,1.5,0;skewx,-0.0375;diffuse,Color("White");diffusealpha,0.125);
};
t[#t+1] = LoadActor("Header") .. {
	InitCommand=cmd(y,1;vertalign,top;zoomtowidth,SCREEN_WIDTH+1;diffuse,color("#ffd400"));
	OnCommand=cmd(croptop,46/60);
};

t[#t+1] = LoadFont("Common Bold") .. {
	Name="HeaderText";
	Text=Screen.String("HeaderText");
	InitCommand=cmd(x,-SCREEN_CENTER_X+24;y,26;zoom,1;horizalign,left;shadowlength,0;maxwidth,200);
	OnCommand=cmd(visible,IsVisible();skewx,-0.125;diffuse,color("#ffd400");shadowlength,2;shadowcolor,BoostColor(color("#ffd40077"),0.25));
	UpdateScreenHeaderMessageCommand=function(self,param)
		self:settext(param.Header);
	end;
};

t[#t+1] = Def.Quad {
	Name="Underline";
	InitCommand=cmd(x,-SCREEN_CENTER_X+24;y,36;horizalign,left);
	OnCommand=cmd(visible,IsVisible();diffuse,color("#ffd400");shadowlength,2;shadowcolor,BoostColor(color("#ffd40077"),0.25);linear,0.25;zoomtowidth,192;faderight,0.5);
};


return t
