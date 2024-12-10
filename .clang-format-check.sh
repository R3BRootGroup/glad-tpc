#!/bin/bash

# Parse arguments
test "$1" == "--autofix" && AUTOFIX=1 && shift
test "$1" == "--ci" && AUTOFIX=1 && CI=1 && shift

# Define clang-format command
CLANG_FORMAT=${1:-clang-format}

# Starting
echo -n "Running clang-format checks, version: "
${CLANG_FORMAT} --version

# Check if clang-format is available
if [ $? -ne 0 ]; then
    echo -e "\033[1;31mclang-format missing: ${CLANG_FORMAT}\033[0m"
    exit 1
fi

# Fetch latest changes
git fetch --all
base_commit="origin/dev"
echo "Checking against parent commit $(git rev-parse $base_commit)"

# To simplify CI debugging, list the files to be checked
echo "--- Listing all changed files:"
git diff --name-only ${base_commit}
echo "---"

FMT_FILE=$(mktemp)

# Get the list of files to check: .cxx, .h, excluding .C and others
filesToCheck=$(git diff --name-only ${base_commit} | grep -E '\.(cxx|h)$' | grep -vE '^(macros/|params/|.*\.par|.*\.C)$' || true)

# Iterate over each file
for f in $filesToCheck; do
    # Check if the file exists
    if ! test -f "$f"; then
        echo -e "\033[1;31mError: File $f not found. Skipping.\033[0m"
        continue  # Skip this file if it doesn't exist
    fi

    echo "Processing: $f"  # Show the file being processed

    # If auto-fix is enabled
    if test -n "$AUTOFIX"; then
        echo "Fixing $f, if needed."
        $CLANG_FORMAT -i -style=file "$f"
    else
        # If not auto-fixing, compare the file with its formatted version
        echo "Checking: $f"
        d=$(diff -u "$f" <($CLANG_FORMAT -style=file "$f") || true)

        # If differences are found, log them
        if ! [ -z "$d" ]; then
            echo "$d" | tee -a ${FMT_FILE}
            fail=1
        fi
    fi
done

# Check the result of formatting
if test -n "$CI"; then
    # If running in CI, check if any files failed
    if test -n "$fail"; then
        echo -e "\033[1;31mYou must pass the clang-format checks before submitting a pull request for the files:\033[0m"
        for f in $filesToCheck; do
            # Check and list files that need formatting
            d=$(diff -u "$f" <($CLANG_FORMAT -style=file "$f") || true)
            if ! [ -z "$d" ]; then
                echo "$f"
            fi
        done
        exit 1
    else
        echo -e "\033[1;32m✔ Passed clang-format checks\033[0m"
    fi
else
    # If not in CI, inform the user about failed checks
    if test -n "$fail"; then
        echo "Clang-format check failed. Try --autofix to fix it in the code."
    else
        echo "All files passed the clang-format check."
    fi
fi

