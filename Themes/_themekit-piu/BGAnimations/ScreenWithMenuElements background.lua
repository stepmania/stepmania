local particles = {
	Def.Quad { 
		InitCommand=function(self)
			self:FullScreen();
			self:diffuse(color("#003200"));
			self:diffusetopedge(color("#000000"));
		end;
	}
}

for i=1,30 do
	particles[#particles+1] = Draw.Oval(math.random(20,80))..{
		InitCommand=function(self)
			self:finishtweening();
			self:decelerate(0.25);
			
			self:x(math.random(20,SCREEN_WIDTH-20));
			self:y(math.random(20,SCREEN_HEIGHT-20));
			
			self:diffuse(math.random(0.2,1),math.random(0.2,1),math.random(0.2,1),math.random(0.75,0.95))
		end;
		ScreenChangedMessageCommand=function(self)
			self:playcommand("Init");
		end;
	}
end

return Def.ActorFrame {
	children = particles;
}