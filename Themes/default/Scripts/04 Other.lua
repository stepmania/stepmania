function SongMeterDisplayX(pn)
	if Center1Player() then
		return SCREEN_CENTER_X
	else
		return pn == PLAYER_1 and SCREEN_LEFT+16 or SCREEN_RIGHT-16
	end
end

function SongMeterDisplayY(pn)
	return Center1Player() and SCREEN_TOP+50 or SCREEN_CENTER_Y
end

function SongMeterDisplayCommand(pn)
	if Center1Player() then
		return cmd(draworder,50;zoom,0;y,SCREEN_TOP-24;sleep,1.5;decelerate,0.5;zoom,1;y,SCREEN_TOP+50)
	else
		local xAdd = (pn == PLAYER_1) and -24 or 24
		return cmd(draworder,5;rotationz,-90;zoom,0;addx,xAdd;sleep,1.5;decelerate,0.5;zoom,1;addx,xAdd*-1)
	end
end

function thify_number(n)
	if n >= 10 and n < 20 then return n .. "th" end
	local th_suffixes= {"st", "nd", "rd"}
	local end_digit= n % 10
	return n .. (th_suffixes[end_digit] or "th")
end

local numbered_stages= {
	Stage_1st= true,
	Stage_2nd= true,
	Stage_3rd= true,
	Stage_4th= true,
	Stage_5th= true,
	Stage_6th= true,
	Stage_Next= true,
}

function is_stage_final()
	-- The correct way to determine whether the player has reached Final Stage
	-- is to check the number of stages left.
	-- If you use the SongsPerPlay preference, you become incorrect after a
	-- continue occurs.
	local master_player= GAMESTATE:GetMasterPlayerNumber()
	local stages_left= GAMESTATE:GetNumStagesLeft(master_player)
	local stage_cost= GAMESTATE:GetCurrentSong():GetStageCost()
	return (stages_left - stage_cost) < 1
end

function thified_curstage_index(on_eval)
	local cur_stage= GAMESTATE:GetCurrentStage()
	local adjust= 1
	-- hack: ScreenEvaluation shows the current stage, but it needs to show
	-- the last stage instead.  Adjust the amount.
	if on_eval then
		adjust= 0
	end
	if numbered_stages[cur_stage] then
		return thify_number(GAMESTATE:GetCurrentStageIndex() + adjust)
	else
		return ToEnumShortString(cur_stage)
	end
end
