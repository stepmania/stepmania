return Def.ActorFrame {
	LoadActor( "top" ) .. {
		InitCommand=cmd();
	};
	Def.ActorFrame {
		InitCommand=cmd(x,-284;y,-6;);
		LoadActor( "bar" ) .. {
			InitCommand=cmd(horizalign,left;y,4);
		};
		LoadFont( "_sf sports night ns upright 52" ) .. {
			InitCommand=cmd(x,64;y,-7;horizalign,left;shadowlength,0;settext,ScreenString("HeaderText");skewx,-0.15);
			Name="HeaderText";
		};
		LoadFont( "_venacti 24" ) .. {
			InitCommand=cmd(x,64;y,12;horizalign,left;shadowlength,0;settext,ScreenString("HeaderSubText"););
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
