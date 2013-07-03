local t = Def.ActorFrame{
	Def.Quad{
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, 96);
			self:diffuse(color("#EEF4FFCC"));
		end;
	};
	Def.ActorFrame{
		InitCommand=function(self)
			self:y(12);
		end;
		LoadFont("_frutiger roman 24px")..{
			Name="Header";
			Text=THEME:GetString("ScreenReferenceMain","Header");
			InitCommand=function(self)
				self:x(-312);
				self:y(-34);
				self:halign(0);
				self:diffuse(color("#111111FF"));
				self:diffusetopedge(color("#11111188"));
				self:shadowlength(-1);
				self:shadowcolor(color("#FFFFFF44"));
			end;
		};
		Def.Quad{
			InitCommand=function(self)
				self:zoomto(SCREEN_WIDTH * 0.975, 2);
				self:y(-20);
				self:diffuse(color("#111111FF"));
				self:diffusetopedge(color("#11111188"));
				self:shadowlength(-1);
				self:shadowcolor(color("#FFFFFF44"));
				self:fadeleft(0.1);
				self:faderight(0.625);
			end;
		};
		LoadFont("_frutiger roman 24px")..{
			Name="Explanation";
			Text=THEME:GetString("ScreenReferenceMain","Explanation");
			InitCommand=function(self)
				self:x(-312);
				self:y(-14);
				self:align(0, 0);
				self:diffuse(color("#111111FF"));
				self:zoom(0.65);
				self:wrapwidthpixels(SCREEN_WIDTH);
			end;
		};
	};
};

return t;