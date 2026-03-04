file(GLOB_RECURSE RAZE_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/main.c"

	"${CMAKE_CURRENT_SOURCE_DIR}/core/logger.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/core/logger.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/core/buffer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/core/buffer.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/core/utility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/core/utility.c"

	"${CMAKE_CURRENT_SOURCE_DIR}/server/server.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/server/server.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/server/connection.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/server/connection.c"

	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_protocol.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_protocol.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_utility.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_utility.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_parser.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_request.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_request.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_response.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_response.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_router.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/http/http_router.c"
)

add_executable(${RAZE_TARGET_NAME} ${RAZE_SOURCES})

set_compile_options_for_target(${RAZE_TARGET_NAME})

target_include_directories(${RAZE_TARGET_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../")
# target_link_libraries(${RAZE_TARGET_NAME} PRIVATE)

function(CopyLibToTarget LIB_NAME TARGET_NAME)
	add_custom_command(TARGET ${TARGET_NAME}
		POST_BUILD
		COMMAND
		${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:${LIB_NAME}>
		$<TARGET_FILE_DIR:${TARGET_NAME}>
	)
endfunction()
