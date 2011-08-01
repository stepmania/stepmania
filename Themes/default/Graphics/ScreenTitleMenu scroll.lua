local gc = Var("GameCommand");

return Def.ActorFrame {
	Def.Quad{
		InitCommand=cmd(zoomto,192,26;diffuse,HSVA(192,0.8,1,0.45);blend,Blend.Add;fadeleft,0.5;faderight,0.35;skewx,-0.1);
		OnCommand=cmd(glowshift;effectcolor1,color("1,1,1,0");effectcolor2,color("1,1,1,0.125"););
		--[[
		GainFocusCommand=cmd(stoptweening;croptop,0;cropbottom,1;linear,0.05;cropbottom,0;);
		LoseFocusCommand=cmd(stoptweening;croptop,0;linear,0.05;croptop,1;);
		--]]
		GainFocusCommand=cmd(stoptweening;cropright,0;cropleft,1;linear,0.05;cropleft,0;);
		LoseFocusCommand=cmd(stoptweening;cropright,0;linear,0.05;cropright,1;);
	};
	LoadFont("Common Normal") .. {
		Text=THEME:GetString("ScreenTitleMenu",gc:GetText());
--[[ 		EnabledCommand=function(self)
			if string.find( THEME:GetMetric( Var "LoadingScreen", "DisabledChoices") , gc:GetText() ) ~= nil then
				self:diffuse(Color("Red"));
			end
		end; --]]
		OnCommand=cmd(strokecolor,Color("Outline"));
		GainFocusCommand=cmd(stoptweening;linear,0.125;zoom,1;diffuse,color("1,1,1,1"));
--[[ 		GainFocusCommand=function(self)
			if string.find( tostring( THEME:GetMetric( Var "LoadingScreen", "DisabledChoices") ) , gc:GetText() ) ~= nil then
				self:zoom(1);
				self:diffuse(Color("Red"));
			else
				self:zoom(1);
				self:diffuse(Color("White"));
			end
		end; --]]
		LoseFocusCommand=cmd(stoptweening;linear,0.125;zoom,0.75;diffuse,color("0.5,0.5,0.5,1"));
	};
};