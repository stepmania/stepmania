return Def.ActorFrame {
	LoadActor("frame") .. {
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Hands") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*0;);
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Holds") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*1;);
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Jumps") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*2;);
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Mines") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*3;);
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label NumSteps") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*4;);
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Rolls") ) .. {
		InitCommand=cmd(x,-50;y,-20+16*5;);
	};
};