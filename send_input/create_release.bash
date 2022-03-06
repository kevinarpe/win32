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

    check_args "$@"

    # Ex: '1.0'
    local version="$1"

    local orig_pwd="$(pwd)"

    echo_and_run_cmd \
        cd "$this_script_abs_dir_path"

    local package_name
    # Ex: 'send_input'
    package_name="$(basename "$(pwd)")"

    # Ex: 'send_input-1.0'
    local release_dir_name="$package_name-$version"

    # Ex: 'send_input-1.0.zip'
    local release_package_file_name="$release_dir_name.zip"

    echo_and_run_cmd \
        ./build.bash

    if [ -d "release/$release_dir_name" ]
    then
        echo_and_run_cmd \
            /bin/rm -Rf "release/$release_dir_name"
    fi

    echo_and_run_cmd \
        mkdir --parents --verbose "release/$release_dir_name"

    local file_name
    for file_name in README.txt sample_config.txt
    do
        echo_and_run_cmd \
            cp --verbose "release/$file_name" "release/$release_dir_name/"
    done

    echo_and_run_cmd \
        cp --verbose "${package_name}.exe" "release/$release_dir_name/"

    echo_and_run_cmd \
        cd release

    echo_and_run_cmd \
        zip -v -r "$release_package_file_name" "$release_dir_name/"

    echo_and_run_cmd \
        unzip -v -l "$release_package_file_name"

    echo_and_run_cmd \
        ls -l "$release_package_file_name"

    echo_and_run_cmd \
        cd "$orig_pwd"
}

check_args()
{
    if [ 1 != $# ]
    then
        printf -- '\n'
        printf -- 'Usage: %s VERSION\n' "$0"
        printf -- 'Build latest code, then create release ZIP package\n'
        printf -- '\n'
        printf -- 'Required Arguments:\n'
        printf -- '    VERSION: Release version\n'
        printf -- '        Ex: 1.0\n'
        printf -- '\n'
        exit 1
    fi
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

