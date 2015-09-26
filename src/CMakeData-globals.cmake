list(APPEND SMDATA_GLOBAL_FILES_SRC
  "${SM_SRC_GLOBALS_DIR}/GameLoop.cpp"
  "${SM_SRC_GLOBALS_DIR}/global.cpp"
  "${SM_SRC_GLOBALS_DIR}/SpecialFiles.cpp"
  "${SM_SRC_GLOBALS_DIR}/StepMania.cpp" # TODO: Refactor into separate main project.
)

list(APPEND SMDATA_GLOBAL_FILES_HPP
  "${SM_SRC_GENERATED_DIR}/config.hpp"
  "${SM_SRC_GLOBALS_DIR}/GameLoop.h"
  "${SM_SRC_GLOBALS_DIR}/global.h"
  "${SM_SRC_GLOBALS_DIR}/ProductInfo.h" # TODO: Have this be auto-generated.
  "${SM_SRC_GLOBALS_DIR}/SpecialFiles.h"
  "${SM_SRC_GLOBALS_DIR}/StdString.h" # TODO: Remove the need for this file, transition to std::string.
  "${SM_SRC_GLOBALS_DIR}/StepMania.h" # TODO: Refactor into separate main project.
)

source_group("Global Files" FILES ${SMDATA_GLOBAL_FILES_SRC} ${SMDATA_GLOBAL_FILES_HPP})
