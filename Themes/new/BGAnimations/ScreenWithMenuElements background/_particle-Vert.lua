local t = Def.ActorFrame{};

local Params = { 
	NumParticles = 45,
	VelocityXMin = -1,
	VelocityXMax = 1,
	VelocityYMin = -250,
	VelocityYMax = -130,
	VelocityZMin = 0,
	VelocityZMax = 0,
	BobRateZMin = 0.4,
	BobRateZMax = 0.7,
	ZoomMin = 0.5,
	ZoomMax = 1,
	SpinZ = 0,
	BobZ = 52,
    Width = 8,
    Height = 8,
	File = "_block",
};

local tParticleInfo = {}

for i=1,Params.NumParticles do
	tParticleInfo[i] = {
		X = 0,
		Y = Params.VelocityYMin ~= Params.VelocityYMax and math.random(Params.VelocityYMin, Params.VelocityYMax) or Params.VelocityYMin,
		Z = Params.VelocityZMin ~= Params.VelocityZMax and math.random(Params.VelocityZMin, Params.VelocityZMax) or Params.VelocityZMin,
		Zoom = math.random(Params.ZoomMin*1000,Params.ZoomMax*1000) / 1000,
		BobZRate = math.random(Params.BobRateZMin*1000,Params.BobRateZMax*1000) / 1000,
		Age = 0,
	};
	t[#t+1] = LoadActor( Params.File )..{
	Name="Particle"..i;
		InitCommand=function(self)
		self:basezoom(tParticleInfo[i].Zoom);
		self:x(math.random(SCREEN_LEFT+(Params.Width/2),SCREEN_RIGHT-(Params.Width/2)));
		self:y(math.random(SCREEN_TOP+(Params.Height/2),SCREEN_BOTTOM-(Params.Height/2)));
		--self:z(math.random(-64,0));
	end;
		OnCommand=cmd(diffuse,Color.Black;diffusealpha,0.65);
	};
end

local function UpdateParticles(self,DeltaTime)
	tParticles = self:GetChildren();
	for i=1, Params.NumParticles do
		local p = tParticles["Particle"..i];
		local vX = tParticleInfo[i].X;
		local vY = tParticleInfo[i].Y;
		local vZ = tParticleInfo[i].Z;
		local vAlpha = scale(p:GetY(),SCREEN_BOTTOM,SCREEN_CENTER_Y-60,0,1);
		tParticleInfo[i].Age = tParticleInfo[i].Age + DeltaTime;
		p:x(p:GetX() + (vX * DeltaTime));
		p:y(p:GetY() + (vY * DeltaTime));
		p:z(p:GetZ() + (vZ * DeltaTime));
		p:diffusealpha( 0.45 - (vAlpha*0.45) );
--		p:zoom( 1 + math.cos(
--			(tParticleInfo[i].Age * math.pi*2) 
--			)	* 0.125 );
		if p:GetX() > SCREEN_RIGHT + (Params.Width/2 - p:GetZ()) then
			p:x(SCREEN_LEFT - (Params.Width/2));
		elseif p:GetX() < SCREEN_LEFT - (Params.Width/2 - p:GetZ()) then
			p:x(SCREEN_RIGHT + (Params.Width/2));
		end
		if p:GetY() > SCREEN_BOTTOM + (Params.Height/2 - p:GetZ()) then
			p:y(SCREEN_TOP - (Params.Height/2));
		elseif p:GetY() < SCREEN_TOP - (Params.Height/2 - p:GetZ()) then
			p:y(SCREEN_BOTTOM + (Params.Height/2));
		end
	end;
end;

t.InitCommand = cmd(fov,90;SetUpdateFunction,UpdateParticles);

return t;
