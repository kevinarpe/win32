#!/usr/bin/env bash

source "$(dirname "$0")"/bashlib

TRUE=0
FALSE=1

ARG_HELP='-h'
ARG_HELP2='--help'
ARG_CLEAN='--clean'

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    local is_clean=$FALSE

    local arg
    for arg in "$@"
    do
        if [ "$ARG_HELP" = "$arg" ] || [ "$ARG_HELP2" = "$arg" ]
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
        else
            printf -- '\nError: Unknown argument: [%s]\n' "$arg"
            show_help_then_exit
        fi
    done

    bashlib_echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    if [ $TRUE = $is_clean ]
    then
        bashlib_echo_and_run_cmd \
            rm -f *.o
    fi

    local bname
    for bname in win32 log error_exit win32_xmalloc wstr min_max console
    do
        bashlib_echo_and_run_gcc_cmd_if_necessary \
            $bname.c $bname.o
    done

    bashlib_echo_and_run_cmd \
        cd -
}

show_help_then_exit()
{
    printf -- '\n'
    printf -- 'Usage: %s [%s] [%s|%s]\n' "$0" "$ARG_CLEAN" "$ARG_HELP" "$ARG_HELP2"
    printf -- 'Build common libraries\n'
    printf -- '\n'
    printf -- 'Required Arguments:\n'
    printf -- '    None\n'
    printf -- '\n'
    printf -- 'Optional Arguments:\n'
    printf -- '    %s: Remove object files to force full rebuild\n' "$ARG_CLEAN"
    printf -- '\n'
    printf -- '    %s or %s: Show this help\n' "$ARG_HELP" "$ARG_HELP2"
    printf -- '\n'
    printf -- 'Compatibility Notes:\n'
    printf -- '    This Bash shell script is compatible with Linux, Cygwin, and MSYS2\n'
    printf -- '\n'

    exit 1
}

main "$@"

