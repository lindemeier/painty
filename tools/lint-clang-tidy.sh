#!/usr/bin/env bash

workspace_dir=$(bazel info workspace)

cd $workspace_dir

# generate compile_commands.json
bazel build //painty:painty_comdb

# get the compile_commands.json
compile_commands_outfile="$(bazel info bazel-bin)/painty/compile_commands.json"
echo "Compilation database in "$compile_commands_outfile" is ready."

# post process the compilation commands file.
# __EXEC_ROOT__ needs to be replaced with the directory containing the sources.
sed -i -- "s|__EXEC_ROOT__|$workspace_dir|g" $compile_commands_outfile

if [ -z "${CLANG_TIDY:=$(which clang-tidy)}" ]; then
  echo "Unable to find clang-tidy" 1>&2
  exit 1
fi

# get all relevant files and let clang-tidy process those
find $workspace_dir/painty -regex '.*\.\(cpp\|h\|hpp\|cc\|cxx\)' -exec $CLANG_TIDY -p $compile_commands_outfile -fix  -fix-errors -extra-arg=-std=c++17 -extra-arg=-isystem -extra-arg=-I$workspace_dir {}  \;

exit 0
