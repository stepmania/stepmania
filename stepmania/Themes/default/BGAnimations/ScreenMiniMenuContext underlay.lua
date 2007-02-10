local children = {
	Def.ActorFrame {
		OnCommand=function(self)
			local width=136
			local height=130
			local thickness=3
			self:GetChild("middle"):zoomtoheight(0)
			self:GetChild("middle"):zoomtowidth(width+thickness)
			self:GetChild("middle"):zoomtoheight(height+thickness)
			self:GetChild("top"):y(0)
			self:GetChild("top"):zoomtowidth(width+thickness)
			self:GetChild("top"):y(-height/2)
			self:GetChild("bottom"):y(0)
			self:GetChild("bottom"):zoomtowidth(width+thickness)
			self:GetChild("bottom"):y(height/2)
			self:GetChild("left"):zoomtoheight(0)
			self:GetChild("left"):x(-width/2)
			self:GetChild("left"):zoomtoheight(height+thickness)
			self:GetChild("right"):zoomtoheight(0)
			self:GetChild("right"):x(width/2)
			self:GetChild("right"):zoomtoheight(height+thickness)
		end;

		children = {
			Def.Quad {
				Name="middle";
				InitCommand=cmd(diffuse,0,0,0,1;shadowlength,6);
				OnCommand=cmd();
			};
			Def.Quad {
				Name="top";
				InitCommand=cmd(zoomtoheight,3);
			};
			Def.Quad {
				Name="bottom";
				InitCommand=cmd(zoomtoheight,3;diffuse,0.2,0.2,1,1);
			};
			Def.Quad {
				Name="left";
				InitCommand=cmd(zoomtowidth,3;diffusebottomedge,0.2,0.2,1,1);
			};
			Def.Quad {
				Name="right";
				InitCommand=cmd(zoomtowidth,3;diffusebottomedge,0.2,0.2,1,1);
			};
		};
	};
}

return Def.ActorFrame { children = children };
