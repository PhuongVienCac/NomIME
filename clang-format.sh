#!/usr/bin/env bash

NOMIME_SOURCE_PATH="RimeWithNomIME NomIMEDeployer NomIMEIPC NomIMEIPCServer NomIMEServer NomIMESetup NomIMETSF NomIMEUI include test"

# clang format options
method="-i"

while getopts "in" option; do
	case "${option}" in
	n) # format code
		find ${NOMIME_SOURCE_PATH} -name '*.cpp' -o -name '*.h' ! -path "include/wtl/*" | grep -wiv -f .exclude_pattern.txt | xargs clang-format --verbose -i
		;;
	i) # dry run and changes formatting warnings to errors
		find ${NOMIME_SOURCE_PATH} -name '*.cpp' -o -name '*.h' ! -path "include/wtl/*" | grep -wiv -f .exclude_pattern.txt | xargs clang-format --verbose -Werror --dry-run || { echo Please lint your code by '"'"./clang-format.sh -n"'"'.; false; }
		;;
	\?) # invalid option
		echo "invalid option, please use -i or -n."
		exit 1
		;;
	esac
done
