local t = Def.ActorFrame {};
-- Background Color
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	--
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:diffuse(color("#005185"));
		end;
	};
};

-- Additive Tint
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	--
	Def.Quad {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:fadetop(1);
			self:blend("BlendMode_Add");
			self:diffusealpha(0.2);
		end;
	}
};

-- Textures Frame
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	
	-- Scanline
	LoadActor("_texture scanline") .. {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:customtexturerect(0, 0, SCREEN_WIDTH / 16, SCREEN_HEIGHT / 32);
			self:diffuse(Color.Black);
			self:diffusealpha(0.25);
		end;
	};
	
	-- Checkerboard
	LoadActor("_texture checkerboard") .. {
		InitCommand=function(self)
			self:zoomto(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:customtexturerect(0, 0, SCREEN_WIDTH / 64, SCREEN_HEIGHT / 64);
			self:texcoordvelocity(0.5, 0);
			self:diffuse(Color.Black);
			self:diffusealpha(0.25);
		end;
	};
};

--
return t;