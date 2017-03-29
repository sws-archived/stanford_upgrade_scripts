#!/bin/bash

source includes/common.inc
source variables.inc

function save_list_of_sites_on_server {
  if [ -z "$sws_sites_list_path" ]; then
    find $server_webroot -type d -mindepth 1 -maxdepth 1 -exec basename {} \; > sws_sites_list.csv
  fi
}

function find_module_paths {
  # Update sitename to include suffix if specified
  if [ ! -z "$sitename_suffix" ]; then sitename_with_suffix="$sitename/$sitename_suffix"; fi

  # Save module paths for both sites/default and sites/all
  module_all_path=$(find $server_webroot/$sitename_with_suffix/sites/all/modules/*/ -mindepth 1 -maxdepth 1 -type d -name "$module_input")
  module_default_path=$(find $server_webroot/$sitename_with_suffix/sites/default/modules/*/ -mindepth 1 -maxdepth 1 -type d -name "$module_input")
}

function compare_module_paths {
  # Skip to the next iteration, if module was not found in sites/default
  [ -z "$module_default_path" ] && continue
  if [ -z "$module_all_path" ]; then module_difference_summary="not_in_sites_all"; fi
}

function find_module_versions {
  # Determine module version only if the same module was found in both sites/default and sites/all
  if [ ! -z "$module_all_path" ] && [ ! -z "$module_default_path" ]; then
    sites_all_version=$(grep version $module_all_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
    sites_default_version=$(grep version $module_default_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
  fi
}

function find_module_code_differences {
  # Check this only if we've already established an equality in module versions
  module_code_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path" 2> /dev/null)
  if [ ! -z "$module_code_difference" ]; then
    code_comparison="code_differs"
  elif [ ! -z "$module_all_path" ]; then
    code_comparison="matches"
  fi
}

function compare_module_versions {
  # Evaluate version differences between sites/default and sites/all
  if echo $sites_default_version $sites_all_version | awk '{exit !($1 > $2)}'; then
    code_comparison="default_older"
  elif echo $sites_default_version $sites_all_version | awk '{exit !($1 == $2)}'; then
    code_comparison="equal"
    find_module_code_differences
  elif echo $sites_default_version $sites_all_version | awk '{exit !($1 < $2)}'; then
    code_comparison="default_newer"
  else
    code_comparison="not able to compare versions"
    find_module_code_differences
  fi
}

function reduce_list_by_difference {
  # determine whether module differences match user input
  if (( `in_array "$code_comparison" "${difference_selection[@]}"` == 1 )); then
    site_difference_inclusion="included"
  fi
}

function reduce_list_by_status {
  # determine whether module status matches user input
  if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
    site_status_inclusion="included"
  fi
}

function prepare_log_file {
  # if not log directory exists, create one
  if [ ! -d log ]; then mkdir log; fi
  echo "sitename,overall inclusion,default path,all path,status selection,status inclusion,module status,difference selection,difference inclusion,code comparison,default version,all version,code difference" >> log/delete-modules-$module_input-$timestamp.csv
}

function save_results_to_log {
  echo "$sitename,$site_inclusion,$module_default_path,$module_all_path,$status_selection,$site_status_inclusion,$module_status,$difference_selection,$site_difference_inclusion,$code_comparison,$sites_default_version,$sites_all_version,$module_code_difference" >> log/delete-modules-$module_input-$timestamp.csv
}

function generate_sites_options {
  sites_options=()
  save_list_of_sites_on_server

  # Prepare log file
  echo "Site," >> log/delete-modules-$module_input-$timestamp.csv

  # Report review progress to user
  counter=0
  number_of_sites=$(wc -l < sws_sites_list.csv | sed 's/ //g')

  # Loop through the sites found in sites_list file
  while read sitename; do
    (( counter++ ))
    echo "$counter $sitename of $number_of_sites sites to check"

    find_module_paths
    compare_module_paths

    # find_module_status
    module_status=$(drush @$server_alias.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    find_module_versions
    compare_module_versions

    reduce_list_by_status
    reduce_list_by_difference

    # if status AND difference have both been found in user input, consider the site a candidate
    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )) && (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      sites_options+=( ${sitename//\"} "" off// )
      site_inclusion="included"
    fi

    save_results_to_log
  done < /afs/ir/group/webservices/tools/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv
}
