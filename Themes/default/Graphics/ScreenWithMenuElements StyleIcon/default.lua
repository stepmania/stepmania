local t = {};

local style = GAMESTATE:GetCurrentStyle();
if style then
	local master_pn = GAMESTATE:GetMasterPlayerNumber();
	local st = style:GetStyleType();
	local pad_file = "";
	if st == "StyleType_OnePlayerOneSide"  or  st == "StyleType_OnePlayerTwoSides" then
		pad_file = st .. " " .. master_pn;
	elseif st == "StyleType_TwoPlayersTwoSides" then
		pad_file = st;
	end

	return LoadActor( THEME:GetPathG("","_StyleIcon " .. pad_file ) ) .. {};
else
	return Def.Actor {};
end
