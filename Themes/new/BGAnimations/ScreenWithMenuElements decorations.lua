local t = Def.ActorFrame {}
--
t[#t+1] = StandardDecorationFromFile("Header","Header");
t[#t+1] = StandardDecorationFromFile("Footer","Footer");
t[#t+1] = StandardDecorationFromFileOptional("TextHeader","TextHeader");
t[#t+1] = StandardDecorationFromFileOptional("TextSubtitle","TextSubtitle");
t[#t+1] = StandardDecorationFromFile("CustomOverlay","CustomOverlay");
--
return t;