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
		InitCommand=cmd(x,SCREEN_LEFT+(SCREEN_WIDTH*0.05);y,SCREEN_CENTER_Y;);

		Def.Quad{
			Name="Red";
			InitCommand=cmd(horizalign,left;zoomto,0,4;diffuse,color("#DA8989");diffuserightedge,color("#FFBBBB"));
			OnCommand=cmd(sleep,0.1;linear,lineTime;zoomx,lineWidth);
		};

		Def.Quad{
			Name="Yellow";
			InitCommand=cmd(y,4;horizalign,left;zoomto,0,4;diffuse,color("#DAD989");diffuserightedge,color("#FFFCBB"));
			OnCommand=cmd(sleep,0.05;linear,lineTime;zoomx,lineWidth);
		};

		Def.Quad{
			Name="Blue";
			InitCommand=cmd(y,8;horizalign,left;zoomto,0,4;diffuse,color("#89C6DA");diffuserightedge,color("#BBEEFF"));
			OnCommand=cmd(linear,lineTime;zoomx,lineWidth);
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
		InitCommand=cmd(x,SCREEN_CENTER_X*0.8125;y,SCREEN_CENTER_Y-44;diffusealpha,0;);
		-- ...as we see in the OnCommand. linear means to perform the next
		-- commands over a span of time (in seconds).
		-- diffusealpha,1 makes the logo fade in when applied here.
		OnCommand=cmd(linear,0.5;diffusealpha,1;);
	};

	-- Creative Commons logo, using a local file in the same directory:
	LoadActor( "creativecommons" )..{
		InitCommand=cmd(x,SCREEN_LEFT+64;y,SCREEN_BOTTOM-43;Real);
		OnCommand=cmd(addy,32;cropbottom,1;fadebottom,1;decelerate,0.8;cropbottom,0;fadebottom,0;addy,-32);
	};
	-- Making a cheap reflection effect for the sake of showing off the fov and
	-- vanishpoint commands on ActorFrames.
	Def.ActorFrame{
		InitCommand=cmd(fov,45;vanishpoint,SCREEN_LEFT+64,SCREEN_BOTTOM-20;);
		LoadActor( "creativecommons" )..{
			InitCommand=cmd(x,SCREEN_LEFT+64;y,SCREEN_BOTTOM-20;valign,0;zoomy,-0.6;rotationx,-60;diffusealpha,0.6;Real;);
			OnCommand=cmd(croptop,1;fadetop,0;decelerate,0.8;croptop,0;fadetop,1;);
		};
	};
};