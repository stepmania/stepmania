local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	LoadActor("_Background");
};
--
for idx,diff in pairs(Difficulty) do
	local sDifficulty = ToEnumShortString( diff );
	local tLocation = {
		Beginner	= 16,
		Easy 		= 16*2,
		Medium		= 16*3,
		Hard		= 16*4,
		Challenge	= 16*5,
		Edit 		= 16*7,
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
					if meter >= 50 then
						meter = "!";
					end;
					if diff == 'Difficulty_Edit' then
						meter = meter .. " Edit";
					end;
				end;
			end;
			c.Meter:settext( meter );
			self:playcommand( bHasStepsTypeAndDifficulty and "Show" or "Hide" );
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		--
		LoadActor("_barpeice " .. sDifficulty ) .. {
			Name="BarPeice";
			ShowCommand=cmd(stoptweening;linear,0.1;diffuse,CustomDifficultyToColor( sDifficulty ));
			HideCommand=cmd(stoptweening;decelerate,0.2;diffuse,CustomDifficultyToDarkColor( sDifficulty ));
			InitCommand=cmd(diffuse,CustomDifficultyToColor( sDifficulty ));
		};
		LoadFont("StepsDisplay","Meter") .. {
			Name="Meter";
			Text=(sDifficulty == "Edit") and "0 Edits" or "0";
			ShowCommand=cmd(stoptweening;linear,0.1;diffuse,CustomDifficultyToColor( sDifficulty ));
			HideCommand=cmd(stoptweening;decelerate,0.2;diffuse,CustomDifficultyToDarkColor( sDifficulty ));
			InitCommand=cmd(x,-64-8+tLocation[sDifficulty];shadowlength,1;zoom,0.5;diffuse,CustomDifficultyToColor( sDifficulty ));
		};
	};
end
return t