local t = LoadFallbackB();

t[#t+1] = StandardDecorationFromFileOptional("BPMLabel","BPMLabel");
t[#t+1] = StandardDecorationFromFileOptional("BPMDisplay","BPMDisplay");
t[#t+1] = StandardDecorationFromFileOptional("DifficultyDisplay","DifficultyDisplay");
return t;