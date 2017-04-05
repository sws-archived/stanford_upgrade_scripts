#!/usr/local/bin/bash
# Script must be run with bash 4
# See http://clubmate.fi/upgrade-to-bash-4-in-mac-os-x/ for instructions on how update OSX bash
bash --version
echo $BASH_VERSION

source includes/find_functions.inc

# Usage: ./sed-make <module> <oldversion> <newversion>
# E.g., "./sed-make block_class 1.3 2.3"
# E.g., "./sed-make.sh stanford_events_importer 7.x-3.2 7.x-3.3
# Assumes data source will be called modules_to_upgrade.csv
# And be located in the same directory as sed-make.sh

# variables that need updating
last_stable_branch="stable20161206"
modules_being_upgraded=("")

# variables you can ignore for the most part
date=`date +%Y%m%d`

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

path="/Users/kelliebrownell/Sites"

function find_module_versions {
  if [[ $module_name == *"stanford"* ]]; then
    current_version=$(cat $path/$stanford_profile/$product_path/stanford.make | sed -n -E 's|projects\['$module_name'\]\[download\]\[tag\] = \"(.*)\"|\1|p')
  elif [[ $module_name == *"open_framework"* ]]; then
    current_version=$(cat contrib.make | sed -n -E 's|projects\['$module_name'\]\[download\]\[tag\] = \"(.*)\"|\1|p')
  else
    current_version=$(cat contrib.make | sed -n -E 's|projects\['$module_name'\]\[version\] = \"(.*)\"|\1|p')
  fi
}

while read module_name; do
  for product in ${!products_list[@]}; do
    if [ "$product" == "Stanford-Default-Profile" ]; then
      product_path="group"
    else
      product_path="production/$product"
      developent_path="development/$product"
    fi
    find_module_version
  done
done < modules_to_upgrade.csv
