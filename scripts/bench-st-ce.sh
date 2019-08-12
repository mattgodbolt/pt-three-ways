#!/bin/bash

set -e

make -C cmake-build-release

for way in oo fp dod; do
  echo Testing $way:
  time ./cmake-build-release/bin/pt_three_ways \
    --width 64 --height 64 \
    --num-cpus 1 --spp 1 --scene ce \
    --way $way /dev/null
done
