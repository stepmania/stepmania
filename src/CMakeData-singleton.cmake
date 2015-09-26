list(APPEND SMDATA_GLOBAL_SINGLETON_SRC
  "${SM_SRC_SINGLETON_DIR}/AnnouncerManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/Bookkeeper.cpp"
  "${SM_SRC_SINGLETON_DIR}/CharacterManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/CommandLineActions.cpp"
  "${SM_SRC_SINGLETON_DIR}/CryptManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/FontManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/GameManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/GameSoundManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/GameState.cpp"
  "${SM_SRC_SINGLETON_DIR}/InputFilter.cpp"
  "${SM_SRC_SINGLETON_DIR}/InputMapper.cpp"
  "${SM_SRC_SINGLETON_DIR}/InputQueue.cpp"
  "${SM_SRC_SINGLETON_DIR}/LightsManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/LuaManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/MemoryCardManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/MessageManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/NetworkSyncManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/NoteSkinManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/PrefsManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/ProfileManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/ScreenManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/SongManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/StatsManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/ThemeManager.cpp"
  "${SM_SRC_SINGLETON_DIR}/UnlockManager.cpp"
)
list(APPEND SMDATA_GLOBAL_SINGLETON_HPP
  "${SM_SRC_SINGLETON_DIR}/AnnouncerManager.h"
  "${SM_SRC_SINGLETON_DIR}/Bookkeeper.h"
  "${SM_SRC_SINGLETON_DIR}/CharacterManager.h"
  "${SM_SRC_SINGLETON_DIR}/CommandLineActions.h"
  "${SM_SRC_SINGLETON_DIR}/CryptManager.h"
  "${SM_SRC_SINGLETON_DIR}/FontManager.h"
  "${SM_SRC_SINGLETON_DIR}/GameManager.h"
  "${SM_SRC_SINGLETON_DIR}/GameSoundManager.h"
  "${SM_SRC_SINGLETON_DIR}/GameState.h"
  "${SM_SRC_SINGLETON_DIR}/InputFilter.h"
  "${SM_SRC_SINGLETON_DIR}/InputMapper.h"
  "${SM_SRC_SINGLETON_DIR}/InputQueue.h"
  "${SM_SRC_SINGLETON_DIR}/LightsManager.h"
  "${SM_SRC_SINGLETON_DIR}/LuaManager.h"
  "${SM_SRC_SINGLETON_DIR}/MemoryCardManager.h"
  "${SM_SRC_SINGLETON_DIR}/MessageManager.h"
  "${SM_SRC_SINGLETON_DIR}/NetworkSyncManager.h"
  "${SM_SRC_SINGLETON_DIR}/NoteSkinManager.h"
  "${SM_SRC_SINGLETON_DIR}/PrefsManager.h"
  "${SM_SRC_SINGLETON_DIR}/ProfileManager.h"
  "${SM_SRC_SINGLETON_DIR}/ScreenManager.h"
  "${SM_SRC_SINGLETON_DIR}/SongManager.h"
  "${SM_SRC_SINGLETON_DIR}/StatsManager.h"
  "${SM_SRC_SINGLETON_DIR}/ThemeManager.h"
  "${SM_SRC_SINGLETON_DIR}/UnlockManager.h"
)

if(WITH_NETWORKING)
  list(APPEND SMDATA_GLOBAL_SINGLETON_SRC
    "${SM_SRC_SINGLETON_DIR}/ezsockets.cpp"
  )
  list(APPEND SMDATA_GLOBAL_SINGLETON_HPP
    "${SM_SRC_SINGLETON_DIR}/ezsockets.h"
  )
endif()

source_group("Global Singletons" FILES ${SMDATA_GLOBAL_SINGLETON_SRC} ${SMDATA_GLOBAL_SINGLETON_HPP})
