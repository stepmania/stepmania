local t = Def.ActorFrame {};
local bOpen = false;
local function GetTime(self)
	-- Painfully ugly, sorry.
	local c = self:GetChildren();
	local tTime = { Hour = nil, Minute = nil, Second = nil, Append = nil};
	
	if Hour() then tTime.Hour = Hour() else tTime.Hour = 0 end;
	if Minute() then tTime.Minute = Minute() else tTime.Minute = 0 end;
	if Second() then tTime.Second = Second() else tTime.Second = 0 end;
	
	if( Hour() < 12 ) then 
		tTime.Append = "AM" 
	else 
		tTime.Append = "PM" 
	end;
	
	if( Hour() == 0 ) then
		tTime.Hour = 12;
	end;
	
	c.Time:settextf("%02i:%02i:%02i %s",tTime.Hour,tTime.Minute,tTime.Second,tTime.Append);
end;

t[#t+1] = Def.ActorFrame {
	Def.ActorFrame {
		LoadActor(THEME:GetPathB("","_frame 3x3"),"rounded black",96,12) .. {
			Name="Background";
		};
		LoadFont("Common Normal") ..  {
			Text="Test";
			Name="Time";
			InitCommand=function(self)
				self:zoom(0.675);
			end;
		};
		--
		BeginCommand=function(self)
			self:SetUpdateFunction( GetTime );
			self:SetUpdateRate( 1/30 );
		end;
	};
	ToggleConsoleDisplayMessageCommand=function(self)   
		bOpen = not bOpen;
		if bOpen then self:playcommand("Show") else self:playcommand("Hide") end
	end;
	InitCommand=function(self)
		self:x(SCREEN_RIGHT-50);
		self:y(10);
		self:zoomy(0);
	end;
	ShowCommand=function(self)
		self:finishtweening();
		self:zoomx(1);
		self:zoomy(0);
		self:bounceend(0.125);
		self:zoomy(1);
	end;
	HideCommand=function(self)
		self:finishtweening();
		self:zoom(1);
		self:bouncebegin(0.125);
		self:zoomy(0);
	end;
};
return t;