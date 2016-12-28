if IsNetSMOnline() then
	-- don't show "Ready" online; it will obscure the immediately-starting steps.
	return Def.ActorFrame{}
end

local lang = THEME:GetCurLanguage()
local cur_dir = "/Themes/default/BGAnimations/ScreenGameplay go/"
if lang ~= "en" and FILEMAN:DoesFileExist(cur_dir.."go (lang "..lang..").png") then
	return LoadActor("go (lang "..lang..")") .. {
		InitCommand=cmd(Center;draworder,105);
		StartTransitioningCommand=cmd(zoom,1.3;diffusealpha,0;bounceend,0.25;zoom,1;diffusealpha,1;linear,0.15;glow,BoostColor(Color("Blue"),1.75);decelerate,0.3;glow,1,1,1,0;sleep,1-0.45;linear,0.25;diffusealpha,0;);
	};
else
	return LoadActor("go.png") .. {
		InitCommand=cmd(Center;draworder,105);
		StartTransitioningCommand=cmd(zoom,1.3;diffusealpha,0;bounceend,0.25;zoom,1;diffusealpha,1;linear,0.15;glow,BoostColor(Color("Blue"),1.75);decelerate,0.3;glow,1,1,1,0;sleep,1-0.45;linear,0.25;diffusealpha,0;);
	};
end
