local names = {
	{ "chris", "Chris Danford", "Seattle, WA, USA" },
	{ "glenn", "Glenn Maynard", "Boston, MA, USA" },
	{ "steve", "Steve Checkoway", "San Diego, CA, USA" },
};

local t = Def.ActorFrame {};
for i=1,#names do
	local name = names[i];
	t[#t+1] = Def.ActorFrame {
		OnCommand=cmd(addx,SCREEN_WIDTH;sleep,(i-1)*3;decelerate,0.5;addx,-SCREEN_WIDTH;sleep,2;accelerate,0.5;addx,-SCREEN_WIDTH;);
		LoadActor( name[1] ) .. {
			InitCommand = cmd(scaletoclipped,520,342;y,0;);
		};
		LoadActor( "picture frame" );
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(horizalign,left;x,100;y,138;settext,string.upper(name[2] .. "\n" .. name[3]);strokecolor,color("#00000077"););
		};
	};
end

t.BeginCommand=function(self)
	SCREENMAN:GetTopScreen():PostScreenMessage( "SM_BeginFadingOut", (3 * #names) );
end;

return t;
