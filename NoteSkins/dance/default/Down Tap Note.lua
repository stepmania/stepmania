local t = Def.ActorFrame {
	Def.Sprite {
		Texture="_arrow";
		Frame0000=0;
		Delay0000=1;
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=function(self)
			self:y(15);
			self:effectclock("beat");
			self:diffuseramp();
			self:effectcolor1(color("1,1,1,0"));
			self:effectcolor2(color("1,1,1,0.35"));
			self:effectoffset(0);
		end;
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=function(self)
			self:y(5);
			self:effectclock("beat");
			self:diffuseramp();
			self:effectcolor1(color("1,1,1,0"));
			self:effectcolor2(color("1,1,1,0.35"));
			self:effectoffset(0.25);
		end;
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=function(self)
			self:y(-5);
			self:effectclock("beat");
			self:diffuseramp();
			self:effectcolor1(color("1,1,1,0"));
			self:effectcolor2(color("1,1,1,0.35"));
			self:effectoffset(0.5);
		end;
	};
	Def.Sprite {
		Texture="_circle";
		Frame0000=0;
		Delay0000=1;
		InitCommand=function(self)
			self:y(-15);
			self:effectclock("beat");
			self:diffuseramp();
			self:effectcolor1(color("1,1,1,0"));
			self:effectcolor2(color("1,1,1,0.35"));
			self:effectoffset(0.75);
		end;
	};
};
return t;
