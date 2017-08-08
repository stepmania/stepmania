local netConnected = IsNetConnected();
local loggedOnSMO = IsNetSMOnline();

local t = Def.ActorFrame{
	Def.Quad {
		InitCommand=cmd(y,-12;x,160;zoomto,320+32,38;vertalign,top;diffuse,Color.Black;diffusealpha,0.5);
		OnCommand=cmd(faderight,0.45);
		BeginCommand=function(self)
			if netConnected then
				self:zoomtoheight( 38 );
			else
				self:zoomtoheight( 24 );
			end
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=cmd(uppercase,true;zoom,0.75;shadowlength,1;horizalign,left);
		BeginCommand=function(self)
			-- check network status
			if netConnected then
				self:diffuse( color("0.95,0.975,1,1") );
				self:diffusebottomedge( color("0.72,0.89,1,1") );
				self:settext( Screen.String("Network OK") );
			else
				self:diffuse( color("1,1,1,1") );
				self:settext( Screen.String("Offline") );
			end;
		end;
	};
};

if netConnected then
	t[#t+1] = LoadFont("Common Normal") .. {
		InitCommand=cmd(y,16;horizalign,left;zoom,0.5875;shadowlength,1;diffuse,color("0.72,0.89,1,1"));
		BeginCommand=function(self)
			self:settext( string.format(Screen.String("Connected to %s"), GetServerName()) );
		end;
	};
end;

return t;