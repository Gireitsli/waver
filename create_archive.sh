#!/bin/bash

prog_name=${PWD##*/}
cur_date=$(date +%s)
cur_version=$(cat ./waver_version)

# excludes
ex_ignore_file="${prog_name}/.gitignore"
ex_git_folder="${prog_name}/.git"
ex_obj="${prog_name}/obj"
ex_bin="${prog_name}/bin"

cd ..

tar cvJf "${prog_name}-${cur_version}-${cur_date}.tar.xz" "${prog_name}" \
         --exclude=${ex_ignore_file} \
         --exclude=${ex_git_folder} \
         --exclude=${ex_obj} \
         --exclude=${ex_bin}

