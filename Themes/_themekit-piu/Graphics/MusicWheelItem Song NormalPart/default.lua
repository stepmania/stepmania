return Def.ActorFrame {
	Draw.RoundBox(276,212,20,20,color("#323232"));
	Def.Banner {
		Name="SongBanner";
		InitCommand=cmd(scaletoclipped,256,192);
		SetMessageCommand=function(self,params)
			local path = params.Song:GetBannerPath()
			if not path then path = THEME:GetPathG("Common","fallback banner") end
			
			local bHighResTextures = PREFSMAN:GetPreference("HighResolutionTextures")
			
			--banner loading stuff...
			--is on
			if bHighResTextures == 'HighResolutionTextures_ForceOn' or GetUserPrefB("goodbanners") then
				self:LoadFromSong(params.Song);
			--is off
			elseif bHighResTextures == 'HighResolutionTextures_ForceOff' then
				self:LoadFromCachedBanner(path);
			--is auto
			elseif bHighResTextures == 'HighResolutionTextures_Auto' then
				local iWidth = DISPLAY:GetDisplayHeight()
				--display is over ninethousand eh... 480
				if iWidth > 480 then
					self:LoadFromSong(params.Song);
				--display is 480 or less
				else
					self:LoadFromCachedBanner(path);
				end
			end
		end;
	};
}
