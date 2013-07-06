local t = Def.ActorFrame {};

t[#t+1] = Def.ActorFrame {
	FOV=90;
	InitCommand=function(self)
		self:Center();
	end;
	Def.Quad {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH,SCREEN_HEIGHT);
		end;
		OnCommand=function(self)
			self:diffuse(color("#FFCB05"));
			self:diffusebottomedge(color("#F0BA00"));
		end;
	};
	Def.ActorFrame {
		InitCommand=function(self)
			self:hide_if(hideFancyElements);
		end;
		LoadActor("_checkerboard") .. {
			InitCommand=function(self)
				self:rotationy(0);
				self:rotationz(0);
				self:rotationx(-90 / 4 * 3.5);
				self:zoomto(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2);
				self:customtexturerect(0, 0, SCREEN_WIDTH * 4 / 256, SCREEN_HEIGHT * 4 / 256);
			end;
			OnCommand=function(self)
				self:texcoordvelocity(0, 0.5);
				self:diffuse(color("#ffd400"));
				self:diffusealpha(0.5);
				self:fadetop(1);
			end;
		};
	};
	LoadActor("_particleLoader") .. {
		InitCommand=function(self)
			self:x(-SCREEN_CENTER_X);
			self:y(-SCREEN_CENTER_Y);
			self:hide_if(hideFancyElements);
		end;
	};		
};

return t;
