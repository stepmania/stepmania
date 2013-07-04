local scrolltime = 95;

local t = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	LoadActor("_space")..{
		InitCommand=function(self)
			self:y(-SCREEN_HEIGHT * 1.5);
			self:fadebottom(0.125);
			self:fadetop(0.25);
		end;
		OnCommand=function(self)
			self:linear(scrolltime);
			self:addy(SCREEN_HEIGHT * 1.5825);
		end;
	};
	Def.Quad {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffusecolor(color("#FFCB05"));
			self:diffusebottomedge(color("#F0BA00"));
			self:diffusealpha(0.45);
		end;
	};
	LoadActor("_grid")..{
		InitCommand=function(self)
			self:customtexturerect(0, 0, (SCREEN_WIDTH + 1) / 4, SCREEN_HEIGHT / 4);
			self:SetTextureFiltering(true);
		end;
		OnCommand=function(self)
			self:zoomto(SCREEN_WIDTH + 1, SCREEN_HEIGHT);
			self:diffuse(Color("Black"));
			self:diffuseshift();
			self:effecttiming((1 / 8) * 4, 0, (7 / 8) * 4, 0);
			self:effectclock('beatnooffset');
			self:effectcolor2(Color("Black"));
			self:effectcolor1(Color.Alpha(Color("Black"),0.45));
			self:fadebottom(0.25);
			self:fadetop(0.25);
			self:croptop(48 / 480);
			self:cropbottom(48 / 480);
			self:diffusealpha(0.345);
		end;
	};
};

return t