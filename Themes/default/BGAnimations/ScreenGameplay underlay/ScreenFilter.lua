--[[ Screen Filter ]]
local numPlayers = GAMESTATE:GetNumPlayersEnabled()
local center1P = PREFSMAN:GetPreference("Center1Player")

local padding = 8 -- 4px on each side
local arrowWidth = 96 -- until noteskin metrics are implemented...

local filterColor = color("#000000")
local filterAlphas = {
	PlayerNumber_P1 = 1,
	PlayerNumber_P2 = 1,
	Default = 0,
}

local t = Def.ActorFrame{};

local style = GAMESTATE:GetCurrentStyle()
local cols = style:ColumnsPerPlayer()
local styleType = ToEnumShortString(style:GetStyleType())
local filterWidth = (arrowWidth * cols) + padding
local stepsType = style:GetStepsType()

if numPlayers == 1 then
	local player = GAMESTATE:GetMasterPlayerNumber()
	local pNum = (player == PLAYER_1) and 1 or 2
	filterAlphas[player] = tonumber(getenv("ScreenFilterP"..pNum)) or 0;

	local pos;
	-- [ScreenGameplay] PlayerP#Player*Side(s)X
	if center1P then
		pos = SCREEN_CENTER_X
	elseif stepsType == "StepsType_Dance_Solo" then
		pos = SCREEN_CENTER_X
	else
		local metricName = string.format("PlayerP%i%sX",pNum,styleType)
		pos = THEME:GetMetric("ScreenGameplay",metricName)
	end
	t[#t+1] = Def.Quad{
		Name="SinglePlayerFilter";
		InitCommand=cmd(x,pos;CenterY;zoomto,filterWidth,SCREEN_HEIGHT;diffusecolor,filterColor;diffusealpha,filterAlphas[player]);
	};
else
	-- two players... a bit more complex.
	if styleType == "TwoPlayersSharedSides" then
		-- routine, just use one in the center.
		local player = GAMESTATE:GetMasterPlayerNumber()
		local pNum = player == PLAYER_1 and 1 or 2
		local metricName = "PlayerP".. pNum .."TwoPlayersSharedSidesX"
		t[#t+1] = Def.Quad{
			Name="RoutineFilter";
			InitCommand=cmd(x,THEME:GetMetric("ScreenGameplay",metricName);CenterY;zoomto,filterWidth,SCREEN_HEIGHT;diffusecolor,filterColor;diffusealpha,filterAlphas[player]);
		};
	else
		-- otherwise we need two separate ones. to the pairsmobile!
		for i, player in ipairs(PlayerNumber) do
			local pNum = (player == PLAYER_1) and 1 or 2
			filterAlphas[player] = tonumber(getenv("ScreenFilterP"..pNum)) or 0;
			local metricName = string.format("PlayerP%i%sX",pNum,styleType)
			local pos = THEME:GetMetric("ScreenGameplay",metricName)
			t[#t+1] = Def.Quad{
				Name="Player"..pNum.."Filter";
				InitCommand=cmd(x,pos;CenterY;zoomto,filterWidth,SCREEN_HEIGHT;diffusecolor,filterColor;diffusealpha,filterAlphas[player]);
			};
		end
	end
end

return t;
