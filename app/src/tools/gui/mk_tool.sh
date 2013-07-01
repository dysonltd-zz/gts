#!/bin/bash

toolClassName=$1
upperToolClassName=$(echo $toolClassName | tr [:lower:] [:upper:])

echo Making $toolClassName ...
for toolFile in templates/Tool*.*
do
    tmpToolFile=`mktemp`
    toolFileBasename=$(basename $toolFile)
    newToolFile=$toolClassName${toolFileBasename#Tool}
    echo "    Making $newToolFile"
    sed -r "s/NameOfTool/$toolClassName/g"  $toolFile > $tmpToolFile
    sed -r "s/NAMEOFTOOL/$upperToolClassName/g"  $tmpToolFile > $newToolFile
done
echo Done.



