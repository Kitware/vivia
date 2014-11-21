#!/bin/bash
#kate: indent normal;

# Note: contrary to the name, this script doesn't find "all" sources, but
# rather tries to find all files that should have Kitware copyright statements.
# It isn't perfect (and doesn't need to be), but should have a fairly low false
# positive rate.

cd $(dirname "$0")
if ! [ "$(git rev-parse --is-inside-work-tree)" = "true" ]; then
    echo "couldn't find repository root" >&2
    exit 1
fi
rel="$(git rev-parse --show-cdup)"
cd "${rel-.}"

find . \
    -type d \( -name QtTestingSupport -o -name '.kde*' -o -name .git \) \
        -prune -false \
    -o \! \( -type d -o -name '*.ui' -o -name '*.qrc' -o -name '*.xml' \
             -o -name 'CMakeLists.txt' -o -name '*.cmake' \
             -o -name '*.png' -o -name '*.svg' -o -name '*.dot' \
             -o -name '*.prj'  -o -name '*.prj.in' \
             -o -name 'jmem_src.c' -o -name 'testdata.kst' \
             -o -name '*.kdev*' -o -name .gitignore -o -name .gitattributes \) \
    -print | cut -c3-
