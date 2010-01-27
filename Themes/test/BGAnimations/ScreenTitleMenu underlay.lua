local t = Def.ActorFrame{};
t[#t+1] = LoadActor( THEME:GetPathG("","_theme/logo") )..{
	InitCommand=cmd(Center;diffusealpha,0.2;blend,bmAdd;pulse;effectmagnitude,0.95,1.25,1;effectcolor1,1.5,0.5,1,1;effectcolor2,0.5,1.5,2,1);
};
t[#t+1] = LoadActor( THEME:GetPathG("","_theme/logo") )..{
	InitCommand=cmd(Center);
	--[[
	OnCommand=function(self)
		-- let's test getpixel
		local sxx = "";
		--local pixCol = self:GetTexture():GetPixel(850,70);
		local pixCol = color("0.75,0.9,0,1");
		for i=1,#pixCol do
			sxx = sxx..pixCol[i];
			if i < #pixCol then
				sxx = sxx..",";
			end;
		end;
		Trace( "pixCol(850,70) = "..sxx );
	end;
	--]]
};
--[[
-- old version
t[#t+1] = LoadActor( THEME:GetPathG("","_theme/logo") )..{
	InitCommand=cmd(Center;diffusealpha,0.2;blend,bmAdd;pulse;effectperiod,15;effectmagnitude,1,1.05,1;);
};
--]]

-- returns a table of all the files and directories in the scripts dir
--local files = FILEMAN:GetDirListing( "/Themes/test/Scripts/*" );
-- ActorUtil:GetFileType(path);

return t;