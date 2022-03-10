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

TRUE=0
FALSE=1

ARG_CLEAN='--clean'
ARG_RELEASE='--release'

EXECUTABLE='send_input.exe'

GCC_PREFIX='x86_64-w64-mingw32-'

# Ex: 'x86_64-w64-mingw32-gcc'
GCC="$GCC_PREFIX"'gcc'

# Ex: 'x86_64-w64-mingw32-strip'
STRIP="$GCC_PREFIX"'strip'

is_linux()
{
    local uname
    uname="$(uname)"
    [ "Linux" = "$uname" ]
}

echo_exit_status()
{
    local exit_status
    exit_status="$("$@")"
    printf -- '%d' $?
}

IS_LINUX="$(echo_exit_status is_linux)"

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    # Intentional: By default, build debug executable.
    local is_release=$FALSE
    local is_clean=$FALSE

    local arg
    for arg in "$@"
    do
        if [ "-h" = "$arg" ] || [ "--help" = "$arg" ]
        then
            show_help_then_exit

        elif [ "$ARG_CLEAN" = "$arg" ]
        then
            if [ $FALSE = $is_clean ]
            then
                is_clean=$TRUE
            else
                printf -- '\nError: Found multiple %s arguments\n' "$ARG_CLEAN"
                show_help_then_exit
            fi

        elif [ "$ARG_RELEASE" = "$arg" ]
        then
            if [ $FALSE = $is_release ]
            then
                is_release=$TRUE
            else
                printf -- '\nError: Found multiple %s arguments\n' "$ARG_RELEASE"
                show_help_then_exit
            fi
        else
            printf -- '\nError: Unknown argument: [%s]\n' "$arg"
            show_help_then_exit
        fi
    done

    if [ $TRUE = $is_release ]
    then
        printf -- '\nRelease build\n'
    else
        printf -- '\nDebug build\n'
    fi

    echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    if [ $TRUE = $is_clean ]
    then
        echo_and_run_cmd \
            rm -f *.o
    fi

    echo_and_run_gcc_cmd_if_necessary \
        win32.c win32.o

    echo_and_run_gcc_cmd_if_necessary \
        log.c log.o

    echo_and_run_gcc_cmd_if_necessary \
        error_exit.c error_exit.o

    echo_and_run_gcc_cmd_if_necessary \
        win32_xmalloc.c win32_xmalloc.o

    echo_and_run_gcc_cmd_if_necessary \
        wstr.c wstr.o

    echo_and_run_gcc_cmd_if_necessary \
        config.c config.o

    echo_and_run_gcc_cmd_if_necessary \
        min_max.c min_max.o

    echo_and_run_gcc_cmd_if_necessary \
        console.c console.o

    echo_and_run_gcc_cmd_if_necessary \
        main.c main.o

    echo_and_run_gcc_cmd \
        -o send_input.exe win32.o log.o error_exit.o win32_xmalloc.o wstr.o config.o min_max.o console.o main.o -lgdi32

    echo_and_run_cmd \
        ls -l send_input.exe

    if [ $TRUE = $is_release ]
    then
        echo_and_run_cmd \
            "$STRIP" --verbose send_input.exe

        echo_and_run_cmd \
            ls -l send_input.exe
    fi

    echo_and_run_cmd \
        cd -

    printf -- '\n'
    printf -- 'Try the new executable:\n'
    printf -- '$ wine send_input.exe release/sample_config.txt\n'
    printf -- '\n'
}

show_help_then_exit()
{
    printf -- '\n'
    printf -- 'Usage: %s [%s] [%s] [-h|--help]\n' "$0" "$ARG_CLEAN" "$ARG_RELEASE"
    printf -- 'Build executable (debug or release): %s\n' "$EXECUTABLE"
    printf -- '\n'
    printf -- 'Required Arguments:\n'
    printf -- '    None\n'
    printf -- '\n'
    printf -- 'Optional Arguments:\n'
    printf -- '    %s: Remove object files to force full rebuild\n' "$ARG_CLEAN"
    printf -- '\n'
    printf -- '    %s: Strip final executable\n' "$ARG_RELEASE"
    printf -- '\n'
    printf -- '    -h or --help: Show this help\n'
    printf -- '\n'
    printf -- 'Compatibility Notes:\n'
    printf -- '    This Bash shell script is compatible with Linux, Cygwin, and MSYS2\n'
    printf -- '\n'
    exit 1
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
    local arg_arr=()
    if [ $TRUE = $IS_LINUX ]
    then
        # Normally, 'winegcc' defines this auto-magically.
        arg_arr+=(-D__WINE__)
    fi
    # Ref: https://stackoverflow.com/a/62488988/257299
    echo_and_run_cmd \
        "$GCC" \
            "${arg_arr[@]}" \
            -g3 -ggdb3 \
            -Wall -Wextra -Wshadow \
            -Werror \
            -Wl,-subsystem,windows \
            -municode \
            "$@"
}

echo_and_run_gcc_cmd_if_necessary()
{
    # Ex: 'error_exit.c'
    local input_file_path="$1"; shift

    # Ex: 'error_exit.o'
    local output_file_path="$1"; shift

    # Remaining args stored in "$@"

    # $ help test -> "FILE1 -nt FILE2: True if file1 is newer than file2 (according to modification date)."
    if [ "$input_file_path" -nt "$output_file_path" ]
    then
        echo_and_run_gcc_cmd \
            -c -o "$output_file_path" "$input_file_path" "$@"
    else
        printf -- '\n%s: Not updated (do not re-compile)\n' "$input_file_path"
    fi
}

main "$@"

