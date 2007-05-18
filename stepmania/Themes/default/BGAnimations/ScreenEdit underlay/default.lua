return Def.ActorFrame {
	LoadActor("help.png") .. {
		InitCommand=cmd(y,SCREEN_CENTER_Y);
		OnCommand=cmd(horizalign,left;finishtweening);
		EditCommand=cmd(playcommand,"Show");
		PlayingCommand=cmd(playcommand,"Hide");
		RecordCommand=cmd(playcommand,"Hide");
		RecordPausedCommand=cmd(playcommand,"Hide");
		ShowCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_LEFT);
		HideCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_LEFT-192);
	};
	LoadActor("info.png") .. {
		InitCommand=cmd(y,SCREEN_CENTER_Y);
		OnCommand=cmd(horizalign,right;finishtweening);
		EditCommand=cmd(playcommand,"Show");
		PlayingCommand=cmd(playcommand,"Hide");
		RecordCommand=cmd(playcommand,"Hide");
		RecordPausedCommand=cmd(playcommand,"Hide");
		ShowCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_RIGHT);
		HideCommand=cmd(stoptweening;accelerate,0.25;x,SCREEN_RIGHT+192);
	};
};
