local t = Def.ActorFrame {};
local tInfo = {
	{"EventMode","Stages"},
	{"LifeDifficulty","Life"},
	{"TimingDifficulty","Difficulty"},
};
local fSpacingX = 72;
local function MakeDisplayBar( fZoomX, fZoomY )
	return Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(vertalign,bottom;y,1;zoomto,fZoomX+2,fZoomY+2);
			OnCommand=cmd(diffuse,Color("Black"));
		};
		Def.Quad {
			InitCommand=cmd(vertalign,bottom;zoomto,fZoomX,fZoomY);
			OnCommand=cmd(diffuse,Color("Orange");diffusetopedge,Color("Yellow"));
		};
	};
end
local function MakeIcon( sTarget )
	local t = Def.ActorFrame {
		LoadActor(THEME:GetPathG("MenuTimer","Frame"));
		LoadFont("Common Normal") .. {
			Text=sTarget[2];
			InitCommand=cmd(y,24+2;zoom,0.5;shadowlength,1);
		};
		--
		LoadFont("Common Normal") .. {
			Text="0";
			OnCommand=cmd(settext,
			( PREFSMAN:GetPreference("EventMode") ) and "∞" or PREFSMAN:GetPreference("SongsPerPlay")
			);
			Condition=sTarget[1] == "EventMode";
		};
		Def.ActorFrame {
			-- Life goes up to 1-5
			Def.ActorFrame {
				InitCommand=function(self)
					self:y(12);
				end;
				MakeDisplayBar( 6, 5 ) .. {
					InitCommand=function(self)
						self:x(-16);
						self:visible( GetLifeDifficulty() >= 1 );
					end;
				};
				MakeDisplayBar( 6, 9 ) .. {
					InitCommand=function(self)
						self:x(-8);
						self:visible( GetLifeDifficulty() >= 2 );
					end;
				};
				MakeDisplayBar( 6, 13 ) .. {
					InitCommand=function(self)
						self:x(0);
						self:visible( GetLifeDifficulty() >= 3 );
					end;
				};
				MakeDisplayBar( 6, 16 ) .. {
					InitCommand=function(self)
						self:x(8);
						self:visible( GetLifeDifficulty() >= 4 );
					end;
				};
				MakeDisplayBar( 6, 20 ) .. {
					InitCommand=function(self)
						self:x(16);
						self:visible( GetLifeDifficulty() >= 5 );
					end;
				};
			};
			Condition=sTarget[1] == "LifeDifficulty";
		};
		Def.ActorFrame {
			-- Timing goes up to 1-8
			Def.ActorFrame {
				InitCommand=function(self)
					self:y(12);
				end;
				MakeDisplayBar( 4, 5 ) .. {
					InitCommand=function(self)
						self:x(-20);
						self:visible( GetTimingDifficulty() >= 1 );
					end;
				};
				MakeDisplayBar( 4, 9 ) .. {
					InitCommand=function(self)
						self:x(-215);
						self:visible( GetTimingDifficulty() >= 2 );
					end;
				};
				MakeDisplayBar( 4, 13 ) .. {
					InitCommand=function(self)
						self:x(-10);
						self:visible( GetTimingDifficulty() >= 3 );
					end;
				};
				MakeDisplayBar( 4, 16 ) .. {
					InitCommand=function(self)
						self:x(-5);
						self:visible( GetTimingDifficulty() >= 4 );
					end;
				};
				MakeDisplayBar( 4, 20 ) .. {
					InitCommand=function(self)
						self:x(5);
						self:visible( GetTimingDifficulty() >= 5 );
					end;
				};
				MakeDisplayBar( 4, 20 ) .. {
					function(self)
						self:x(10);
						self:visible( GetTimingDifficulty() >= 6 );
					end;
				};
				MakeDisplayBar( 4, 20 ) .. {
					function(self)
						self:x(15);
						self:visible( GetTimingDifficulty() >= 7 );
					end;
				};
				MakeDisplayBar( 4, 20 ) .. {
					function(self)
						self:x(20);
						self:visible( GetTimingDifficulty() >= 8 );
					end;
				};
			};
			Condition=sTarget[1] == "TimingDifficulty";
		};
	};
	return t
end;

for i=1,#tInfo do
	t[#t+1] = MakeIcon( tInfo[i] ) .. {
		InitCommand=function(self)
			self:x((i - 1) * fSpacingX);
		end;
	};
end

return t

