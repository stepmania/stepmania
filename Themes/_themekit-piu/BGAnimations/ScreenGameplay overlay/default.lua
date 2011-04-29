--a simple life display...
local function LifeDisplay(pn)
	local function update(self)
		--local this = self:GetChildren()
		local lifemeter = SCREENMAN:GetTopScreen():GetLifeMeter(pn)
		--ºvº
		local life = lifemeter:GetLife()
		life = clamp(life,0,1)
		
		self:GetChild("LifeText"):settextf("Life: %.2f%%", life*100)
	end
	
	return Def.ActorFrame {
		InitCommand=cmd(player,pn;SetUpdateFunction,update);
		
		LoadFont("_arial black 20px")..{
			--Text="100%";
			Name="LifeText";
		};
	};
end

return Def.ActorFrame {
	LoadFont("_impact 50px")..{
		InitCommand=cmd(CenterX;FromTop,30);
		Text=string.format("%02i", STATSMAN:GetStagesPlayed()+1);
	};
	LifeDisplay(PLAYER_1)..{ InitCommand=cmd(FromCenterX,-160;FromTop,20); };
	LifeDisplay(PLAYER_2)..{ InitCommand=cmd(FromCenterX,160;FromTop,20); };
}