#!/usr/bin/env bash

COMMON_DIR_PATH='../common'
source "$(dirname "$0")/$COMMON_DIR_PATH/bashlib"

TRUE=0
FALSE=1

ARG_HELP='-h'
ARG_HELP2='--help'
ARG_CLEAN='--clean'
ARG_RELEASE='--release'

EXECUTABLE='send_input.exe'

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

    bashlib_echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    build_common $is_clean

    if [ $TRUE = $is_clean ]
    then
        bashlib_echo_and_run_cmd \
            rm -f *.o
    fi

    # Note: -iquote is more specific than -I
    bashlib_echo_and_run_gcc_cmd_if_necessary \
        config.c config.o -iquote "$COMMON_DIR_PATH"

    bashlib_echo_and_run_gcc_cmd_if_necessary \
        main.c main.o -iquote "$COMMON_DIR_PATH"

    bashlib_echo_and_run_gcc_cmd \
        -o "$EXECUTABLE" \
        "$COMMON_DIR_PATH/win32.o" \
        "$COMMON_DIR_PATH/log.o" \
        "$COMMON_DIR_PATH/error_exit.o" \
        "$COMMON_DIR_PATH/win32_xmalloc.o" \
        "$COMMON_DIR_PATH/wstr.o" \
        "$COMMON_DIR_PATH/min_max.o" \
        "$COMMON_DIR_PATH/console.o" \
        config.o main.o -lgdi32

    bashlib_echo_and_run_cmd \
        ls -l "$EXECUTABLE"

    if [ $TRUE = $is_release ]
    then
        bashlib_echo_and_run_strip_cmd \
            --verbose "$EXECUTABLE"

        bashlib_echo_and_run_cmd \
            ls -l "$EXECUTABLE"
    fi

    bashlib_echo_and_run_cmd \
        cd -

    printf -- '\n'
    printf -- 'Try the new executable:\n'

    local exec2="$EXECUTABLE"
    if [ $TRUE = $BASHLIB_IS_LINUX ]
    then
        exec2="wine $exec2"
    fi

    printf -- '$ %s release/sample_config.txt\n' "$exec2"
    printf -- '\n'
}

build_common()
{
    local is_clean="$1" ; shift

    local arg_arr=()
    if [ $TRUE = $is_clean ]
    then
        arg_arr=("$ARG_CLEAN")
    fi

    bashlib_echo_and_run_cmd \
        "$COMMON_DIR_PATH/build.bash" "${arg_arr[@]}"
}

show_help_then_exit()
{
    printf -- '\n'
    printf -- 'Usage: %s [%s] [%s] [%s|%s]\n' "$0" "$ARG_CLEAN" "$ARG_RELEASE" "$ARG_HELP" "$ARG_HELP2"
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
    printf -- '    %s or %s: Show this help\n' "$ARG_HELP" "$ARG_HELP2"
    printf -- '\n'
    printf -- 'Compatibility Notes:\n'
    printf -- '    This Bash shell script is compatible with Linux, Cygwin, and MSYS2\n'
    printf -- '\n'
    exit 1
}

main "$@"

