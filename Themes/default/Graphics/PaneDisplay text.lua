local iPN = ...;
assert(iPN,"[Graphics/PaneDisplay text.lua] No PlayerNumber Provided.");

local t = Def.ActorFrame {};
local function GetRadarData( pnPlayer, rcRadarCategory )
	local tRadarValues;
	local StepsOrTrail;
	local fDesiredValue = 0;
	if GAMESTATE:GetCurrentSteps( pnPlayer ) then
		StepsOrTrail = GAMESTATE:GetCurrentSteps( pnPlayer );
		fDesiredValue = StepsOrTrail:GetRadarValues( pnPlayer ):GetValue( rcRadarCategory );
	elseif GAMESTATE:GetCurrentTrail( pnPlayer ) then
		StepsOrTrail = GAMESTATE:GetCurrentTrail( pnPlayer );
		fDesiredValue = StepsOrTrail:GetRadarValues( pnPlayer ):GetValue( rcRadarCategory );
	else
		StepsOrTrail = nil;
	end;
	return fDesiredValue;
end;

local function CreatePaneDisplayItem( _pnPlayer, _sLabel, _rcRadarCategory )
	return Def.ActorFrame {
		LoadFont("Common SemiBold") .. {
			Text=string.upper( THEME:GetString("PaneDisplay",_sLabel) );
			InitCommand=function(self)
				self:horizalign(left);
			end;
			OnCommand=function(self)
				self:zoom(0.5875);
				self:diffuse(color("0.9,0.9,0.9"));
				self:shadowlength(1);
			end;
		};
		LoadFont("Common Normal") .. {
			Text=string.format("%04i", 0);
			InitCommand=function(self)
				self:x(96);
				self:horizalign(right);
			end;
			OnCommand=function(self)
				self:zoom(0.5875);
				self:shadowlength(1);
			end;
			CurrentSongChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentCourseChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			SetCommand=function(self)
				local song = GAMESTATE:GetCurrentSong()
				local course = GAMESTATE:GetCurrentCourse()
				if not song and not course then
					self:settextf("%04i", 0);
				else
					self:settextf("%04i", GetRadarData( _pnPlayer, _rcRadarCategory ) );
				end
			end;
		};
	};
end;

local function CreatePaneDisplayGraph( _pnPlayer, _sLabel, _rcRadarCategory )
	return Def.ActorFrame {
		LoadFont("Common Normal") .. {
			Text=_sLabel;
			InitCommand=function(self)
				self:horizalign(left);
			end;
			OnCommand=function(self)
				self:zoom(0.5);
				self:shadowlength(1);
			end;
		};
		Def.Quad { 
			InitCommand=function(self)
				self:x(12);
				self:zoomto(50, 10);
				self:horizalign(left);
			end;
			OnCommand=function(self)
				self:diffuse(Color("Black"));
				self:shadowlength(1);
				self:diffusealpha(0.5);
			end;
		};
		Def.Quad {
			InitCommand=function(self)
				self:x(12);
				self:zoomto(50, 10);
				self:horizalign(left);
			end;
			OnCommand=function(self)
				self:shadowlength(0);
				self:diffuse(Color("Green"));
				self:diffusebottomedge(ColorLightTone(Color("Green")));
			end;
			CurrentSongChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentCourseChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			SetCommand=function(self)
				local song = GAMESTATE:GetCurrentSong()
				local course = GAMESTATE:GetCurrentCourse()
				if not song and not course then
					self:stoptweening();
					self:decelerate(0.2);
					self:zoomtowidth(0);
				else
					self:stoptweening();
					self:decelerate(0.2);
					self:zoomtowidth( clamp(GetRadarData( _pnPlayer, _rcRadarCategory ) * 50,0,50) );
				end
			end;
		};
		LoadFont("Common Normal") .. {
			InitCommand=function(self)
				self:x(14);
				self:zoom(0.5);
				self:halign(0);
			end;
			OnCommand=function(self)
				self:shadowlength(1);
				self:strokecolor(color("0.15,0.15,0.15,0.625"));
			end;
			CurrentSongChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentStepsP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP1ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentTrailP2ChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			CurrentCourseChangedMessageCommand=function(self)
				self:playcommand("Set");
			end;
			SetCommand=function(self)
				local song = GAMESTATE:GetCurrentSong()
				local course = GAMESTATE:GetCurrentCourse()
				if not song and not course then
					self:settext("")
				else
					self:settextf("%i%%", GetRadarData( _pnPlayer, _rcRadarCategory ) * 100 );
				end
			end;
		};
	};
end;

--[[ Numbers ]]
t[#t+1] = Def.ActorFrame {
	-- Left 
	CreatePaneDisplayItem( iPN, "Taps", 'RadarCategory_TapsAndHolds' ) .. {
		InitCommand=function(self)
			self:x(-128 + 16 + 8);
			self:y(-14 + 16 * 0);
		end;
	};
	CreatePaneDisplayItem( iPN, "Jumps", 'RadarCategory_Jumps' ) .. {
		InitCommand=function(self)
			self:x(-128 + 16 + 8);
			self:y(-14 + 16 * 1);
		end;
	};
	CreatePaneDisplayItem( iPN, "Holds", 'RadarCategory_Holds' ) .. {
		InitCommand=function(self)
			self:x(-128 + 16 + 8);
			self:y(-14 + 16 * 2);
		end;
	};
	CreatePaneDisplayItem( iPN, "Mines", 'RadarCategory_Mines' ) .. {
		InitCommand=function(self)
			self:x(-128 + 16 + 8);
			self:y(-14 + 16 * 3);
		end;
	};
	-- Center
	CreatePaneDisplayItem( iPN, "Hands", 'RadarCategory_Hands' ) .. {
		InitCommand=function(self)
			self:x(8);
			self:y(-14 + 16 * 0);
		end;
	};
	CreatePaneDisplayItem( iPN, "Rolls", 'RadarCategory_Rolls' ) .. {
		InitCommand=function(self)
			self:x(8);
			self:y(-14 + 16 * 1);
		end;
	};
	CreatePaneDisplayItem( iPN, "Lifts", 'RadarCategory_Lifts' ) .. {
		InitCommand=function(self)
			self:x(8);
			self:y(-14 + 16 * 2);
		end;
	};
	CreatePaneDisplayItem( iPN, "Fakes", 'RadarCategory_Fakes' ) .. {
		InitCommand=function(self)
			self:x(8);
			self:y(-14 + 16 * 3);
		end;
	};
};
return t;