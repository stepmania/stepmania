InitUserPrefs();
local t = Def.ActorFrame {};

t[#t+1] = StandardDecorationFromFileOptional("Logo","Logo");
t[#t+1] = StandardDecorationFromFileOptional("VersionInfo","VersionInfo");
t[#t+1] = StandardDecorationFromFileOptional("CurrentGametype","CurrentGametype");
t[#t+1] = StandardDecorationFromFileOptional("LifeDifficulty","LifeDifficulty");
t[#t+1] = StandardDecorationFromFileOptional("TimingDifficulty","TimingDifficulty");
t[#t+1] = StandardDecorationFromFileOptional("NetworkStatus","NetworkStatus");
t[#t+1] = StandardDecorationFromFileOptional("SystemDirection","SystemDirection");

return t