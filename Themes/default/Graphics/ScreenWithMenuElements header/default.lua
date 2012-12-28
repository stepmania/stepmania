local t = Def.ActorFrame {};

t[#t+1] = Def.Quad {
	InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH+1,48;diffuse,Color.Black);
}
--[[ t[#t+1] = LoadActor("Header") .. {
	InitCommand=cmd(vertalign,top;zoomtowidth,SCREEN_WIDTH+1;diffuse,color("#ffd400"));
}; ]]
--[[ t[#t+1] = LoadActor("_texture stripe") .. {
	InitCommand=cmd(vertalign,top;zoomto,SCREEN_WIDTH+64,44;customtexturerect,0,0,SCREEN_WIDTH+64/8,44/32);
	OnCommand=cmd(fadebottom,0.8;texcoordvelocity,1,0;skewx,-0.0025;diffuse,Color("Black");diffusealpha,0.235);
}; --]]
t[#t+1] = LoadFont("Common Bold") .. {
	Name="HeaderText";
	Text=Screen.String("HeaderText");
	InitCommand=cmd(x,-SCREEN_CENTER_X+24;y,24;zoom,1;horizalign,left;shadowlength,0;maxwidth,200);
	OnCommand=cmd(strokecolor,Color.Invisible;diffusebottomedge,color("0.75,0.75,0.75"));
	UpdateScreenHeaderMessageCommand=function(self,param)
		self:settext(param.Header);
	end;
};


return t