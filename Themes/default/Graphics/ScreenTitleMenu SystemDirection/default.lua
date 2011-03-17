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
				InitCommand=cmd(y,12);
				MakeDisplayBar( 6, 5 ) .. {
					InitCommand=cmd(x,-16;visible,( GetLifeDifficulty() >= 1 ));
				};
				MakeDisplayBar( 6, 9 ) .. {
					InitCommand=cmd(x,-8;visible,( GetLifeDifficulty() >= 2 ));
				};
				MakeDisplayBar( 6, 13 ) .. {
					InitCommand=cmd(x,0;visible,( GetLifeDifficulty() >= 3 ));
				};
				MakeDisplayBar( 6, 16 ) .. {
					InitCommand=cmd(x,8;visible,( GetLifeDifficulty() >= 4 ));
				};
				MakeDisplayBar( 6, 20 ) .. {
					InitCommand=cmd(x,16;visible,( GetLifeDifficulty() >= 5 ));
				};
			};
			Condition=sTarget[1] == "LifeDifficulty";
		};
		Def.ActorFrame {
			-- Timing goes up to 1-8
			Def.ActorFrame {
				InitCommand=cmd(y,12);
				MakeDisplayBar( 4, 5 ) .. {
					InitCommand=cmd(x,-20;visible,( GetTimingDifficulty() >= 1 ));
				};
				MakeDisplayBar( 4, 9 ) .. {
					InitCommand=cmd(x,-15;visible,( GetTimingDifficulty() >= 2 ));
				};
				MakeDisplayBar( 4, 13 ) .. {
					InitCommand=cmd(x,-10;visible,( GetTimingDifficulty() >= 3 ));
				};
				MakeDisplayBar( 4, 16 ) .. {
					InitCommand=cmd(x,-5;visible,( GetTimingDifficulty() >= 4 ));
				};
				MakeDisplayBar( 4, 20 ) .. {
					InitCommand=cmd(x,5;visible,( GetTimingDifficulty() >= 5 ));
				};
				MakeDisplayBar( 4, 20 ) .. {
					InitCommand=cmd(x,10;visible,( GetTimingDifficulty() >= 6 ));
				};
				MakeDisplayBar( 4, 20 ) .. {
					InitCommand=cmd(x,15;visible,( GetTimingDifficulty() >= 7 ));
				};
				MakeDisplayBar( 4, 20 ) .. {
					InitCommand=cmd(x,20;visible,( GetTimingDifficulty() >= 8 ));
				};
			};
			Condition=sTarget[1] == "TimingDifficulty";
		};
		--
--[[ 		for i=1,8 do
			t[#t+1] = Def.Quad {
				InitCommand=cmd(vertalign,bottom;zoomto,4,10+(i*4));
			};
		end --]]
	};
	return t
end;

for i=1,#tInfo do
	t[#t+1] = MakeIcon( tInfo[i] ) .. {
		InitCommand=cmd(x,(i-1)*fSpacingX);
	};
end

return t

