local t = Def.ActorFrame {
	LoadActor( "../ScreenTitleMenu overlay" );
	LoadActor( "pay" ) .. {
		OnCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+120;diffuseshift;effectcolor1,0.5,0.5,0.5,1;effectcolor2,1,1,1,1);
		Condition = GAMESTATE:GetCoinMode() == "CoinMode_Pay";
	};
	LoadActor( "jp" ) .. {
		Stretch = 1;
		NumTilesHigh = 1;
		TilesStartY = 380;
		TilesSpacingX = 10000;
		TileVelocityX = -300;
		OnCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+50);
		Condition = GAMESTATE:GetPremium() == "Premium_2PlayersFor1Credit";
	};
	LoadActor( "free" ) .. {
		OnCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y-60;diffuseshift;effectcolor1,0.5,0.5,0.5,1;effectcolor2,1,1,1,1);
		Condition = GAMESTATE:GetCoinMode() == "CoinMode_Free";
	};
	LoadActor( "dp" ) .. {
		Stretch = 1;
		NumTilesHigh = 1;
		TilesStartY = 380;
		TilesSpacingY = 10000;
		TileVelocityX = -300;
		OnCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+50);
		Condition = GAMESTATE:GetPremium() == "Premium_DoubleFor1Credit";
	};
}
return t
