#!/bin/bash

prog_name=${PWD##*/}
cur_date=$(date +%s)
cur_version=$(cat src/waver_version)

cd ..

tar cvJf "${prog_name}-${cur_version}-${cur_date}.tar.xz" "${prog_name}"

