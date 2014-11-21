#!/bin/bash

# Check for list of files
if [ -z "$*" ]; then
  echo "Usage: ${0##*/} <files or directories>" >&2
  exit 1
fi

# Find astyle options
self="$(readlink -f "$0")"
optfile="$(
  cd "${self%/*}" >/dev/null;
  cd "./$(git rev-parse --show-cdup)" >/dev/null
  readlink -f .astyle
)"
if ! [ -f "$optfile" ]; then
  echo "Unable to locate astyle options file." >&2
  exit 2
fi

# Create file list
filelist="$(mktemp)"
if ! [ -r "$filelist" ]; then
  echo "Unable to create temporary file for file list." >&2
  exit 2
fi

# Populate file list, finding source files in any directories given
for f in "$@"; do
  if [ -d "$f" ]; then
    find "$f" \( -name '*.cxx' -o -name '*.h' \) -print0 >> "$filelist"
  else
    printf "%s\0" "$f" >> "$filelist"
  fi
done

# Fix the files
xargs -0 sed -i \
  -e 's/\[ /[/g' \
  -e 's/ \]/]/g' \
  -e 's/> >/>!__template__!>/g' \
  -e 's/< /</g' \
  -e 's/ >/>/g' \
  -e 's/>!__template__!>/> >/g' \
  -e 's/template</template </g' \
  < "$filelist"
xargs -0 astyle --suffix=none --options="$optfile" < "$filelist"
xargs -0 sed -ri \
  -e 's/\b(return|emit)\(/\1 (/g' \
  -e 's/\b((for(each[a-z_]*)?|Q(TE)?_FOREACH[A-Z_]*) *\([^*&;,]*) ([*&])/\1\5/g' \
  -e 's/([^ ])(\(&[a-zA-Z0-9_]*\)\[)/\1 \2/g' \
  -e 's/operator *([^A-Za-z0-9_])/operator\1/g' \
  < "$filelist"
rm -f "$filelist"
