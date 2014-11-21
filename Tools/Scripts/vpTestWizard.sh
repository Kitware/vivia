#!/bin/bash
# Purpose: Automatize workflow for adding tests, generating baselines, and including them in the build

## Specify where you would like to save all the tests you've made.
## This provides a scratch area to decide what you'd like to include.
PROJECT_TEST_DIRECTORY=$HOME/Testing

if [ -z "$VISGUI_DATA_ROOT" ] || [ ! -d "$VISGUI_DATA_ROOT" ]; then
  export VISGUI_DATA_ROOT=$HOME/Projects/visgui/data
fi
if [ -z "$VISGUI_BASELINE_ROOT" ] || [ ! -d "$VISGUI_BASELINE_ROOT" ]; then
  export VISGUI_BASELINE_ROOT=$VISGUI_DATA_ROOT/Baseline
fi

if [ ! -d "$PROJECT_TEST_DIRECTORY" ]; then
  echo "Creating directory to store all tests ($PROJECT_TEST_DIRECTORY)"
  mkdir -p "$PROJECT_TEST_DIRECTORY" || exit 1
fi
if [ ! -d "$VISGUI_DATA_ROOT" ] || [ ! -d "$VISGUI_BASELINE_ROOT" ]; then
  echo "Cannot find VISGUI_DATA_ROOT and/or VISGUI_BASELINE_ROOT."
  echo "Please set these environment variables properly."
  exit 1
fi

## Change the following to match your setup.

VISGUI_SOURCE_DIRECTORY=$HOME/Projects/visgui/source
VISGUI_BINARY_DIRECTORY=$HOME/Projects/visgui/build
# These don't need to change unless the project's internal structure changes.
VISGUI_TEMP_DIRECTORY=$VISGUI_BINARY_DIRECTORY/Testing/Temporary
VPVIEW_TESTING_DIRECTORY=$VISGUI_SOURCE_DIRECTORY/Applications/VpView/Testing
VPVIEW_BASELINE_DIRECTORY=$VISGUI_BASELINE_ROOT/vpView
VPVIEW_EXECUTABLE=$VISGUI_BINARY_DIRECTORY/bin/vpView

## Change the following to use alternate versions of these programs

GIT_COMMAND=$(type -P git)
CMAKE_COMMAND=$(type -P cmake)
MAKE_COMMAND=$(type -P make)
CTEST_COMMAND=$(type -P ctest)

###############################################################################

## Query information from the environment; these shouldn't need to change.

MAKE_JOBS=$(( $(grep -c 'model name' /proc/cpuinfo) + 1 ))

