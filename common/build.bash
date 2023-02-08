#!/usr/bin/env bash

source "$(dirname "$0")"/bashlib

ARG_HELP='-h'
ARG_HELP2='--help'
ARG_CLEAN='--clean'

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    # Intentional: By default, build debug executable.
    local is_release=$BASHLIB_FALSE
    local is_clean=$BASHLIB_FALSE

    local arg
    for arg in "$@"
    do
        if [ "$BASHLIB_ARG_HELP" = "$arg" ] || [ "$BASHLIB_ARG_HELP2" = "$arg" ]
        then
            show_help_then_exit

        elif [ "$BASHLIB_ARG_CLEAN" = "$arg" ]
        then
            if [ $BASHLIB_FALSE = $is_clean ]
            then
                is_clean=$BASHLIB_TRUE
            else
                printf -- '\nError: Found multiple %s arguments\n' "$BASHLIB_ARG_CLEAN"
                show_help_then_exit
            fi

        elif [ "$BASHLIB_ARG_RELEASE" = "$arg" ]
        then
            if [ $BASHLIB_FALSE = $is_release ]
            then
                is_release=$BASHLIB_TRUE
            else
                printf -- '\nError: Found multiple %s arguments\n' "$BASHLIB_ARG_RELEASE"
                show_help_then_exit
            fi
        else
            printf -- '\nError: Unknown argument: [%s]\n' "$arg"
            show_help_then_exit
        fi
    done

    if [ $BASHLIB_TRUE = $is_release ]
    then
        bashlib_logf 'Release build'
    else
        bashlib_logf 'Debug build'
    fi

    bashlib_log_and_run_cmd \
        cd "$this_script_abs_dir_path"

    if [ $BASHLIB_TRUE = $is_clean ]
    then
        bashlib_log_and_run_cmd \
            rm -f *.o
    fi

    local csrc
    for csrc in *.c
    do
        local bname
        # Ex: "wstr.c" -> "wstr"
        bname="$(basename "$csrc" '.c')"
        bashlib_log_and_run_gcc_cmd_if_necessary \
            "$is_release" "$bname.c" "$bname.o"
    done

    bashlib_log_and_run_cmd \
        cd -
}

show_help_then_exit()
{
    printf -- '\n'
    printf -- 'Usage: %s [%s] [%s] [%s|%s]\n' "$0" "$BASHLIB_ARG_CLEAN" "$BASHLIB_ARG_RELEASE" "$BASHLIB_ARG_HELP" "$BASHLIB_ARG_HELP2"
    printf -- 'Build common libraries\n'
    printf -- '\n'
    printf -- 'Required Arguments:\n'
    printf -- '    None\n'
    printf -- '\n'
    printf -- 'Optional Arguments:\n'
    printf -- '    %s: Remove object files to force full rebuild\n' "$BASHLIB_ARG_CLEAN"
    printf -- '\n'
    printf -- '    %s: Strip final executable\n' "$BASHLIB_ARG_RELEASE"
    printf -- '\n'
    printf -- '    %s or %s: Show this help\n' "$BASHLIB_ARG_HELP" "$BASHLIB_ARG_HELP2"
    printf -- '\n'
    printf -- 'Compatibility Notes:\n'
    printf -- '    This Bash shell script is compatible with Linux, Cygwin, and MSYS2\n'
    printf -- '\n'

    exit 1
}

main "$@"

