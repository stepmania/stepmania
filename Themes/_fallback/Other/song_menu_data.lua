local option_data= {
	AutosyncSetting= {broad_type= "choice", choices= AutosyncType, value_type= "enum"},
	MusicRate= {broad_type= "percent", min= .5, max= 2},
	Haste= {broad_type= "percent", min= -2, max= 2},
}

for i, name in ipairs{
	"AssistClap", "AssistMetronome", "StaticBackground","RandomBGOnly",
	"SaveScore","SaveReplay",
} do
	option_data[name]= {broad_type= "bool"}
end

return option_data
