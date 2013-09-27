#!/bin/bash

# Script to bump the version of the GTS.

# Usage: ./bump_version.sh <major.minor.patch> --commit --tag

if [[ -z $1 ]]
then
	echo No new version number given.
	exit 1
fi

VERSION=$1
MAJOR=$(echo $1 | sed 's/\([0-9]\+\).[0-9]\+.[0-9]\+/\1/')
MINOR=$(echo $1 | sed 's/[0-9]\+.\([0-9]\+\).[0-9]\+/\1/')
PATCH=$(echo $1 | sed 's/[0-9]\+.[0-9]\+.\([0-9]\+\)/\1/')

DAY=$(date +%d)
MONTH=$(date +%m)
YEAR=$(date +%Y)

VERSION_FILE=$(find . -name "Version.h")
HELP_FILE=$(find . -name "gts_userguide.htm")
INSTALLER_CMAKE_FILE=$(find . -name "CMakeLists.txt")

# Version.h version number
sed -i "s/[0-9]\+\.[0-9]\+\.[0-9]\+/${1}/g" $VERSION_FILE

# Help file version number
sed -i "s/\(style='font-size:[0-9]\+.[0-9]\+pt'>\)[0-9]\+.[0-9]\+.[0-9]\+/\1${1}/g" $HELP_FILE

# Version file date
sed -i "s/[0-9]\+\/[0-9]\+\/[0-9]\+/${DAY}\/${MONTH}\/${YEAR}/g" $VERSION_FILE

# Installer CMakelists.txt version
sed -i "s/\(CPACK_PACKAGE_VERSION_MAJOR \"\)[0-9]\+/\1${MAJOR}/g" $INSTALLER_CMAKE_FILE
sed -i "s/\(CPACK_PACKAGE_VERSION_MINOR \"\)[0-9]\+/\1${MINOR}/g" $INSTALLER_CMAKE_FILE
sed -i "s/\(CPACK_PACKAGE_VERSION_PATCH \"\)[0-9]\+/\1${PATCH}/g" $INSTALLER_CMAKE_FILE

git add $VERSION_FILE $HELP_FILE $INSTALLER_CMAKE_FILE

git commit -m "Bumped to v$VERSION"
echo "Git commit created"

if [[ $? != 0 ]]
then
	echo "Commit and tagging aborted."
	exit 1
fi

git tag -a $VERSION -m "v$VERSION"
echo "Git tag created v$VERSION"

exit 0
