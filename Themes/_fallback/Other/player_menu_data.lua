local option_data= {
	BatteryLives= {broad_type= "number"},
}

for i, name in ipairs{
	"TurnNone", "Mirror", "Backwards", "Left", "Right", "Shuffle",
	"SoftShuffle", "SuperShuffle", "NoHolds", "NoRolls", "NoMines", "Little", 
	"Wide", "Big", "Quick", "BMRize", "Skippy", "Mines", "AttackMines",
	"Echo", "Stomp", "Planted", "Floored", "Twister", "HoldRolls", "NoJumps", 
	"NoHands", "NoLifts", "NoFakes", "NoQuads", "NoStretch", "MuteOnError",
} do
	option_data[name]= {broad_type= "bool"}
end
-- The time life bar doesn't work sensibly outside the survival courses, so
-- keep it out of the menu.
local life_types= {"LifeType_Bar", "LifeType_Battery"}
for i, info in ipairs{
	{"FailSetting", FailType}, {"MinTNSToHideNotes", TapNoteScore},
	{"LifeSetting", life_types}, {"DrainSetting", DrainType},
} do
	option_data[info[1]]= {broad_type= "choice", choices= info[2]}
end

return option_data
