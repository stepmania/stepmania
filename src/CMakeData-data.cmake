list(APPEND SM_DATA_LUA_SRC
  "${SM_SRC_DATA_DIR}/LuaBinding.cpp"
  "${SM_SRC_DATA_DIR}/LuaExpressionTransform.cpp"
  "${SM_SRC_DATA_DIR}/LuaReference.cpp"
)

list(APPEND SM_DATA_LUA_HPP
  "${SM_SRC_DATA_DIR}/LuaBinding.h"
  "${SM_SRC_DATA_DIR}/LuaExpressionTransform.h"
  "${SM_SRC_DATA_DIR}/LuaReference.h"
)

source_group("Data Structures\\\\Lua" FILES ${SM_DATA_LUA_SRC} ${SM_DATA_LUA_HPP})

list(APPEND SM_DATA_FONT_SRC
  "${SM_SRC_DATA_DIR}/Font.cpp"
  "${SM_SRC_DATA_DIR}/FontCharAliases.cpp"
  "${SM_SRC_DATA_DIR}/FontCharmaps.cpp"
)

list(APPEND SM_DATA_FONT_HPP
  "${SM_SRC_DATA_DIR}/Font.h"
  "${SM_SRC_DATA_DIR}/FontCharAliases.h"
  "${SM_SRC_DATA_DIR}/FontCharmaps.h"
)

source_group("Data Structures\\\\Fonts" FILES ${SM_DATA_FONT_SRC} ${SM_DATA_FONT_HPP})

list(APPEND SM_DATA_COURSE_SRC
  "${SM_SRC_DATA_DIR}/Course.cpp"
  "${SM_SRC_DATA_DIR}/CourseLoaderCRS.cpp"
  "${SM_SRC_DATA_DIR}/CourseUtil.cpp"
  "${SM_SRC_DATA_DIR}/CourseWriterCRS.cpp"
  "${SM_SRC_DATA_DIR}/Trail.cpp"
  "${SM_SRC_DATA_DIR}/TrailUtil.cpp"
)

list(APPEND SM_DATA_COURSE_HPP
  "${SM_SRC_DATA_DIR}/Course.h"
  "${SM_SRC_DATA_DIR}/CourseLoaderCRS.h"
  "${SM_SRC_DATA_DIR}/CourseUtil.h"
  "${SM_SRC_DATA_DIR}/CourseWriterCRS.h"
  "${SM_SRC_DATA_DIR}/Trail.h"
  "${SM_SRC_DATA_DIR}/TrailUtil.h"
)

source_group("Data Structures\\\\Courses and Trails" FILES ${SM_DATA_COURSE_SRC} ${SM_DATA_COURSE_HPP})

list(APPEND SM_DATA_NOTEDATA_SRC
  "${SM_SRC_DATA_DIR}/NoteData.cpp"
  "${SM_SRC_DATA_DIR}/NoteDataUtil.cpp"
  "${SM_SRC_DATA_DIR}/NoteDataWithScoring.cpp"
)

list(APPEND SM_DATA_NOTEDATA_HPP
  "${SM_SRC_DATA_DIR}/NoteData.h"
  "${SM_SRC_DATA_DIR}/NoteDataUtil.h"
  "${SM_SRC_DATA_DIR}/NoteDataWithScoring.h"
)

source_group("Data Structures\\\\Note Data" FILES ${SM_DATA_NOTEDATA_SRC} ${SM_DATA_NOTEDATA_HPP})

list(APPEND SM_DATA_NOTELOAD_SRC
  "${SM_SRC_DATA_DIR}/NotesLoader.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderBMS.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderDWI.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderJson.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderKSF.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderSM.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderSMA.cpp"
  "${SM_SRC_DATA_DIR}/NotesLoaderSSC.cpp"
)

list(APPEND SM_DATA_NOTELOAD_HPP
  "${SM_SRC_DATA_DIR}/NotesLoader.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderBMS.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderDWI.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderJson.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderKSF.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderSM.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderSMA.h"
  "${SM_SRC_DATA_DIR}/NotesLoaderSSC.h"
)

source_group("Data Structures\\\\Notes Loaders" FILES ${SM_DATA_NOTELOAD_SRC} ${SM_DATA_NOTELOAD_HPP})

