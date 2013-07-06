local netConnected = IsNetConnected();
local loggedOnSMO = IsNetSMOnline();

local t = Def.ActorFrame{
	Def.Quad {
		InitCommand=function(self)
			self:y(-12);
			self:x(160);
			self:zoomto(320 + 32, 38);
			self:vertalign(top);
			self:diffuse(Color.Black);
			self:diffusealpha(0.5);
		end;
		OnCommand=function(self)
			self:faderight(0.45);
		end;
		BeginCommand=function(self)
			if netConnected then
				self:zoomtoheight( 38 );
			else
				self:zoomtoheight( 24 );
			end
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:uppercase(true);
			self:zoom(0.75);
			self:shadowlength(1);
			self:horizalign(left);
		end;
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
		InitCommand=function(self)
			self:y(16);
			self:horizalign(left);
			self:zoom(0.5875);
			self:shadowlength(1);
			self:diffuse(color("0.72,0.89,1,1"));
		end;
		BeginCommand=function(self)
			self:settext( string.format(Screen.String("Connected to %s"), GetServerName()) );
		end;
	};
end;

return t;