if [ -f "$VISGUI_SOURCE_DIRECTORY/.git/HEAD" ]; then
  GIT_BRANCH=$(cat "$VISGUI_SOURCE_DIRECTORY/.git/HEAD")
  GIT_BRANCH=${GIT_BRANCH##ref:\ refs/heads/}
  GIT_BRANCH=${GIT_BRANCH:-HEAD}
fi

BOLD="`tput bold`"
UNBOLD="`tput sgr0`"

###############################################################################

function update {
  WORKING_BRANCH=${1:-${GIT_BRANCH:-master}}
  if [ ! -d "$VISGUI_SOURCE_DIRECTORY" ]; then
    $GIT_COMMAND clone -b $WORKING_BRANCH "$VISGUI_SOURCE_DIRECTORY"
  fi
  pushd $VISGUI_SOURCE_DIRECTORY
  echo "${BOLD}[update] Updating project (fetch origin --prune, reset --keep origin/$WORKING_BRANCH)${UNBOLD}"
  $GIT_COMMAND fetch origin --prune && $GIT_COMMAND reset --keep origin/$WORKING_BRANCH
  UPDATE_SUCCESS=$?
  popd
  return $UPDATE_SUCCESS
}

function build {
  BUILD_DIRECTORY=${1:-$VISGUI_BINARY_DIRECTORY}
  pushd "$BUILD_DIRECTORY"
  echo "${BOLD}[build] Building project ($MAKE_COMMAND -j$MAKE_JOBS)${UNBOLD}"
  $CMAKE_COMMAND $VISGUI_SOURCE_DIRECTORY && $MAKE_COMMAND -j$MAKE_JOBS
  BUILD_SUCCESS=$?
  popd
  return $BUILD_SUCCESS
}

function record {
  if [ -n "$1" ]; then touch "$PROJECT_TEST_DIRECTORY/$1.xml"; fi
  echo "${BOLD}[record] Starting executable ($VPVIEW_EXECUTABLE)${UNBOLD}"
  if [ -x "$VPVIEW_EXECUTABLE" ]; then
    "$VPVIEW_EXECUTABLE"
  else
    echo "${BOLD}[record] Failure to start executable ($VPVIEW_EXECUTABLE)${UNBOLD}"
    exit 1
  fi
  if [ -n "$1" ] && [ -f "$PROJECT_TEST_DIRECTORY/$1.xml" ]; then
    echo "${BOLD}[record] Recorded test file ($PROJECT_TEST_DIRECTORY/$1.xml)${UNBOLD}"
  fi
  return $?
}

function insert {
  if [ -n "$1" ] && [ -f "$PROJECT_TEST_DIRECTORY/$1.xml" ]; then
    echo "${BOLD}[insert] Found test file ($PROJECT_TEST_DIRECTORY/$1.xml)${UNBOLD}"
    if cp -iv "$PROJECT_TEST_DIRECTORY/$1.xml" "$VPVIEW_TESTING_DIRECTORY"; then
      sed -e "s%$VISGUI_DATA_ROOT%\$VISGUI_DATA_ROOT/%g" -i "$VPVIEW_TESTING_DIRECTORY/$1.xml"
      if grep -qs "$1" "$VPVIEW_TESTING_DIRECTORY/CMakeLists.txt"; then
        echo "${BOLD}[insert] $VPVIEW_TESTING_DIRECTORY/CMakeLists.txt already contains '$1'${UNBOLD}"
      else
        echo -n "${BOLD}[insert] Adding test file ($1.xml) to ($VPVIEW_TESTING_DIRECTORY/CMakeLists.txt)... "
        if sed -e "/^SET (TESTS_WITH_BASELINES/a \ \ $1.xml" -i "$VPVIEW_TESTING_DIRECTORY/CMakeLists.txt"; then
          echo "SUCCESS${UNBOLD}"
        else
          echo "FAILURE${UNBOLD}"
        fi
      fi
    fi
  else
    echo "${BOLD}[insert] Missing test file ($PROJECT_TEST_DIRECTORY/$1.xml${UNBOLD}"
  fi
  return $?
}

function generate {
  if [ -n "$1" ]; then build && run "$1"
    if [ -f "$VISGUI_TEMP_DIRECTORY/$1.png" ]; then
      echo "${BOLD}[generate] Copying baseline ($VISGUI_TEMP_DIRECTORY/$1.png)${UNBOLD}"
      cp -iv "$VISGUI_TEMP_DIRECTORY/$1.png" "$VPVIEW_BASELINE_DIRECTORY"
    else
      echo "${BOLD}[generate] $1 baseline not found ($VISGUI_TEMP_DIRECTORY)${UNBOLD}"
    fi
  fi
  return $?
}

function run {
  pushd "$VISGUI_BINARY_DIRECTORY"
  echo "${BOLD}[run] Running test ($CTEST_COMMAND -R $1)${UNBOLD}"
  "$CTEST_COMMAND" -R $1
  popd
  return $?
}

function commit {
  pushd "$VISGUI_SOURCE_DIRECTORY"
  echo "${BOLD}[commit] Commiting test${UNBOLD}"
  "$GIT_COMMAND" add "$VPVIEW_TESTING_DIRECTORY/$1.xml"
  "$GIT_COMMAND" commit -m "Added test: $1"
  popd
}

# Decide which subset of the above subroutines to run in what sequence.
SCRIPT_ROOT=$(readlink -f $(dirname $0))
SCRIPT_FILE=$(basename $0)
SCRIPT_OPTIONS=( update build record insert generate run commit clean list refresh all some less )
COMMAND=$1
if [ -n "$COMMAND" ]; then
  shift
  case $COMMAND in
    refresh)
      update && build
      ;;
    all)
      record $@ && insert $@ && generate $@ && run $@
      ;;
    some)
      record $@ && generate $@ && run $@
      ;;
    less)
      generate $@ && run $@
      ;;
    clean)
      pushd "$VISGUI_SOURCE_DIRECTORY"
      "$GIT_COMMAND" reset --hard ${*:-${GIT_BRANCH:-origin/master}}
      rm "$HOME/.config/Kitware/VisGUI\ View.*" -iv
      popd
      rm -iv "$VISGUI_TESTING_DIRECTORY"/*
      ;;
    list)
      pushd "$VISGUI_BINARY_DIRECTORY"
      "$CTEST_COMMAND" -N
      popd
      ;;
    *)
      if [ "$(type -t $COMMAND)" == "function" ]; then $COMMAND $@; exit $?
      else echo "$COMMAND isn't one of: ${SCRIPT_OPTIONS[@]}"; fi
      ;;
  esac
else
  SCRIPT_OPTION_STRING=${SCRIPT_OPTIONS[*]}
  echo "USAGE: $SCRIPT_FILE (${SCRIPT_OPTION_STRING// /|}) test_name..."
fi

