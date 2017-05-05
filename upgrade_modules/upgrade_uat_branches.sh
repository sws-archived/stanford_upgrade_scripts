#!/usr/local/bin/bash
# Script must be run with bash 4
# See http://clubmate.fi/upgrade-to-bash-4-in-mac-os-x/ for instructions on how update OSX bash
bash --version
echo $BASH_VERSION

source includes/upgrade_functions.inc

# Usage: ./sed-make <module> <oldversion> <newversion>
# E.g., "./sed-make block_class 1.3 2.3"
# E.g., "./sed-make.sh stanford_events_importer 7.x-3.2 7.x-3.3
# Assumes data source will be called modules_to_upgrade.csv
# And be located in the same directory as sed-make.sh

# variables that need updating
last_stable_branch="stable20161206"
modules_being_upgraded=("views" "token" "stanford_sites_helper" "stanford_feeds_helper" "stanford_events_importer" "stanford_capx" "stanford_bean_types" "social_share" "metatag" "field_collection" "colorbox" )
# variables you can ignore for the most part
date=`date +%Y%m%d`
new_uat_branch="uat$date"

declare -A products_list=(
  ["jumpstart-academic"]="-jsa"
  ["jumpstart-plus"]="-jsplus"
  ["jumpstart"]="-jsv"
  ["jumpstart-lab"]="-jsl"
  ["jumpstart-engineering"]="-jse"
  ["jumpstart-vpsa"]="-jsvpsa"
  ["dept"]="-dept"
  ["group"]=""
)

declare -A stanford_profiles=(
  ["stanford-jumpstart-deployer"]="7.x-5.x"
  ["Stanford-Drupal-Profile"]="7.x-2.x"
)

# find_modules_in_profiles

# download gitolite repository and checkout last stable branch
if [ ! -d ~/Sites/$last_stable_branch ]; then
  git clone -b $last_stable_branch gitolite@git.stanford.edu:web/drupal-sites ~/Sites/$last_stable_branch
  cd ~/Sites/$last_stable_branch
  git branch -r | grep -v '\->' | while read remote; do git branch --track "${remote#origin/}" "$remote"; done
fi

update_stanford_profiles
read -p "Check the profiles updates.  Yes to continue, No to quit now. " -n 1 -r
echo "\n"
if [[ $REPLY =~ ^[Yy]$ ]]; then
  push_profile_branch_changes
else
 exit
fi

for product in ${!products_list[@]}; do
  suffix="${products_list[$product]}"
  new_uat_product_branch="$new_uat_branch$suffix"
  create_new_product_branch_in_gitolite_repository
  build_product_site
  move_commit_modules
  read -p "Check the products updates in ~/Sites/$last_stable_branch.  Yes to continue, No to quit now. " -n 1 -r
  echo "\n"
  if [[ $REPLY =~ ^[Yy]$ ]]; then
    push_product_branch_changes
  else
    exit
  fi
done
