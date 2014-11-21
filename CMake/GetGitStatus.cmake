execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
                WORKING_DIRECTORY ${GIT_STATUS_WORKING_DIR}
                RESULT_VARIABLE _git_status_result
                OUTPUT_VARIABLE GIT_SHA1
                OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(COMMAND "${GIT_EXECUTABLE}" status -z --porcelain --untracked-files=normal
                WORKING_DIRECTORY ${GIT_STATUS_WORKING_DIR}
                RESULT_VARIABLE _git_status_result
                OUTPUT_VARIABLE GIT_STATUS
                OUTPUT_STRIP_TRAILING_WHITESPACE)

configure_file(${GIT_STATUS_INPUT_FILE} ${GIT_STATUS_OUTPUT_FILE} @ONLY)
