-- This file returns an ActorFrame that holds a few objects.

-- Since I like to collapse similar code, here is a simple example:
local lineWidth = SCREEN_WIDTH*0.9;
-- previously, I had this set to SCREEN_WIDTH*0.8 in each of the
-- Def.Quad blocks below. Now I just use lineWidth and I get two advantages:
-- 1) I can change it and it takes effect in multiple places.
-- 2) I get a human readable name out of it.
-- It makes your code easier to read and work with.

local lineTime = 0.625;
-- same with this code.

return Def.ActorFrame {
	Def.ActorFrame{
		Name="Flourishes";
		-- it has to be 0.05 since 0.9 uses up space on both sides.
		InitCommand=function(self)
			self:x(SCREEN_LEFT + (SCREEN_WIDTH * 0.05);
			self:y(SCREEN_CENTER_Y);
		end;

		Def.Quad{
			Name="Red";
			InitCommand=function(self)
				self:horizalign(left);
				self:zoomto(0, 4);
				self:diffuse(color("#DA8989"));
				self:diffuserightedge(color("#FFBBBB"));
			end;
			OnCommand=function(self)
				self:sleep(0.1);
				self:linear(lineTime);
				self:zoomx(lineWidth);
			end;
		};

		Def.Quad{
			Name="Yellow";
			InitCommand=function(self)
				self:y(4);
				self:horizalign(left);
				self:zoomto(0, 4);
				self:diffuse(color("#DAD989"));
				self:diffuserightedge(color("#FFFCBB"));
			end;
			OnCommand=function(self)
				self:sleep(0.05);
				self:linear(lineTime);
				self:zoomx(lineWidth);
			end;
		};

		Def.Quad{
			Name="Blue";
			InitCommand=function(self)
				self:y(8);
				self:horizalign(left);
				self:zoomto(0, 4);
				self:diffuse(color("#89C6DA"));
				self:diffuserightedge(color("#BBEEFF"));
			end;
			OnCommand=function(self)
				self:linear(lineTime);
				self:zoomx(lineWidth);
			end;
		};
	};

	-- LoadActor() is used to automatically load objects.
	-- You'll be using it a lot.

	-- THEME:GetPathG() gets a file from the Graphics folder.
	-- What graphic depends on the arguments.
	-- There must always be two, and there usually is a space between the
	-- first and second, unless you perform a trick like this:
	LoadActor( THEME:GetPathG("","_common/_logo") ) .. {
		-- the InitCommand puts the logo 44 pixels above vertical center
		-- and moves it 60 pixels to the left of horizontal center.
		-- diffusealpha,0 makes the object invisible,
		-- which is handy for transitions...
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X * 0.8125);
			self:y(SCREEN_CENTER_Y - 44);
			self:diffusealpha(0);
		end;
		-- ...as we see in the OnCommand. linear means to perform the next
		-- commands over a span of time (in seconds).
		-- diffusealpha,1 makes the logo fade in when applied here.
		OnCommand=function(self)
			self:linear(0.5);
			self:diffusealpha(1);
		end;
	};

	-- Creative Commons logo, using a local file in the same directory:
	LoadActor( "creativecommons" )..{
		InitCommand=function(self)
			self:x(SCREEN_LEFT + 64);
			self:y(SCREEN_BOTTOM - 43);
			self:Real();
		end;
		OnCommand=function(self)
			self:addy(32);
			self:cropbottom(1);
			self:fadebottom(1);
			self:decelerate(0.8);
			self:cropbottom(0);
			self:fadebottom(0);
			self:addy(-32);
		end;
	};
	-- Making a cheap reflection effect for the sake of showing off the fov and
	-- vanishpoint commands on ActorFrames.
	Def.ActorFrame{
		InitCommand=function(self)
			self:fov(45);
			self:vanishpoint(SCREEN_LEFT + 64, SCREEN_BOTTOM - 20);
		end;
		LoadActor( "creativecommons" )..{
			InitCommand=function(self)
				self:x(SCREEN_LEFT + 64);
				self:y(SCREEN_BOTTOM - 20);
				self:valign(0);
				self:zoomy(-0.6);
				self:rotationx(-60);
				self:diffusealpha(0.6);
				self:Real();
			end;
			OnCommand=function(self)
				self:croptop(1);
				self:fadetop(0);
				self:decelerate(0.8);
				self:croptop(0);
				self:fadetop(1);
			end;
		};
	};
};