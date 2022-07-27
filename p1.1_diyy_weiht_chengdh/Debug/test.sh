#!/usr/bin/env bash

outFile=output.s
types=('itype' 'utype' 'rtype' 'full' 'sbtype')
for i in "${types[@]}"; do
  read -ra input -d '' <<< "$(fdfind --full-path test/in/${i} | awk '/.s$/')"
  read -ra output -d '' <<< "$(fdfind --full-path test/ref/${i} | awk '/.s$/')"
  for j in "${!input[@]}"; do
    ./translator "${input[$j]}" ${outFile}
    diff ${outFile} "${output[$j]}"
  done
done
rm $outFile