list(APPEND SMDATA_ACTOR_BASE_SRC
  "${SM_SRC_ACTOR_DIR}/Actor.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorFrame.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorFrameTexture.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorMultiTexture.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorMultiVertex.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorProxy.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorScroller.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorSound.cpp"
  "${SM_SRC_ACTOR_DIR}/ActorUtil.cpp"
  "${SM_SRC_ACTOR_DIR}/AutoActor.cpp"
  "${SM_SRC_ACTOR_DIR}/BitmapText.cpp"
  "${SM_SRC_ACTOR_DIR}/DynamicActorScroller.cpp"
  "${SM_SRC_ACTOR_DIR}/Model.cpp"
  "${SM_SRC_ACTOR_DIR}/ModelManager.cpp"
  "${SM_SRC_ACTOR_DIR}/ModelTypes.cpp"
  "${SM_SRC_ACTOR_DIR}/Quad.cpp"
  "${SM_SRC_ACTOR_DIR}/RollingNumbers.cpp"
  "${SM_SRC_ACTOR_DIR}/Sprite.cpp"
  "${SM_SRC_ACTOR_DIR}/Tween.cpp"
)
list(APPEND SMDATA_ACTOR_BASE_HPP
  "${SM_SRC_ACTOR_DIR}/Actor.h"
  "${SM_SRC_ACTOR_DIR}/ActorFrame.h"
  "${SM_SRC_ACTOR_DIR}/ActorFrameTexture.h"
  "${SM_SRC_ACTOR_DIR}/ActorMultiTexture.h"
  "${SM_SRC_ACTOR_DIR}/ActorMultiVertex.h"
  "${SM_SRC_ACTOR_DIR}/ActorProxy.h"
  "${SM_SRC_ACTOR_DIR}/ActorScroller.h"
  "${SM_SRC_ACTOR_DIR}/ActorSound.h"
  "${SM_SRC_ACTOR_DIR}/ActorUtil.h"
  "${SM_SRC_ACTOR_DIR}/AutoActor.h"
  "${SM_SRC_ACTOR_DIR}/BitmapText.h"
  "${SM_SRC_ACTOR_DIR}/DynamicActorScroller.h"
  "${SM_SRC_ACTOR_DIR}/Model.h"
  "${SM_SRC_ACTOR_DIR}/ModelManager.h"
  "${SM_SRC_ACTOR_DIR}/ModelTypes.h"
  "${SM_SRC_ACTOR_DIR}/Quad.h"
  "${SM_SRC_ACTOR_DIR}/RollingNumbers.h"
  "${SM_SRC_ACTOR_DIR}/Sprite.h"
  "${SM_SRC_ACTOR_DIR}/Tween.h"
)

source_group("Actors\\\\Base" FILES ${SMDATA_ACTOR_BASE_SRC} ${SMDATA_ACTOR_BASE_HPP})

list(APPEND SMDATA_ACTOR_GAMEPLAY_SRC
  "${SM_SRC_ACTOR_DIR}/ActiveAttackList.cpp"
  "${SM_SRC_ACTOR_DIR}/ArrowEffects.cpp"
  "${SM_SRC_ACTOR_DIR}/AttackDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/Background.cpp"
  "${SM_SRC_ACTOR_DIR}/BeginnerHelper.cpp"
  "${SM_SRC_ACTOR_DIR}/CombinedLifeMeterTug.cpp"
  "${SM_SRC_ACTOR_DIR}/DancingCharacters.cpp"
  "${SM_SRC_ACTOR_DIR}/Foreground.cpp"
  "${SM_SRC_ACTOR_DIR}/GhostArrowRow.cpp"
  "${SM_SRC_ACTOR_DIR}/HoldJudgment.cpp"
  "${SM_SRC_ACTOR_DIR}/Inventory.cpp"
  "${SM_SRC_ACTOR_DIR}/LifeMeter.cpp"
  "${SM_SRC_ACTOR_DIR}/LifeMeterBar.cpp"
  "${SM_SRC_ACTOR_DIR}/LifeMeterBattery.cpp"
  "${SM_SRC_ACTOR_DIR}/LifeMeterTime.cpp"
  "${SM_SRC_ACTOR_DIR}/LyricDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/NoteDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/NoteField.cpp"
  "${SM_SRC_ACTOR_DIR}/PercentageDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/Player.cpp"
  "${SM_SRC_ACTOR_DIR}/ReceptorArrow.cpp"
  "${SM_SRC_ACTOR_DIR}/ReceptorArrowRow.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayAliveTime.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayBattle.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayCalories.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayLifeTime.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayNormal.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayOni.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayPercentage.cpp"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayRave.cpp"
)

