#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd ${DIR}

../../rbggamemanager/build/print $1 --add_dots_after_alternatives true --dots_only_in_shifttables true > $1.tmp1
rm $1
mv $1.tmp1 $1
