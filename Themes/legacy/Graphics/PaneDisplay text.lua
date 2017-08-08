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
			InitCommand=cmd(horizalign,left);
			OnCommand=cmd(zoom,0.5875;diffuse,color("0.9,0.9,0.9");shadowlength,1);
		};
		LoadFont("Common Normal") .. {
			Text=string.format("%04i", 0);
			InitCommand=cmd(x,96;horizalign,right);
			OnCommand=cmd(zoom,0.5875;shadowlength,1);
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
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
			InitCommand=cmd(horizalign,left);
			OnCommand=cmd(zoom,0.5;shadowlength,1);
		};
		Def.Quad { 
			InitCommand=cmd(x,12;zoomto,50,10;horizalign,left);
			OnCommand=cmd(diffuse,Color("Black");shadowlength,1;diffusealpha,0.5);
		};
		Def.Quad {
			InitCommand=cmd(x,12;zoomto,50,10;horizalign,left);
			OnCommand=cmd(shadowlength,0;diffuse,Color("Green");diffusebottomedge,ColorLightTone(Color("Green")));
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
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
			InitCommand=cmd(x,14;zoom,0.5;halign,0;);
			OnCommand=cmd(shadowlength,1;strokecolor,color("0.15,0.15,0.15,0.625"));
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentStepsP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP1ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentTrailP2ChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
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
		InitCommand=cmd(x,-128+16+8;y,-14);
	};
	CreatePaneDisplayItem( iPN, "Jumps", 'RadarCategory_Jumps' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16);
	};
	CreatePaneDisplayItem( iPN, "Holds", 'RadarCategory_Holds' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16*2);
	};
	CreatePaneDisplayItem( iPN, "Mines", 'RadarCategory_Mines' ) .. {
		InitCommand=cmd(x,-128+16+8;y,-14+16*3);
	};
	-- Center
	CreatePaneDisplayItem( iPN, "Hands", 'RadarCategory_Hands' ) .. {
		InitCommand=cmd(x,8;y,-14);
	};
	CreatePaneDisplayItem( iPN, "Rolls", 'RadarCategory_Rolls' ) .. {
		InitCommand=cmd(x,8;y,-14+16);
	};
	CreatePaneDisplayItem( iPN, "Lifts", 'RadarCategory_Lifts' ) .. {
		InitCommand=cmd(x,8;y,-14+16*2);
	};
	CreatePaneDisplayItem( iPN, "Fakes", 'RadarCategory_Fakes' ) .. {
		InitCommand=cmd(x,8;y,-14+16*3);
	};
};
return t;