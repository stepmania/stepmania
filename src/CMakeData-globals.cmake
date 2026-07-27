list(APPEND SMDATA_GLOBAL_FILES_SRC
            "GameLoop.cpp"
            "global.cpp"
            "SpecialFiles.cpp"
            "StdString.cpp"
            "StepMania.cpp" # TODO: Refactor into separate main project.
            "${SM_GENERATED_SRC_DIR}/verstub.cpp")

list(APPEND SMDATA_GLOBAL_FILES_HPP
            "${SM_GENERATED_SRC_DIR}/config.hpp"
            "GameLoop.h"
            "global.h"
            "ProductInfo.h" # TODO: Have this be auto-generated.
            "SpecialFiles.h"
            "StdString.h" # TODO: Remove the need for this file, transition to
                          # std::string.
            "StepMania.h" # TODO: Refactor into separate main project.
     )

source_group("Global Files"
             FILES
             ${SMDATA_GLOBAL_FILES_SRC}
             ${SMDATA_GLOBAL_FILES_HPP})
