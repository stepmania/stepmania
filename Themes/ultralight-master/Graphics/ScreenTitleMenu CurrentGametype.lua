local icon = GetGameIcon()

local t = Def.ActorFrame{
	LoadActor( THEME:GetPathG("","_stepstype/"..icon) )..{
		InitCommand=cmd(x,88;y,20;Real);
		OffCommand=cmd(accelerate,0.25;addy,SCREEN_CENTER_Y*1.5;);
	};
	Font("mentone","24px") .. {
		InitCommand=cmd(shadowlength,2;halign,0;valign,0;zoom,0.5;NoStroke;);
		BeginCommand=function(self)
			self:settextf( ScreenString("CurrentGametype"), GAMESTATE:GetCurrentGame():GetName() );
		end;
		OnCommand=cmd(addx,-SCREEN_CENTER_X;queuecommand,"Anim");
		AnimCommand=cmd(decelerate,0.5;addx,SCREEN_CENTER_X);
		OffCommand=cmd(decelerate,0.125;zoomy,180;sleep,0.002;accelerate,0.125;addy,SCREEN_CENTER_Y*1.5;zoomy,0.01;);
	};
};
return t;