local template = Def.ActorFrame {
	Def.Sprite {
		OnCommand = cmd(LoadBackground,GetRandomSongBackground();scaletoclipped,312,232);
	};
	LoadActor( THEME:GetPathG("ScreenCredits", "background frame") );
}
local t = Def.ActorScroller {
	SecondsPerItem = 4.25;
	NumItemsToDraw = 4;
	TransformFunction = function( self, offset, itemIndex, numItems )
		self:y(-240*offset)
	end;
	OnCommand = cmd(scrollwithpadding,2,3);
}

for i=1,10 do
	table.insert( t, template );
end

return t;
