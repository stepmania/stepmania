local t = Def.ActorFrame {};
local function GetEdits( in_Song, in_StepsType )
	if in_Song then
		local sSong = in_Song;
		local sCurrentStyle = GAMESTATE:GetCurrentStyle();
		local sStepsType = in_StepsType;
		local iNumEdits = 0;
		if sSong:HasEdits( sStepsType ) then
			local tAllSteps = sSong:GetAllSteps();
			for i,Step in pairs(tAllSteps) do
				if Step:IsAnEdit() and Step:GetStepsType() == sStepsType then
					iNumEdits = iNumEdits + 1;
				end
			end
			return iNumEdits;
		else
			return iNumEdits;
		end
	else
		return 0;
	end
end;
t[#t+1] = Def.ActorFrame {
	LoadActor("_Background");
};
--
for idx,diff in pairs(Difficulty) do
	local sDifficulty = ToEnumShortString( diff );
	local tLocation = {
		Beginner	= 32*-1.25,
		Easy 		= 32*-0.25,
		Medium		= 32*0.75,
		Hard		= 32*1.75,
		Challenge	= 32*2.75,
		Edit 		= 32*4.75,
	};
	t[#t+1] = Def.ActorFrame {
		SetCommand=function(self)
			local c = self:GetChildren();
-- 			local Bar = self:GetChild("Bar");
-- 			local Meter = self:GetChild("Meter"
			local song = GAMESTATE:GetCurrentSong()
			local bHasStepsTypeAndDifficulty = false;
			local meter = "";
			if song then
				local st = GAMESTATE:GetCurrentStyle():GetStepsType()
				bHasStepsTypeAndDifficulty = song:HasStepsTypeAndDifficulty( st, diff );
				local steps = song:GetOneSteps( st, diff );
				if steps then
					meter = steps:GetMeter();
					append = ""
					--
					if diff == 'Difficulty_Edit' then
						meter = GetEdits( song, st );
						append = ( GetEdits( song, st ) > 1 ) and "Edits" or "Edit"
						meter = meter .. " " .. append
					end
				end
			end
			c.Meter:settext( meter );
			self:playcommand( bHasStepsTypeAndDifficulty and "Show" or "Hide" );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		--
		LoadActor("_barpeice " .. sDifficulty ) .. {
			Name="BarPeice";
			ShowCommand=cmd(stoptweening;linear,0.1;diffuse,CustomDifficultyToColor( sDifficulty ));
			HideCommand=cmd(stoptweening;decelerate,0.05;diffuse,CustomDifficultyToDarkColor( sDifficulty ));
			InitCommand=cmd(diffuse,CustomDifficultyToColor( sDifficulty ));
		};
		LoadFont("StepsDisplay","Meter") .. {
			Name="Meter";
			Text=(sDifficulty == "Edit") and "0 Edits" or "0";
			ShowCommand=cmd(stoptweening;linear,0.1;diffuse,CustomDifficultyToColor( sDifficulty ));
			HideCommand=cmd(stoptweening;decelerate,0.05;diffuse,CustomDifficultyToDarkColor( sDifficulty ));
			InitCommand=cmd(x,-64-8+tLocation[sDifficulty];shadowlength,1;zoom,0.75;diffuse,CustomDifficultyToColor( sDifficulty ));
		};
	};
end
return t