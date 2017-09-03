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
for i, info in ipairs{
	{"FailSetting", FailType}, {"MinTNSToHideNotes", TapNoteScore},
	{"LifeSetting", LifeType}, {"DrainSetting", DrainType},
} do
	option_data[info[1]]= {broad_type= "choice", choices= info[2], value_type= "enum"}
end

return option_data
