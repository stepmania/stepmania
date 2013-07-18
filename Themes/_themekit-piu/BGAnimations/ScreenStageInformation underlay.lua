local song = GAMESTATE:GetCurrentSong()
--assert(song,"WHY THIS HAPPENS LOL")
if not song then
	song = SONGMAN:GetRandomSong()
	GAMESTATE:SetCurrentSong( song )
	
	for k,v in ipairs(PlayerNumber) do
		local stepsplayer = GAMESTATE:GetCurrentSteps(v)
		steps = song:GetOneSteps( stepsplayer:GetStepsType() , stepsplayer:GetDifficulty() )
		GAMESTATE:SetCurrentSteps( steps )
	end
end

local path = song:GetBackgroundPath()

SplitMode();

if not path then
	path = THEME:GetPathG("Common", "fallback background")
end

return LoadActor(path)..{
	InitCommand=cmd(scaletocover,0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
};