list(APPEND SM_DATA_NOTEWRITE_SRC
  "${SM_SRC_DATA_DIR}/NotesWriterDWI.cpp"
  "${SM_SRC_DATA_DIR}/NotesWriterJson.cpp"
  "${SM_SRC_DATA_DIR}/NotesWriterSM.cpp"
  "${SM_SRC_DATA_DIR}/NotesWriterSSC.cpp"
)

list(APPEND SM_DATA_NOTEWRITE_HPP
  "${SM_SRC_DATA_DIR}/NotesWriterDWI.h"
  "${SM_SRC_DATA_DIR}/NotesWriterJson.h"
  "${SM_SRC_DATA_DIR}/NotesWriterSM.h"
  "${SM_SRC_DATA_DIR}/NotesWriterSSC.h"
)

source_group("Data Structures\\\\Notes Writers" FILES ${SM_DATA_NOTEWRITE_SRC} ${SM_DATA_NOTEWRITE_HPP})

list(APPEND SM_DATA_SCORE_SRC
  "${SM_SRC_DATA_DIR}/ScoreKeeper.cpp"
  "${SM_SRC_DATA_DIR}/ScoreKeeperNormal.cpp"
  "${SM_SRC_DATA_DIR}/ScoreKeeperRave.cpp"
  "${SM_SRC_DATA_DIR}/ScoreKeeperShared.cpp"
)

list(APPEND SM_DATA_SCORE_HPP
  "${SM_SRC_DATA_DIR}/ScoreKeeper.h"
  "${SM_SRC_DATA_DIR}/ScoreKeeperNormal.h"
  "${SM_SRC_DATA_DIR}/ScoreKeeperRave.h"
  "${SM_SRC_DATA_DIR}/ScoreKeeperShared.h"
)

source_group("Data Structures\\\\Score Keepers" FILES ${SM_DATA_SCORE_SRC} ${SM_DATA_SCORE_HPP})

list(APPEND SM_DATA_SONG_SRC
  "${SM_SRC_DATA_DIR}/Song.cpp"
  "${SM_SRC_DATA_DIR}/SongCacheIndex.cpp"
  "${SM_SRC_DATA_DIR}/SongOptions.cpp"
  "${SM_SRC_DATA_DIR}/SongPosition.cpp"
  "${SM_SRC_DATA_DIR}/SongUtil.cpp"
)

list(APPEND SM_DATA_SONG_HPP
  "${SM_SRC_DATA_DIR}/Song.h"
  "${SM_SRC_DATA_DIR}/SongCacheIndex.h"
  "${SM_SRC_DATA_DIR}/SongOptions.h"
  "${SM_SRC_DATA_DIR}/SongPosition.h"
  "${SM_SRC_DATA_DIR}/SongUtil.h"
)

source_group("Data Structures\\\\Songs" FILES ${SM_DATA_SONG_SRC} ${SM_DATA_SONG_HPP})

list(APPEND SM_DATA_STEPS_SRC
  "${SM_SRC_DATA_DIR}/Steps.cpp"
  "${SM_SRC_DATA_DIR}/StepsUtil.cpp"
  "${SM_SRC_DATA_DIR}/Style.cpp"
  "${SM_SRC_DATA_DIR}/StyleUtil.cpp"
)

list(APPEND SM_DATA_STEPS_HPP
  "${SM_SRC_DATA_DIR}/Steps.h"
  "${SM_SRC_DATA_DIR}/StepsUtil.h"
  "${SM_SRC_DATA_DIR}/Style.h"
  "${SM_SRC_DATA_DIR}/StyleUtil.h"
)

source_group("Data Structures\\\\Steps and Styles" FILES ${SM_DATA_STEPS_SRC} ${SM_DATA_STEPS_HPP})

