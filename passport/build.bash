#!/usr/bin/env bash

COMMON_DIR_PATH='../common'
source "$(dirname "$0")/$COMMON_DIR_PATH/bashlib"

EXECUTABLE='passport.exe'

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    # Intentional: By default, build debug executable.
    local is_release=$BASHLIB_FALSE
    local is_clean=$BASHLIB_FALSE
    local must_build_common=$BASHLIB_FALSE

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
        elif [ "$BASHLIB_ARG_COMMON" = "$arg" ]
        then
            if [ $BASHLIB_FALSE = $must_build_common ]
            then
                must_build_common=$BASHLIB_TRUE
            else
                printf -- '\nError: Found multiple %s arguments\n' "$BASHLIB_ARG_COMMON"
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

    if [ $BASHLIB_TRUE = $must_build_common ]
    then
        build_common $is_release $is_clean
    fi

    if [ $BASHLIB_TRUE = $is_clean ]
    then
        bashlib_log_and_run_cmd \
            rm -f *.o
    fi

    local c_filename
    for c_filename in *.c
    do
        local bname
        bname="$(basename "$c_filename" '.c')"
        # Note: -iquote is more specific than -I
        bashlib_log_and_run_gcc_cmd_if_necessary \
            $is_release "$bname.c" "$bname.o" -iquote "$COMMON_DIR_PATH"
    done

    bashlib_log_and_run_gcc_cmd \
        $is_release -o "$EXECUTABLE" "$COMMON_DIR_PATH"/*.o *.o -lgdi32 -lcomctl32

    bashlib_log_and_run_cmd \
        ls -l "$EXECUTABLE"

    if [ $BASHLIB_TRUE = $is_release ]
    then
        bashlib_log_and_run_strip_cmd \
            --verbose "$EXECUTABLE"

        bashlib_log_and_run_cmd \
            ls -l "$EXECUTABLE"
    fi

    bashlib_log_and_run_cmd \
        cd -

    printf -- '\n'
    printf -- 'Try the new executable:\n'

    local exec2="$EXECUTABLE"
    if [ $BASHLIB_TRUE = $BASHLIB_IS_LINUX ]
    then
        exec2="wine $exec2"
    fi

    printf -- '$ %s\n' "$exec2"
    printf -- '\n'
    if [ $BASHLIB_TRUE = $BASHLIB_IS_LINUX ]
    then
        printf -- 'To debug:\n'
        printf -- '    term1$ wine Z:/usr/share/win64/gdbserver.exe localhost:12345 %s\n' "$EXECUTABLE"
        printf -- "    term2$ x86_64-w64-mingw32-gdb -ex 'set print elements 0' -ex 'target extended-remote localhost:12345' -ex 'break main.c:\$line' -ex continue %s\n" \
                  "$EXECUTABLE"
        printf -- '    ...\n'
        printf -- '    term1$ <Ctrl+C>, then re-run\n'
        printf -- '    term2$\n'
        printf -- '    (gdb) k\n'
        printf -- 'Kill the program being debugged? (y or n) y\n'
        printf -- 'Remote connection closed\n'
        printf -- '    (gdb) target extended-remote localhost:12345\n'
        printf -- '    (gdb) c\n'
        printf -- '    ...\n'
        printf -- '\n'
    fi
}

build_common()
{
    local is_release="$1" ; shift
    local is_clean="$1" ; shift

# TODO: If release/debug, then compile into separate dirs!
#    if [ $BASHLIB_TRUE = $is_release ]
#    then
        # Handle the scenario where common libs where built without optimization and with debug info -- or the opposite.
        # It is safer to *always* cleanly recompile common libs.
        is_clean=$BASHLIB_TRUE
#    fi

    local arg_arr=()
    if [ $BASHLIB_TRUE = $is_release ]
    then
        arg_arr=("$BASHLIB_ARG_RELEASE")
    fi

    if [ $BASHLIB_TRUE = $is_clean ]
    then
        arg_arr=("$BASHLIB_ARG_CLEAN")
    fi

    bashlib_log_and_run_cmd \
        "$COMMON_DIR_PATH/build.bash" "${arg_arr[@]}"
}

show_help_then_exit()
{
    printf -- '\n'
    printf -- 'Usage: %s [%s] [%s] [%s|%s]\n' "$0" "$BASHLIB_ARG_COMMON" "$BASHLIB_ARG_CLEAN" "$BASHLIB_ARG_RELEASE" "$BASHLIB_ARG_HELP" "$BASHLIB_ARG_HELP2"
    printf -- 'Build executable: %s\n' "$EXECUTABLE"
    printf -- '\n'
    printf -- 'Required Arguments:\n'
    printf -- '    None\n'
    printf -- '\n'
    printf -- 'Optional Arguments:\n'
    printf -- '    %s: Build common library\n' "$BASHLIB_ARG_COMMON"
    printf -- '\n'
    printf -- '    %s: Remove object files to force full rebuild\n' "$BASHLIB_ARG_CLEAN"
    printf -- '\n'
    printf -- '    %s: Strip final executable\n' "$BASHLIB_ARG_RELEASE"
    printf -- '\n'
    printf -- '    %s or %s: Show this help\n' "$BASHLIB_ARG_HELP" "$BASHLIB_ARG_HELP2"
    printf -- '\n'
    printf -- 'Compatibility Notes:\n'
    printf -- '    This Bash shell script is compatible with Linux, Cygwin, and MSYS2.\n'
    printf -- '\n'

    exit 1
}

main "$@"

