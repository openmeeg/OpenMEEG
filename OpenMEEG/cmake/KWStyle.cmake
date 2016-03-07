#-----------------------------------------------
# Code Syntax Checking with KWStyle
#-----------------------------------------------

option(CHECK_SYNTAX "Checking code syntax using KWStyle" OFF)
mark_as_advanced(CHECK_SYNTAX)

if (CHECK_SYNTAX)
    add_custom_target(check_syntax
        COMMAND KWStyle -xml ${PROJECT_SOURCE_DIR}/OpenMEEGConfig.kws.xml -html ${PROJECT_BINARY_DIR}/KWStyleCheck -D KWStyleFilesToCheck.txt -v
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