list(APPEND SM_DATA_REST_SRC
  "${SM_SRC_DATA_DIR}/AdjustSync.cpp"
  "${SM_SRC_DATA_DIR}/Attack.cpp"
  "${SM_SRC_DATA_DIR}/AutoKeysounds.cpp"
  "${SM_SRC_DATA_DIR}/BackgroundUtil.cpp"
  "${SM_SRC_DATA_DIR}/BannerCache.cpp"
  "${SM_SRC_DATA_DIR}/Character.cpp"
  "${SM_SRC_DATA_DIR}/CodeDetector.cpp"
  "${SM_SRC_DATA_DIR}/CodeSet.cpp"
  "${SM_SRC_DATA_DIR}/CubicSpline.cpp"
  "${SM_SRC_DATA_DIR}/Command.cpp"
  "${SM_SRC_DATA_DIR}/CommonMetrics.cpp"
  "${SM_SRC_DATA_DIR}/CreateZip.cpp"
  "${SM_SRC_DATA_DIR}/CryptHelpers.cpp"
  "${SM_SRC_DATA_DIR}/DateTime.cpp"
  "${SM_SRC_DATA_DIR}/Difficulty.cpp"
  "${SM_SRC_DATA_DIR}/EnumHelper.cpp"
  "${SM_SRC_DATA_DIR}/FileDownload.cpp"
  "${SM_SRC_DATA_DIR}/Game.cpp"
  "${SM_SRC_DATA_DIR}/GameCommand.cpp"
  "${SM_SRC_DATA_DIR}/GameConstantsAndTypes.cpp"
  "${SM_SRC_DATA_DIR}/GameInput.cpp"
  "${SM_SRC_DATA_DIR}/GameplayAssist.cpp"
  "${SM_SRC_DATA_DIR}/GamePreferences.cpp"
  "${SM_SRC_DATA_DIR}/Grade.cpp"
  "${SM_SRC_DATA_DIR}/HighScore.cpp"
  "${SM_SRC_DATA_DIR}/JsonUtil.cpp"
  "${SM_SRC_DATA_DIR}/LocalizedString.cpp"
  "${SM_SRC_DATA_DIR}/LyricsLoader.cpp"
  "${SM_SRC_DATA_DIR}/ModsGroup.cpp"
  "${SM_SRC_DATA_DIR}/NoteTypes.cpp"
  "${SM_SRC_DATA_DIR}/OptionRowHandler.cpp"
  "${SM_SRC_DATA_DIR}/PlayerAI.cpp"
  "${SM_SRC_DATA_DIR}/PlayerNumber.cpp"
  "${SM_SRC_DATA_DIR}/PlayerOptions.cpp"
  "${SM_SRC_DATA_DIR}/PlayerStageStats.cpp"
  "${SM_SRC_DATA_DIR}/PlayerState.cpp"
  "${SM_SRC_DATA_DIR}/Preference.cpp"
  "${SM_SRC_DATA_DIR}/Profile.cpp"
  "${SM_SRC_DATA_DIR}/RadarValues.cpp"
  "${SM_SRC_DATA_DIR}/RandomSample.cpp"
  "${SM_SRC_DATA_DIR}/SampleHistory.cpp"
  "${SM_SRC_DATA_DIR}/ScreenDimensions.cpp"
  "${SM_SRC_DATA_DIR}/SoundEffectControl.cpp"
  "${SM_SRC_DATA_DIR}/StageStats.cpp"
  "${SM_SRC_DATA_DIR}/TimingData.cpp"
  "${SM_SRC_DATA_DIR}/TimingSegments.cpp"
  "${SM_SRC_DATA_DIR}/TitleSubstitution.cpp"
)

