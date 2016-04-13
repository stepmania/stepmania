return Font("mentone", "24px")..{
	InitCommand=cmd(shadowlength,0;halign,0;zoom,0.5);
	BeginCommand=cmd(playcommand,"Set");

	SetCommand=function(self)
		local sText = GAMESTATE:GetSongOptionsString()
		self:settext( sText )
		--[[
		if GAMESTATE:IsAnExtraStage() then
			self:diffuseblink()
		else
			self:stopeffect()
		end
		--]]
	end;
	SongOptionsChangedMessageCommand=cmd(playcommand,"Set");
};