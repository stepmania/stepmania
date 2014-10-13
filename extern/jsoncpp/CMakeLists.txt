if(MSVC)
	add_definitions(/D _CRT_SECURE_NO_WARNINGS)
#   Uncomment the one below if it ever gets fixed.
#	VS_GLOBAL_ROOTNAMESPACE("Json")
endif(MSVC)

list(APPEND JSON_SRC
	"src/lib_json/json_reader.cpp"
	"src/lib_json/json_value.cpp"
	"src/lib_json/json_writer.cpp"
)

list(APPEND JSON_HPP
	"include/json/config.h"
	"include/json/features.h"
	"include/json/forwards.h"
	"include/json/json.h"
	"include/json/reader.h"
	"include/json/value.h"
	"include/json/writer.h"
)

include_directories("include")

add_library("jsoncpp" ${JSON_SRC} ${JSON_HPP})

set_property(TARGET "jsoncpp" PROPERTY FOLDER "External Libraries")