list(APPEND SM_DATA_REST_HPP
  "${SM_SRC_DATA_DIR}/AdjustSync.h"
  "${SM_SRC_DATA_DIR}/Attack.h"
  "${SM_SRC_DATA_DIR}/AutoKeysounds.h"
  "${SM_SRC_DATA_DIR}/BackgroundUtil.h"
  "${SM_SRC_DATA_DIR}/BannerCache.h"
  "${SM_SRC_DATA_DIR}/Character.h"
  "${SM_SRC_DATA_DIR}/CodeDetector.h"
  "${SM_SRC_DATA_DIR}/CodeSet.h"
  "${SM_SRC_DATA_DIR}/Command.h"
  "${SM_SRC_DATA_DIR}/CommonMetrics.h"
  "${SM_SRC_DATA_DIR}/CreateZip.h"
  "${SM_SRC_DATA_DIR}/CryptHelpers.h"
  "${SM_SRC_DATA_DIR}/CubicSpline.h"
  "${SM_SRC_DATA_DIR}/DateTime.h"
  "${SM_SRC_DATA_DIR}/DisplayResolutions.h"
  "${SM_SRC_DATA_DIR}/Difficulty.h"
  "${SM_SRC_DATA_DIR}/EnumHelper.h"
  "${SM_SRC_DATA_DIR}/FileDownload.h"
  "${SM_SRC_DATA_DIR}/Foreach.h"
  "${SM_SRC_DATA_DIR}/Game.h"
  "${SM_SRC_DATA_DIR}/GameCommand.h"
  "${SM_SRC_DATA_DIR}/GameConstantsAndTypes.h"
  "${SM_SRC_DATA_DIR}/GameInput.h"
  "${SM_SRC_DATA_DIR}/GameplayAssist.h"
  "${SM_SRC_DATA_DIR}/GamePreferences.h"
  "${SM_SRC_DATA_DIR}/Grade.h"
  "${SM_SRC_DATA_DIR}/HighScore.h"
  "${SM_SRC_DATA_DIR}/InputEventPlus.h"
  "${SM_SRC_DATA_DIR}/JsonUtil.h"
  "${SM_SRC_DATA_DIR}/LocalizedString.h"
  "${SM_SRC_DATA_DIR}/LyricsLoader.h"
  "${SM_SRC_DATA_DIR}/ModsGroup.h"
  "${SM_SRC_DATA_DIR}/NoteTypes.h"
  "${SM_SRC_DATA_DIR}/OptionRowHandler.h"
  "${SM_SRC_DATA_DIR}/OptionsBinding.h"
  "${SM_SRC_DATA_DIR}/PlayerAI.h"
  "${SM_SRC_DATA_DIR}/PlayerNumber.h"
  "${SM_SRC_DATA_DIR}/PlayerOptions.h"
  "${SM_SRC_DATA_DIR}/PlayerStageStats.h"
  "${SM_SRC_DATA_DIR}/PlayerState.h"
  "${SM_SRC_DATA_DIR}/Preference.h"
  "${SM_SRC_DATA_DIR}/Profile.h"
  "${SM_SRC_DATA_DIR}/RadarValues.h"
  "${SM_SRC_DATA_DIR}/RandomSample.h"
  "${SM_SRC_DATA_DIR}/SampleHistory.h"
  "${SM_SRC_DATA_DIR}/ScreenDimensions.h"
  "${SM_SRC_DATA_DIR}/SoundEffectControl.h"
  "${SM_SRC_DATA_DIR}/SubscriptionManager.h"
  "${SM_SRC_DATA_DIR}/StageStats.h"
  "${SM_SRC_DATA_DIR}/ThemeMetric.h"
  "${SM_SRC_DATA_DIR}/TimingData.h"
  "${SM_SRC_DATA_DIR}/TimingSegments.h"
  "${SM_SRC_DATA_DIR}/TitleSubstitution.h"
)

if(WITH_NETWORKING)
  list(APPEND SM_DATA_REST_SRC
    "${SM_SRC_DATA_DIR}/RoomWheel.cpp"
  )
  list(APPEND SM_DATA_REST_HPP
    "${SM_SRC_DATA_DIR}/RoomWheel.h"
  )
endif()

source_group("Data Structures\\\\Misc Objects" FILES ${SM_DATA_REST_SRC} ${SM_DATA_REST_HPP})

list(APPEND SMDATA_ALL_DATA_SRC
  ${SM_DATA_COURSE_SRC}
  ${SM_DATA_FONT_SRC}
  ${SM_DATA_LUA_SRC}
  ${SM_DATA_NOTEDATA_SRC}
  ${SM_DATA_NOTELOAD_SRC}
  ${SM_DATA_NOTEWRITE_SRC}
  ${SM_DATA_SCORE_SRC}
  ${SM_DATA_SONG_SRC}
  ${SM_DATA_STEPS_SRC}
  ${SM_DATA_REST_SRC}
)

list(APPEND SMDATA_ALL_DATA_HPP
  ${SM_DATA_COURSE_HPP}
  ${SM_DATA_FONT_HPP}
  ${SM_DATA_LUA_HPP}
  ${SM_DATA_NOTEDATA_HPP}
  ${SM_DATA_NOTELOAD_HPP}
  ${SM_DATA_NOTEWRITE_HPP}
  ${SM_DATA_SCORE_HPP}
  ${SM_DATA_SONG_HPP}
  ${SM_DATA_STEPS_HPP}
  ${SM_DATA_REST_HPP}
)
