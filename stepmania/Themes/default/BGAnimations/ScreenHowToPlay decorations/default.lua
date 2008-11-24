function ExplanationText(s,delay,length)
	return Def.ActorFrame {
		LoadActor("text frame") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+40;diffusealpha,0;sleep,delay;linear,0.5;diffusealpha,1;sleep,length;linear,0.5;diffusealpha,0;);
		};
		LoadFont("_venacti Bold 24px") .. {
			OnCommand=cmd(x,SCREEN_CENTER_X+160;y,SCREEN_CENTER_Y+40;settext,s;diffusetopedge,color("#FFFFFF");diffusebottomedge,color("#fefb00");wrapwidthpixels,250;shadowlength,0;cropright,1;sleep,delay;linear,0.5;cropright,0;sleep,length;linear,0.5;diffusealpha,0;);
		};
	};
end

function Circle(xpos,ypos,delay,length)
	return Def.ActorFrame {
		OnCommand=cmd(x,xpos;y,ypos;draworder,1;);
		LoadActor("circle") .. {
			OnCommand=cmd(cropleft,0.6;cropright,0.6;croptop,0.5;cropbottom,0.0;sleep,delay+0.0;linear,0.2;cropleft,0;glowshift;sleep,length-0.2;linear,0.5;diffusealpha,0;);
		};
		LoadActor("circle") .. {
			OnCommand=cmd(cropleft,0.0;cropright,1;croptop,0.0;cropbottom,0.5;sleep,delay+0.2;linear,0.4;cropright,0;glowshift;sleep,length-0.6;linear,0.5;diffusealpha,0;);
		};
		LoadActor("circle") .. {
			OnCommand=cmd(cropleft,1;cropright,0.0;croptop,0.5;cropbottom,0.0;sleep,delay+0.6;linear,0.2;cropleft,0.4;glowshift;sleep,length-0.8;linear,0.5;diffusealpha,0;);
		};
	};
end

function Arrow(xpos,ypos,delay,length)
	return LoadActor("arrow") .. {
		OnCommand=cmd(x,xpos;y,ypos;diffusealpha,0;sleep,delay;addx,50;linear,0.5;diffusealpha,1;addx,-50;sleep,length;linear,0.5;diffusealpha,0;draworder,1;);
	};
end

return Def.ActorFrame {
	LoadActor("how to play") .. {
		OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;diffusealpha,0;zoom,6;sleep,0.0;linear,0.3;diffusealpha,1;zoom,2;sleep,1.7;linear,0.3;zoom,1;addx,160;addy,-150;draworder,1;);
	};

	ExplanationText( "When the arrows rise to this point, step on the matching panels.", 5, 9 );
	Circle( SCREEN_CENTER_X-160, SCREEN_CENTER_Y-120, 5, 3 );
	--Step( SCREEN_CENTER_X-160, SCREEN_CENTER_Y-120, 5, 3 );
	ExplanationText( "Step on both panels if two different arrows appear at the same time.", 15, 5 );
	ExplanationText( "If you misstep repeatedly, you life meter will decrease until the game is over.", 21, 4 );
	Arrow( SCREEN_CENTER_X-60, SCREEN_CENTER_Y-175, 21, 2 );
};
