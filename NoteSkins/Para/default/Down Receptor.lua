return Def.ActorFrame {
	-- We want this under the noteskin, so that we it looks like a laser (?)
	LoadActor( NOTESKIN:GetPath("", "_Tap Receptor"), NOTESKIN:LoadActor( Var "Button", "KeypressBlock" ) ) .. {
		InitCommand=function(self)
			self:vertalign(top);
			self:zoomx(0);
		end;
		-- Press/Lift allows this to appear and disappear
		PressCommand=function(self)
			self:zoomx(0);
			self:linear(0.02);
			self:zoomx(1);
		end;
		LiftCommand=function(self)
			self:zoomx(1);
			self:linear(0.14);
			self:zoomx(0);
		end;
	};
	-- Overlay the receptor.
	LoadActor( NOTESKIN:GetPath("", "_Tap Receptor"), NOTESKIN:LoadActor( Var "Button", "Go Receptor" ) );	
};

