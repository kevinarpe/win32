#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status.
set -e
# Treat unset variables as an error when substituting.
set -u
# The return value of a pipeline is the status of
# the last command to exit with a non-zero status,
# or zero if no command exited with a non-zero status
set -o pipefail
# Print commands and their arguments as they are executed.
# set -x

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$0")"

    echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    echo_and_run_gcc_cmd \
        -c -o error_exit.o error_exit.c

    echo_and_run_gcc_cmd \
        -c -o win32_xmalloc.o win32_xmalloc.c

    echo_and_run_gcc_cmd \
        -c -o wstr.o wstr.c

    echo_and_run_gcc_cmd \
        -c -o config.o config.c

    echo_and_run_gcc_cmd \
        -c -o main.o main.c

    echo_and_run_gcc_cmd \
        -o send_input.exe error_exit.o win32_xmalloc.o wstr.o config.o main.o -lgdi32

    echo_and_run_cmd \
        ls -l send_input.exe

    echo_and_run_cmd \
        cd -

    printf -- '\n'
    printf -- 'Try the new executable:\n'
    printf -- '$ wine send_input.exe release/sample_config.txt\n'
    printf -- '\n'
}

echo_cmd()
{
    echo
    echo '$' "$@"
}

echo_and_run_cmd()
{
    echo_cmd "$@"
    "$@"
}

echo_and_run_gcc_cmd()
{
    # Ref: https://stackoverflow.com/a/62488988/257299
    echo_and_run_cmd \
        x86_64-w64-mingw32-gcc \
            -g3 -ggdb3 \
            -Werror \
            -Wl,-subsystem,windows \
            -municode \
            "$@"
}

main "$@"

