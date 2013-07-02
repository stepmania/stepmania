local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:y(48);
	end;
	Def.Quad {
		InitCommand=function(self)
			self:vertalign(bottom);
			self:zoomto(SCREEN_WIDTH - 32, 2);
		end;
		OnCommand=function(self)
			self:diffusealpha(0.5);
		end;
	};
};
t[#t+1] = Def.ActorFrame {
	Name="MenuTimerDecoration";
	InitCommand=function(self)
		self:y(48);
	end;
	LoadFont("Common Normal") .. {
		Text="TIME";
		InitCommand=function(self)
			self:vertalign(bottom);
			self:horizalign(right);
			self:x(SCREEN_CENTER_X - 16);
			self:y,(-4.5);
		end;
		OnCommand=function(self)
			self:zoom(0.5);
		end;
	};
};
t[#t+1] = Def.ActorFrame {
	Name="HeaderTextDecoration";
	InitCommand=function(self)
		self:y(48);
	end;
	LoadFont("Common Normal") .. {
		Text="SELECT";
		InitCommand=function(self)
			self:vertalign(bottom);
			self:horizalign(right);
			self:x(SCREEN_CENTER_X - 16);
			self:y,(-4.5);
		end;
		OnCommand=function(self)
			self:zoom(0.5);
		end;
	};
	LoadFont("Common Normal") .. {
		Text="OPTIONS";
		InitCommand=function(self)
			self:vertalign(bottom);
			self:horizalign(right);
			self:x(SCREEN_CENTER_X - 16);
			self:y,(-4.5);
		end;
		OnCommand=function(self)
		end;
	};
};
return t