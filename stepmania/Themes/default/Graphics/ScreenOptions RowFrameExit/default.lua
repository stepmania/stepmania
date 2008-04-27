return Def.ActorFrame {
	LoadActor("focus") .. {
		GainFocusCommand=cmd(visible,true;);
		LoseFocusCommand=cmd(visible,false;);
	};
	LoadActor("nofocus") .. {
		GainFocusCommand=cmd(visible,false;);
		LoseFocusCommand=cmd(visible,true;);
	};
};