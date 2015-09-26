list(APPEND SMDATA_SCREEN_GAMEPLAY_SRC
  "${SM_SRC_SCREEN_DIR}/ScreenGameplay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplayLesson.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplayNormal.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplayShared.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplaySyncMachine.cpp"
)

list(APPEND SMDATA_SCREEN_GAMEPLAY_HPP
  "${SM_SRC_SCREEN_DIR}/ScreenGameplay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplayNormal.h"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplayShared.h"
  "${SM_SRC_SCREEN_DIR}/ScreenGameplaySyncMachine.h"
)

source_group("Screens\\\\Gameplay" FILES ${SMDATA_SCREEN_GAMEPLAY_SRC} ${SMDATA_SCREEN_GAMEPLAY_HPP})

list(APPEND SMDATA_SCREEN_OPTION_SRC
  "${SM_SRC_SCREEN_DIR}/ScreenOptions.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsCourseOverview.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsEditCourse.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsEditProfile.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsExportPackage.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageCourses.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageEditSteps.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageProfiles.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMaster.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMasterPrefs.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMemoryCard.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsToggleSongs.cpp"
)
list(APPEND SMDATA_SCREEN_OPTION_HPP
  "${SM_SRC_SCREEN_DIR}/ScreenOptions.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsCourseOverview.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsEditCourse.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsEditProfile.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsExportPackage.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageCourses.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageEditSteps.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsManageProfiles.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMaster.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMasterPrefs.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsMemoryCard.h"
  "${SM_SRC_SCREEN_DIR}/ScreenOptionsToggleSongs.h"
)

source_group("Screens\\\\Options" FILES ${SMDATA_SCREEN_OPTION_SRC} ${SMDATA_SCREEN_OPTION_HPP})

list(APPEND SMDATA_SCREEN_REST_SRC
  "${SM_SRC_SCREEN_DIR}/Screen.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenAttract.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenBookkeeping.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenContinue.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenDebugOverlay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenDemonstration.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenEdit.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenEditMenu.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenEnding.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenEvaluation.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenExit.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenHighScores.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenHowToPlay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenInstallOverlay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenInstructions.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenJukebox.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenMapControllers.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenMessage.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenMiniMenu.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNameEntry.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNameEntryTraditional.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenPackages.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenPlayerOptions.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenProfileLoad.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenProfileSave.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenPrompt.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenRanking.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenReloadSongs.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSandbox.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSaveSync.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelect.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectCharacter.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectLanguage.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectMaster.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectMusic.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectProfile.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenServiceAction.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSetTime.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSongOptions.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSplash.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenStatsOverlay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSyncOverlay.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenSystemLayer.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenTestInput.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenTestLights.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenTestSound.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenTextEntry.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenTitleMenu.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockBrowse.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockCelebrate.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockStatus.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenWithMenuElements.cpp"
)
list(APPEND SMDATA_SCREEN_REST_HPP
  "${SM_SRC_SCREEN_DIR}/Screen.h"
  "${SM_SRC_SCREEN_DIR}/ScreenAttract.h"
  "${SM_SRC_SCREEN_DIR}/ScreenBookkeeping.h"
  "${SM_SRC_SCREEN_DIR}/ScreenContinue.h"
  "${SM_SRC_SCREEN_DIR}/ScreenDebugOverlay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenDemonstration.h"
  "${SM_SRC_SCREEN_DIR}/ScreenEdit.h"
  "${SM_SRC_SCREEN_DIR}/ScreenEditMenu.h"
  "${SM_SRC_SCREEN_DIR}/ScreenEnding.h"
  "${SM_SRC_SCREEN_DIR}/ScreenEvaluation.h"
  "${SM_SRC_SCREEN_DIR}/ScreenExit.h"
  "${SM_SRC_SCREEN_DIR}/ScreenHighScores.h"
  "${SM_SRC_SCREEN_DIR}/ScreenHowToPlay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenInstallOverlay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenInstructions.h"
  "${SM_SRC_SCREEN_DIR}/ScreenJukebox.h"
  "${SM_SRC_SCREEN_DIR}/ScreenMapControllers.h"
  "${SM_SRC_SCREEN_DIR}/ScreenMessage.h"
  "${SM_SRC_SCREEN_DIR}/ScreenMiniMenu.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNameEntry.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNameEntryTraditional.h"
  "${SM_SRC_SCREEN_DIR}/ScreenPackages.h"
  "${SM_SRC_SCREEN_DIR}/ScreenPlayerOptions.h"
  "${SM_SRC_SCREEN_DIR}/ScreenProfileLoad.h"
  "${SM_SRC_SCREEN_DIR}/ScreenProfileSave.h"
  "${SM_SRC_SCREEN_DIR}/ScreenPrompt.h"
  "${SM_SRC_SCREEN_DIR}/ScreenRanking.h"
  "${SM_SRC_SCREEN_DIR}/ScreenReloadSongs.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSandbox.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSaveSync.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelect.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectCharacter.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectLanguage.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectMaster.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectMusic.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSelectProfile.h"
  "${SM_SRC_SCREEN_DIR}/ScreenServiceAction.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSetTime.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSongOptions.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSplash.h"
  "${SM_SRC_SCREEN_DIR}/ScreenStatsOverlay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSyncOverlay.h"
  "${SM_SRC_SCREEN_DIR}/ScreenSystemLayer.h"
  "${SM_SRC_SCREEN_DIR}/ScreenTestInput.h"
  "${SM_SRC_SCREEN_DIR}/ScreenTestLights.h"
  "${SM_SRC_SCREEN_DIR}/ScreenTestSound.h"
  "${SM_SRC_SCREEN_DIR}/ScreenTextEntry.h"
  "${SM_SRC_SCREEN_DIR}/ScreenTitleMenu.h"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockBrowse.h"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockCelebrate.h"
  "${SM_SRC_SCREEN_DIR}/ScreenUnlockStatus.h"
  "${SM_SRC_SCREEN_DIR}/ScreenWithMenuElements.h"
)

