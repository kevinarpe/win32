#!/usr/bin/env bash

COMMON_DIR_PATH='../../common'
source "$(dirname "$0")/$COMMON_DIR_PATH/bashlib"

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$0")"

    bashlib_echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    bashlib_echo_and_run_cmd \
        "$COMMON_DIR_PATH/build.bash"

    # Note: -iquote is more specific than -I
    bashlib_echo_and_run_gcc_cmd_if_necessary \
        kb_test.c kb_test.o -iquote "$COMMON_DIR_PATH"

    bashlib_echo_and_run_gcc_cmd \
        -o kb_test.exe \
        "$COMMON_DIR_PATH/log.o" \
        "$COMMON_DIR_PATH/error_exit.o" \
        kb_test.o -lgdi32

    bashlib_echo_and_run_cmd \
        ls -l kb_test.exe

    bashlib_echo_and_run_cmd \
        cd -

    bashlib_echo_and_run_cmd \
        wine "$this_script_abs_dir_path/kb_test.exe"
}

main "$@"

