-- color based on screen name
function ScreenColor(screen)
    local colors = {
        ["ScreenSelectStyle"]         = ScreenColors.Style, 
        ["ScreenSelectPlayMode"]      = ScreenColors.PlayMode, 
        ["ScreenSelectMusic"]         = ScreenColors.Music, 
        ["ScreenSelectCourse"]        = ScreenColors.Course, 
        ["ScreenPlayerOptions"]       = ScreenColors.PlayerOptions,
        ["ScreenNestyPlayerOptions"]  = ScreenColors.PlayerOptions,
        ["ScreenOptionsService"]      = ScreenColors.OptionsService,
        ["ScreenEvaluationNormal"]    = ScreenColors.Evaluation, 
        ["ScreenHighScores"]    = ScreenColors.Evaluation, 
        ["ScreenEvaluationSummary"]   = ScreenColors.Summary, 
        ["ScreenStageInformation"]   = ScreenColors.StageInformation, 
        ["ScreenEditMenu"]			  = ScreenColors.Edit, 
        ["ScreenSMOnlineLogin"]			  = ScreenColors.Online, 
        ["ScreenNetRoom"]			  = ScreenColors.Online, 
        ["ScreenNetSelectMusic"]			  = ScreenColors.Online, 
		["ScreenNetEvaluation"]    = ScreenColors.Evaluation, 
        ["Default"]                   = ScreenColors.Default,
    }

    if colors[screen] then return colors[screen];
    else return colors["Default"]; end;
end;

ScreenColors = {
    Style           = color("#882D47"),
    PlayMode        = color("#882D47"),
    Music           = color("#882D47"),
    Online           = color("#882D47"),
    Course          = color("#882D47"),
    PlayerOptions   = color("#882D47"),
    OptionsService  = color("#882D47"),
    Evaluation      = color("#882D47"),
    Summary         = color("#882D47"),
    StageInformation  = color("#D05722"),
    Edit         = color("#B34754"),
    Default         = color("#882D47"),
}

ModeIconColors = {
    Normal      = color("#1AE0E4"),
    Rave        = color("#3ACF2A"), 
    Nonstop     = color("#CFC42A"),
    Oni         = color("#CF502A"),
    Endless     = color("#981F41"),
}

GameColor = {
    PlayerColors = {
        PLAYER_1 = color("#4B82DC"),
        PLAYER_2 = color("#DF4C47"),
		both = color("#FFFFFF"),
    },
    PlayerDarkColors = {
        PLAYER_1 = color("#16386E"),
        PLAYER_2 = color("#65110F"),
		both = color("#F5E1E1"),
    },
    Difficulty = {
        --[[ These are for 'Custom' Difficulty Ranks. It can be very  useful
        in some cases, especially to apply new colors for stuff you
        couldn't before. (huh? -aj) ]]
        Beginner    = color("#1AE0E4"),         -- Mint
        Easy        = color("#3ACF2A"),         -- Green
        Medium      = color("#CFC42A"),         -- Yellow
        Hard        = color("#CF502A"),         -- Orange
        Challenge   = color("#981F41"),         -- Plum
        Edit        = color("0.8,0.8,0.8,1"),   -- Gray
        Couple      = color("#ed0972"),         -- hot pink
        Routine     = color("#ff9a00"),         -- orange
        --[[ These are for courses, so let's slap them here in case someone
        wanted to use Difficulty in Course and Step regions. ]]
        Difficulty_Beginner = color("#1AE0E4"),     -- Mint
        Difficulty_Easy     = color("#2FA74D"),     -- Green
        Difficulty_Medium   = color("#CFC42A"),     -- Yellow
        Difficulty_Hard     = color("#CF502A"),     -- Orange
        Difficulty_Challenge    = color("#981F41"), -- Plum
        Difficulty_Edit     = color("0.8,0.8,0.8,1"),       -- gray
        Difficulty_Couple   = color("#ed0972"),             -- hot pink
        Difficulty_Routine  = color("#ff9a00")              -- orange
    },
    Stage = {
        Stage_1st   = color("#9d324e"),
        Stage_2nd   = color("#9d3262"),
        Stage_3rd   = color("#9d3280"),
        Stage_4th   = color("#9d329d"),
        Stage_5th   = color("#7b329d"),
        Stage_6th   = color("#52329d"),
        Stage_Next  = color("#52329d"),
        Stage_Final = color("#325c9d"),
        Stage_Extra1    = color("#B60052"),
        Stage_Extra2    = color("#FF499B"),
        Stage_Nonstop   = color("#9d324e"),
        Stage_Oni   = color("#9d3232"),
        Stage_Endless   = color("#9d3232"),
        Stage_Event = color("#9d324e"),
        Stage_Demo  = color("#9d324e")
    },
    Judgment = {
        JudgmentLine_W1     = color("#A0DBF1"),
        JudgmentLine_W2     = color("#F1E4A2"),
        JudgmentLine_W3     = color("#ABE39B"),
        JudgmentLine_W4     = color("#86ACD6"),
        JudgmentLine_W5     = color("#958CD6"),
        JudgmentLine_Held   = color("#FFFFFF"),
        JudgmentLine_Miss   = color("#F97E7E"),
        JudgmentLine_MaxCombo   = color("#ffc600")
    },
}

GameColor.Difficulty["Crazy"]       = GameColor.Difficulty["Hard"]
GameColor.Difficulty["Freestyle"]   = GameColor.Difficulty["Easy"]
GameColor.Difficulty["Nightmare"]   = GameColor.Difficulty["Challenge"]
GameColor.Difficulty["HalfDouble"]  = GameColor.Difficulty["Medium"]