source_group("Screens\\\\Others" FILES ${SMDATA_SCREEN_REST_SRC} ${SMDATA_SCREEN_REST_HPP})

list(APPEND SMDATA_SCREEN_NET_SRC
  "${SM_SRC_SCREEN_DIR}/ScreenNetEvaluation.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNetRoom.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNetSelectBase.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNetSelectMusic.cpp"
  "${SM_SRC_SCREEN_DIR}/ScreenNetworkOptions.cpp"
)

list(APPEND SMDATA_SCREEN_NET_HPP
  "${SM_SRC_SCREEN_DIR}/ScreenNetEvaluation.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNetRoom.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNetSelectBase.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNetSelectMusic.h"
  "${SM_SRC_SCREEN_DIR}/ScreenNetworkOptions.h"
)

if (WITH_NETWORKING)
  list(APPEND SMDATA_SCREEN_NET_SRC
    "${SM_SRC_SCREEN_DIR}/ScreenSMOnlineLogin.cpp"
  )
  list(APPEND SMDATA_SCREEN_NET_HPP
    "${SM_SRC_SCREEN_DIR}/ScreenSMOnlineLogin.h"
  )
endif()

source_group("Screens\\\\Network" FILES ${SMDATA_SCREEN_NET_SRC} ${SMDATA_SCREEN_NET_HPP})

list(APPEND SMDATA_ALL_SCREENS_SRC
  ${SMDATA_SCREEN_GAMEPLAY_SRC}
  ${SMDATA_SCREEN_OPTION_SRC}
  ${SMDATA_SCREEN_NET_SRC}
  ${SMDATA_SCREEN_REST_SRC}
)

list(APPEND SMDATA_ALL_SCREENS_HPP
  ${SMDATA_SCREEN_GAMEPLAY_HPP}
  ${SMDATA_SCREEN_OPTION_HPP}
  ${SMDATA_SCREEN_NET_HPP}
  ${SMDATA_SCREEN_REST_HPP}
)
