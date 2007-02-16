local text = ...;

return Def.ActorFrame {
        OnCommand=cmd();
        OffFocusedCommand=cmd();
        OffUnfocusedCommand=cmd();

	children =
	{
		LoadFont( "Common", "normal" ) .. {
			Text=THEME:GetString("PaneDisplay", text);
			OnCommand=cmd(shadowlength,0;horizalign,left;zoom,0.5);
		};
	};
};
