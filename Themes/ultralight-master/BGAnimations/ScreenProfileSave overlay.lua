local x = Def.ActorFrame{
	Def.Quad{
		Name="TopLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-40;zoomto,SCREEN_WIDTH,2;diffuserightedge,HSV(192,1,0.8);cropleft,1);
		OnCommand=cmd(decelerate,0.75;cropleft,0);
		OffCommand=cmd(bouncebegin,0.5;cropright,1);
	};
	Def.Quad{
		Name="BottomLine";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+40;zoomto,SCREEN_WIDTH,2;diffuseleftedge,HSV(192,1,0.8);cropright,1);
		OnCommand=cmd(decelerate,0.75;cropright,0);
		OffCommand=cmd(bouncebegin,0.5;cropleft,1);
	};
	Font("mentone","24px")..{
		Text=ScreenString("SaveProfiles");
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffuse,color("1,1,1,1");shadowlength,1;strokecolor,color("0,0,0,0"));
		OffCommand=cmd(linear,0.15;diffusealpha,0);
	};
};

x[#x+1] = Def.Actor {
	BeginCommand=function(self)
		if SCREENMAN:GetTopScreen():HaveProfileToSave() then self:sleep(1); end;
		self:queuecommand("Load");
	end;
	LoadCommand=function() SCREENMAN:GetTopScreen():Continue(); end;
};

return x;