list(APPEND SMDATA_ACTOR_GAMEPLAY_HPP
  "${SM_SRC_ACTOR_DIR}/ActiveAttackList.h"
  "${SM_SRC_ACTOR_DIR}/ArrowEffects.h"
  "${SM_SRC_ACTOR_DIR}/AttackDisplay.h"
  "${SM_SRC_ACTOR_DIR}/Background.h"
  "${SM_SRC_ACTOR_DIR}/BeginnerHelper.h"
  "${SM_SRC_ACTOR_DIR}/CombinedLifeMeter.h"
  "${SM_SRC_ACTOR_DIR}/CombinedLifeMeterTug.h"
  "${SM_SRC_ACTOR_DIR}/DancingCharacters.h"
  "${SM_SRC_ACTOR_DIR}/Foreground.h"
  "${SM_SRC_ACTOR_DIR}/GhostArrowRow.h"
  "${SM_SRC_ACTOR_DIR}/HoldJudgment.h"
  "${SM_SRC_ACTOR_DIR}/Inventory.h"
  "${SM_SRC_ACTOR_DIR}/LifeMeter.h"
  "${SM_SRC_ACTOR_DIR}/LifeMeterBar.h"
  "${SM_SRC_ACTOR_DIR}/LifeMeterBattery.h"
  "${SM_SRC_ACTOR_DIR}/LifeMeterTime.h"
  "${SM_SRC_ACTOR_DIR}/LyricDisplay.h"
  "${SM_SRC_ACTOR_DIR}/NoteDisplay.h"
  "${SM_SRC_ACTOR_DIR}/NoteField.h"
  "${SM_SRC_ACTOR_DIR}/PercentageDisplay.h"
  "${SM_SRC_ACTOR_DIR}/Player.h"
  "${SM_SRC_ACTOR_DIR}/ReceptorArrow.h"
  "${SM_SRC_ACTOR_DIR}/ReceptorArrowRow.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplay.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayAliveTime.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayBattle.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayCalories.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayLifeTime.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayNormal.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayOni.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayPercentage.h"
  "${SM_SRC_ACTOR_DIR}/ScoreDisplayRave.h"
)
source_group("Actors\\\\Gameplay" FILES ${SMDATA_ACTOR_GAMEPLAY_SRC} ${SMDATA_ACTOR_GAMEPLAY_HPP})

list(APPEND SMDATA_ACTOR_MENU_SRC
  "${SM_SRC_ACTOR_DIR}/BPMDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/ComboGraph.cpp"
  "${SM_SRC_ACTOR_DIR}/ControllerStateDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/CourseContentsList.cpp"
  "${SM_SRC_ACTOR_DIR}/DifficultyList.cpp"
  "${SM_SRC_ACTOR_DIR}/DualScrollBar.cpp"
  "${SM_SRC_ACTOR_DIR}/EditMenu.cpp"
  "${SM_SRC_ACTOR_DIR}/FadingBanner.cpp"
  "${SM_SRC_ACTOR_DIR}/GradeDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/GraphDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/GrooveRadar.cpp"
  "${SM_SRC_ACTOR_DIR}/HelpDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/MemoryCardDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/MenuTimer.cpp"
  "${SM_SRC_ACTOR_DIR}/ModIcon.cpp"
  "${SM_SRC_ACTOR_DIR}/ModIconRow.cpp"
  "${SM_SRC_ACTOR_DIR}/MusicWheel.cpp"
  "${SM_SRC_ACTOR_DIR}/MusicWheelItem.cpp"
  "${SM_SRC_ACTOR_DIR}/OptionRow.cpp"
  "${SM_SRC_ACTOR_DIR}/OptionsCursor.cpp"
  "${SM_SRC_ACTOR_DIR}/OptionsList.cpp"
  "${SM_SRC_ACTOR_DIR}/PaneDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/ScrollBar.cpp"
  "${SM_SRC_ACTOR_DIR}/SnapDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/TextBanner.cpp"
  "${SM_SRC_ACTOR_DIR}/WheelBase.cpp"
  "${SM_SRC_ACTOR_DIR}/WheelItemBase.cpp"
  "${SM_SRC_ACTOR_DIR}/WheelNotifyIcon.cpp"
  "${SM_SRC_ACTOR_DIR}/WorkoutGraph.cpp"
)
list(APPEND SMDATA_ACTOR_MENU_HPP
  "${SM_SRC_ACTOR_DIR}/BPMDisplay.h"
  "${SM_SRC_ACTOR_DIR}/ComboGraph.h"
  "${SM_SRC_ACTOR_DIR}/ControllerStateDisplay.h"
  "${SM_SRC_ACTOR_DIR}/CourseContentsList.h"
  "${SM_SRC_ACTOR_DIR}/DifficultyList.h"
  "${SM_SRC_ACTOR_DIR}/DualScrollBar.h"
  "${SM_SRC_ACTOR_DIR}/EditMenu.h"
  "${SM_SRC_ACTOR_DIR}/FadingBanner.h"
  "${SM_SRC_ACTOR_DIR}/GradeDisplay.h"
  "${SM_SRC_ACTOR_DIR}/GraphDisplay.h"
  "${SM_SRC_ACTOR_DIR}/GrooveRadar.h"
  "${SM_SRC_ACTOR_DIR}/HelpDisplay.h"
  "${SM_SRC_ACTOR_DIR}/MemoryCardDisplay.h"
  "${SM_SRC_ACTOR_DIR}/MenuTimer.h"
  "${SM_SRC_ACTOR_DIR}/ModIcon.h"
  "${SM_SRC_ACTOR_DIR}/ModIconRow.h"
  "${SM_SRC_ACTOR_DIR}/MusicWheel.h"
  "${SM_SRC_ACTOR_DIR}/MusicWheelItem.h"
  "${SM_SRC_ACTOR_DIR}/OptionRow.h"
  "${SM_SRC_ACTOR_DIR}/OptionsCursor.h"
  "${SM_SRC_ACTOR_DIR}/OptionsList.h"
  "${SM_SRC_ACTOR_DIR}/PaneDisplay.h"
  "${SM_SRC_ACTOR_DIR}/ScrollBar.h"
  "${SM_SRC_ACTOR_DIR}/SnapDisplay.h"
  "${SM_SRC_ACTOR_DIR}/TextBanner.h"
  "${SM_SRC_ACTOR_DIR}/WheelBase.h"
  "${SM_SRC_ACTOR_DIR}/WheelItemBase.h"
  "${SM_SRC_ACTOR_DIR}/WheelNotifyIcon.h"
  "${SM_SRC_ACTOR_DIR}/WorkoutGraph.h"
)

