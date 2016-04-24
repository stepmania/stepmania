local function DateAndTime()
	local c;
	local realMonth = (MonthOfYear()+1);
	local clockFrame = Def.ActorFrame{
		LoadFont("","mentone/_24px")..{
			Name="Date";
			InitCommand=cmd(horizalign,right;zoom,0.475;shadowlength,1;strokecolor,color("0,0,0,0"));
		};
		LoadFont("","mentone/_24px")..{
			Name="Time";
			InitCommand=cmd(horizalign,right;zoom,0.4;y,12;shadowlength,1;strokecolor,color("0,0,0,0"));
		};
	};
	local function UpdateDateTime(self)
		self:GetChild('Date'):settext( string.format("%04i/%02i/%02i",Year(),realMonth,DayOfMonth()) );
		self:GetChild('Time'):settext( string.format("%02i:%02i:%02i", Hour(), Minute(), Second()) );
	end;
	clockFrame.InitCommand=cmd(SetUpdateFunction,UpdateDateTime);
	return clockFrame;
end;

return Def.ActorFrame {
	DateAndTime()..{
		InitCommand=cmd(x,SCREEN_RIGHT-4;y,SCREEN_TOP+8);
	};

	-- system messages
	Def.Quad {
		InitCommand=cmd(diffuse,color("0,0,0,0");zoomto,SCREEN_WIDTH,40;horizalign,left),
		SystemMessageMessageCommand=function(self,params)
			local f = cmd(finishtweening;x,SCREEN_LEFT;y,28;diffusealpha,0;addy,-100;decelerate,0.3;diffusealpha,.7;diffusetopedge,color("0.2,0.2,0.2,0.7");addy,100)
			f(self) -- "f your self"
			self:playcommand("On")
			if params.NoAnimate then
				self:finishtweening()
			end
			f = cmd(sleep,3;decelerate,0.3;addy,-100;diffusealpha,0)
			f(self) -- man these are pretty ugly.
			self:playcommand("Off")
		end,
		HideSystemMessageMessageCommand=cmd(finishtweening)
	};
	Font("mentone","24px")..{
		InitCommand=cmd(strokecolor,color("0,0,0,0.75");maxwidth,750;horizalign,left;vertalign,top;zoom,0.8;shadowlength,2;y,20;diffusealpha,0),
		SystemMessageMessageCommand=function(self,params)
			self:settext(params.Message)
			local f = cmd(finishtweening;x,SCREEN_LEFT+20;y,20;diffusealpha,0;addy,-100;decelerate,0.3;diffusealpha,1;addy,100); f(self)
			self:playcommand("On")
			if params.NoAnimate then
				self:finishtweening()
			end
			f = cmd(sleep,3;decelerate,0.3;addy,-100;diffusealpha,0)
			f(self)
			self:playcommand("Off")
		end,
		HideSystemMessageMessageCommand=cmd(finishtweening)
	};
	LoadActor(THEME:GetPathB("ScreenSystemLayer","aux"));
};