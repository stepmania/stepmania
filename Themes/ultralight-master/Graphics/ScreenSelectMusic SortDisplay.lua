return Def.ActorFrame{
	Font("mentone","24px")..{
		InitCommand=cmd(halign,1;shadowlength,0;zoom,0.5;strokecolor,color("0,0,0,0"));
		BeginCommand=cmd(playcommand,"Update");
		UpdateCommand=function(self)
			-- change the text based on SortOrder
			local so = GAMESTATE:GetSortOrder();
			if so ~= nil then
				self:settext( SortOrderToLocalizedString(so) );
			end;
		end;
		SortOrderChangedMessageCommand=cmd(playcommand,"Update");
	};
};