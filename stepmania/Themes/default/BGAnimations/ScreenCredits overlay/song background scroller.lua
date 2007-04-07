local template = Def.ActorFrame {
	children = {
		Def.Sprite {
			OnCommand = cmd(LoadBackground,GetRandomSongBackground();scaletoclipped,312,232);
		};
		LoadActor( THEME:GetPathG("ScreenCredits", "background frame") );
	};
}
local children = {}

for i=1,10 do
	table.insert( children, template )
end

return Def.ActorScroller {
	SecondsPerItem = 4.25;
	NumItemsToDraw = 4;
	TransformFunction = function( self, offset, itemIndex, numItems )
		self:y(-240*offset)
	end;
	OnCommand = cmd(scrollwithpadding,2,3);
	children = children;
}
