local netConnected = IsNetConnected();
local loggedOnSMO = IsNetSMOnline();

local t = Def.ActorFrame{
	LoadFont("Common Normal") .. {
		InitCommand=cmd(halign,1;zoom,0.5);
		BeginCommand=function(self)
			-- check network status
			if netConnected then
				self:diffuse( color("0.95,0.975,1,1") );
				self:diffusebottomedge( color("0.72,0.89,1,1") );
				self:settext( ScreenString("Network OK") );
			else
				self:diffuse( color("0.75,0.75,0.75,1") );
				self:settext( ScreenString("Offline") );
			end;
		end;
		OffCommand=cmd(bouncebegin,0.125;zoomy,0;zoomx,1024;addx,SCREEN_WIDTH);
	};
};

if netConnected then
	t[#t+1] = LoadFont("Common Normal") .. {
		InitCommand=cmd(y,14;halign,1;zoom,0.5;diffuse,color("0.72,0.89,1,1"));
		BeginCommand=function(self)
			self:settext( string.format(ScreenString("Connected to %s"), GetServerName()) );
		end;
		OffCommand=cmd(bouncebegin,0.125;zoomy,0);
	};
end;

return t;