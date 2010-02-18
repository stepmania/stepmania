return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		Text="BPM";
		InitCommand=cmd(horizalign,right;zoom,0.75;strokecolor,Color("Outline"));
		SetCommand=function(self)
			local bIsFirst = false;
			local song = GAMESTATE:GetCurrentSong();
			self:stoptweening();
-- 			self:linear(0.25);
			if song then
				self:diffusebottomedge( song:GetTimingData():HasStops() and color("#55CCEE") or color("#FFFFFF") );
			else
				self:diffusebottomedge( color("#FFFFFF") );
			end;
		end;
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	};
};