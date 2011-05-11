local gc = Var("GameCommand");

local t = Def.ActorFrame {
	LoadActor("PlayMode " .. gc:GetText() );
};

t[1].OnCommand=Screen.Metric("ScrollerItemOnCommand");
t[1].GainFocusCommand=Screen.Metric("ScrollerItemGainFocusCommand");
t[1].LoseFocusCommand=Screen.Metric("ScrollerItemLoseFocusCommand");
t[1].OffFocusedCommand=Screen.Metric("ScrollerItemOffFocusedCommand");
t[1].OffUnfocusedCommand=Screen.Metric("ScrollerItemOffUnfocusedCommand");
return t;