#!/usr/bin/env bash

COMMON_DIR_PATH='..'
source "$(dirname "$0")/$COMMON_DIR_PATH/bashlib"

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    bashlib_log_and_run_cmd \
        cd "$this_script_abs_dir_path"

    bashlib_log_and_run_cmd \
        rm --force *.o *.exe

    bashlib_log_and_run_cmd \
        ../build.bash

    local csrc
    for csrc in *_test.c
    do
        local bname
        # Ex: "wstr_test.c" -> "wstr_test"
        bname="$(basename "$csrc" '.c')"
        build_then_run "$bname"
    done

    bashlib_log_and_run_cmd \
        cd -
}

build_then_run()
{
    # Ex: "wstr_test"
    local test_module="$1" ; shift

    local is_release=$BASHLIB_FALSE

    # Note: -iquote is more specific than -I
    bashlib_log_and_run_gcc_cmd_if_necessary \
        $is_release "$test_module.c" "$test_module.o" -iquote "$COMMON_DIR_PATH"

    bashlib_log_and_run_gcc_cmd \
        $is_release \
        -o "$test_module.exe" \
        "$COMMON_DIR_PATH/"*.o \
        "$test_module.o" \
        -lgdi32

    bashlib_log_and_run_cmd \
        ls -l "$test_module.exe"

    bashlib_log_and_run_cmd \
        wine64 "$test_module.exe"
}

main "$@"