if(WITH_NETWORKING)
  list(APPEND SMDATA_ACTOR_MENU_SRC
    "${SM_SRC_ACTOR_DIR}/RoomInfoDisplay.cpp"
  )
  list(APPEND SMDATA_ACTOR_MENU_HPP
    "${SM_SRC_ACTOR_DIR}/RoomInfoDisplay.h"
  )
endif()

source_group("Actors\\\\Menus" FILES ${SMDATA_ACTOR_MENU_SRC} ${SMDATA_ACTOR_MENU_HPP})

list(APPEND SMDATA_ACTOR_GAMEPLAY_MENU_SRC
  "${SM_SRC_ACTOR_DIR}/Banner.cpp"
  "${SM_SRC_ACTOR_DIR}/BGAnimation.cpp"
  "${SM_SRC_ACTOR_DIR}/BGAnimationLayer.cpp"
  "${SM_SRC_ACTOR_DIR}/DifficultyIcon.cpp"
  "${SM_SRC_ACTOR_DIR}/MeterDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/StepsDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/StreamDisplay.cpp"
  "${SM_SRC_ACTOR_DIR}/Transition.cpp"
)

list(APPEND SMDATA_ACTOR_GAMEPLAY_MENU_HPP
  "${SM_SRC_ACTOR_DIR}/Banner.h"
  "${SM_SRC_ACTOR_DIR}/BGAnimation.h"
  "${SM_SRC_ACTOR_DIR}/BGAnimationLayer.h"
  "${SM_SRC_ACTOR_DIR}/DifficultyIcon.h"
  "${SM_SRC_ACTOR_DIR}/MeterDisplay.h"
  "${SM_SRC_ACTOR_DIR}/StepsDisplay.h"
  "${SM_SRC_ACTOR_DIR}/StreamDisplay.h"
  "${SM_SRC_ACTOR_DIR}/Transition.h"
)

source_group("Actors\\\\Gameplay and Menus" FILES ${SMDATA_ACTOR_GAMEPLAY_MENU_SRC} ${SMDATA_ACTOR_GAMEPLAY_MENU_HPP})

list(APPEND SMDATA_ALL_ACTORS_SRC
  ${SMDATA_ACTOR_BASE_SRC}
  ${SMDATA_ACTOR_GAMEPLAY_SRC}
  ${SMDATA_ACTOR_MENU_SRC}
  ${SMDATA_ACTOR_GAMEPLAY_MENU_SRC}
)
list(APPEND SMDATA_ALL_ACTORS_HPP
  ${SMDATA_ACTOR_BASE_HPP}
  ${SMDATA_ACTOR_GAMEPLAY_HPP}
  ${SMDATA_ACTOR_MENU_HPP}
  ${SMDATA_ACTOR_GAMEPLAY_MENU_HPP}
)
