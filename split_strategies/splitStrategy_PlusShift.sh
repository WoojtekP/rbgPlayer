#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd ${DIR}

../../rbggamemanager/build/print $1 --add_dots_in_alternatives true --disable_adding_dots_in_shifttables true --one_dot_or_modifier_per_concat true > $1.tmp1
../../rbggamemanager/build/print $1.tmp1 --add_dots_after_alternatives true --dots_only_in_shifttables true --one_dot_or_modifier_per_concat false > $1.tmp2
rm $1.tmp1
rm $1
mv $1.tmp2 $1
