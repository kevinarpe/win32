#!/usr/bin/env bash

COMMON_DIR_PATH='../common'
source "$(dirname "$0")/$COMMON_DIR_PATH/bashlib"

TRUE=0
FALSE=1

ARG_REAL='--real'

main()
{
    local this_script_abs_dir_path
    this_script_abs_dir_path="$(dirname "$(readlink --canonicalize "$0")")"

    check_args "$@"

    # Ex: '1.0'
    local version="$1" ; shift

    local is_real=$FALSE
    if [ 1 = $# ]
    then
        if [ "$ARG_REAL" = "$1" ]
        then
            is_real=$TRUE ; shift
        else
            printf -- '\nUnknown argument: [%s]\n' "$1"
        fi
    fi

    local orig_pwd="$(pwd)"

    bashlib_echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    check_git_status_clean

    local package_name
    # Ex: 'send_input'
    package_name="$(basename "$(pwd)")"

    # Ex: 'send_input-1.0'
    local release_dir_name="$package_name-$version"

    # Ex: 'send_input-1.0.zip'
    local release_package_file_name="$release_dir_name.zip"

    bashlib_echo_and_run_cmd \
        ./build.bash --clean --release

    if [ -d "release/$release_dir_name" ]
    then
        bashlib_echo_and_run_cmd \
            /bin/rm -Rf "release/$release_dir_name"
    fi

    bashlib_echo_and_run_cmd \
        mkdir --parents --verbose "release/$release_dir_name"

    local file_name
    for file_name in README.txt sample_config.txt
    do
        bashlib_echo_and_run_cmd \
            cp --verbose "release/$file_name" "release/$release_dir_name/"
    done

    bashlib_echo_and_run_cmd \
        cp --verbose "${package_name}.exe" "release/$release_dir_name/"

    bashlib_echo_and_run_cmd \
        cd release

    bashlib_echo_and_run_cmd \
        zip -v -r "$release_package_file_name" "$release_dir_name/"

    bashlib_echo_and_run_cmd \
        unzip -v -l "$release_package_file_name"

    bashlib_echo_and_run_cmd \
        ls -l "$release_package_file_name"

    if [ $TRUE = $is_real ]
    then
        bashlib_echo_and_run_cmd \
            git tag "$release_dir_name"

        bashlib_echo_and_run_cmd \
            git push --tags
    else
        printf -- '\nDry run: Do not run git tag & push\n'
    fi

    bashlib_echo_and_run_cmd \
        cd "$orig_pwd"

    if [ $TRUE = $is_real ]
    then
        printf -- '\nPlease upload this release to GitHub here: https://github.com/kevinarpe/win32/releases/new\n\n'
    fi
}

check_args()
{
    if [ 0 == $# ] || [ $# -gt 2 ]
    then
        printf -- '\n'
        printf -- 'Usage: %s VERSION [%s]\n' "$0" "$ARG_REAL"
        printf -- 'Build latest code, then create release ZIP package\n'
        printf -- '\n'
        printf -- 'Required Arguments:\n'
        printf -- '    VERSION: Release version\n'
        printf -- '        Ex: 1.0\n'
        printf -- '\n'
        printf -- 'Required Arguments:\n'
        printf -- '    %s: Run git tag & push; else, this is a dry run.\n' "$ARG_REAL"
        printf -- '\n'
        exit 1
    fi
}

# Ref: https://unix.stackexchange.com/a/155077/29414
check_git_status_clean()
{
    local output
    output="$(git status --porcelain)"

    if [ ! -z "$output" ]
    then
        printf -- '\n'
        printf -- '$ git status\n'
        printf -- '... is not clean.  Please commit, then try again.\n'

        bashlib_echo_and_run_cmd \
            git status

        exit 1
    fi
}

main "$@"

