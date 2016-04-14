return Def.ActorFrame {
	Def.Actor{
		Name="GroupMesageController_Col";
		SetMessageCommand=function(self,params)
			if params.HasFocus and not params.Song then
				focusIdx = params.Index
				local displayText = params.SongGroup;
				local sectionCount = self:GetParent():GetParent():GetChild("SectionCount");
				local songs = 0;
				if sectionCount then
					songs = sectionCount:GetText();
				end;

				MESSAGEMAN:Broadcast("SectionText", { Text = displayText, Count = tonumber(songs) });
			end;
		end;
	};

	-- light thing
	Def.Quad{
		InitCommand=cmd(x,-80;zoomto,8,24;diffuse,HSVA(192,0.8,1,0.5);diffusebottomedge,HSVA(192,0.8,0.925,1));
	};

	-- bottom line
	Def.Quad{
		InitCommand=cmd(x,48;y,16;zoomto,SCREEN_CENTER_X,2;diffuse,color("1,1,1,0.5");fadeleft,0.25;faderight,0.25);
	};
};