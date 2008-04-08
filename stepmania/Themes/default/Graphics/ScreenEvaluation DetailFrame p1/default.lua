return Def.ActorFrame {
	LoadActor("frame") .. {
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Hands") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*0;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Holds") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*1;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Jumps") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*2;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Mines") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*3;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label NumSteps") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*4;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
	LoadActor( THEME:GetPathG("PaneDisplay","label Rolls") ) .. {
		InitCommand=cmd(horizalign,right;x,-22;y,-12+16*5;shadowlengthx,0;shadowlengthy,2;shadowcolor,color("#000000AA"););
	};
};