return Def.ActorFrame {
	LoadActor( "top" ) .. {
		InitCommand=cmd();
	};
	Def.ActorFrame {
		InitCommand=cmd(x,-284;y,-6;);
		LoadActor( "bar" ) .. {
			InitCommand=cmd(horizalign,left;y,4);
			OnCommand=cmd(cropright,1;bounceend,0.32;cropright,0);
			OffCommand=cmd(cropright,0;bouncebegin,0.32;cropright,1);
		};
		LoadFont( "_sf sports night ns upright 26px header text" ) .. {
			InitCommand=cmd(x,64;y,-7;horizalign,left;shadowlength,0;settext,ScreenString("HeaderText");skewx,-0.15;zoomx,1.2;maxwidth,250;);
			OnCommand=cmd(faderight,1;cropright,1;linear,.32;faderight,0;cropright,0);
			OffCommand=cmd(faderight,0;cropright,0;linear,.32;faderight,1;cropright,1);
			Name="HeaderText";
		};
		LoadFont( "_venacti 10px header subtext" ) .. {
			InitCommand=cmd(x,64;y,11;horizalign,left;shadowlength,0;settext,ScreenString("HeaderSubText");zoomx,1.2;);
			OnCommand=cmd(zoomy,.5;faderight,1;cropright,1;linear,.32;faderight,0;cropright,0;zoomy,1);
			OffCommand=cmd(faderight,0;cropright,0;linear,.32;faderight,1;cropright,1);
			Name="HeaderSubText";
		};
		LoadActor( "arrow" ) .. {
		};
		LoadActor( "ring shadow" ) .. {
			InitCommand=cmd(x,-1.5;y,2;spin;effectmagnitude,0,0,10;);
		};
		LoadActor( "ring diffuse" ) .. {
			InitCommand=cmd(x,-1.5;y,-3;spin;effectmagnitude,0,0,20;);
		};
		UpdateScreenHeaderMessageCommand=function(self,params)
			local header = self:GetChild("HeaderText");
			local subHeader = self:GetChild("HeaderSubText");
			header:settext( params.Header );
			subHeader:settext( params.Subheader );
		end
	};
};
