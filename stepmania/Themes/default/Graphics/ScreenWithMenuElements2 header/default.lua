return Def.ActorFrame {
	LoadActor( "top" ) .. {
		InitCommand=cmd();
	};
	Def.ActorFrame {
		InitCommand=cmd(x,-284;y,-6;);
		LoadActor( "bar" ) .. {
			InitCommand=cmd(horizalign,left;y,4);
		};
		LoadFont( "_sf sports night ns upright 26px header text" ) .. {
			InitCommand=cmd(x,64;y,-7;horizalign,left;shadowlength,0;settext,ScreenString("HeaderText");skewx,-0.15;zoomx,1.2;);
			Name="HeaderText";
		};
		LoadFont( "_venacti 10px header subtext" ) .. {
			InitCommand=cmd(x,64;y,11;horizalign,left;shadowlength,0;settext,ScreenString("HeaderSubText");zoomx,1.2;);
			Name="HeaderSubText";
		};
		LoadActor( "arrow" ) .. {
		};
		LoadActor( "ring" ) .. {
			InitCommand=cmd(x,-1.5);
		};
		UpdateScreenHeaderMessageCommand=function(self,params)
			local header = self:GetChild("HeaderText");
			local subHeader = self:GetChild("HeaderSubText");
			header:settext( params.Header );
			subHeader:settext( params.Subheader );
		end
	};
};
