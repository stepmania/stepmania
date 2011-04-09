return Def.ActorFrame {
	Draw.RoundBox(276,212,20,20,color("#323232"));
	Draw.RoundBox(256,192,10,10)..{
		Name="ColorFill";
		--InitCommand=cmd(zoomto,256,192);
		SetMessageCommand=function(self,params)
			local color = {1,1,1,1}
			if PREFSMAN:GetPreference("MusicWheelUsesSections") ~= 'MusicWheelUsesSections_Never'
			and GAMESTATE:GetSortOrder() == 'SortOrder_Group'
			then
				local goupcolor = SONGMAN:GetSongGroupColor(params.SongGroup)
				
				if goupcolor ~= nil then
					color = SONGMAN:GetSongGroupColor(params.SongGroup)
				end
			end
			
			self:diffuse(color);
		end;
	};
	
	LoadFont("_impact 50px")..{
		Name="GroupName";
		InitCommand=cmd(shadowlength,0;Stroke,color("#000000");maxwidth,210);
		SetMessageCommand=function(self,params)
			text = params.SongGroup
			self:settext(text);
		end;
	};
}
