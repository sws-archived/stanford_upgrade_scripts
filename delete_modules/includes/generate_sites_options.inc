#!/bin/bash

source includes/common.inc
source variables.inc

function save_list_of_sites_on_server {
  if [ -z "$sws_sites_list_path" ]; then
    find $server_webroot -mindepth 1 -maxdepth 1 -type d -exec basename {} \; > sws_sites_list.csv
    sws_sites_list_path=$(echo "`pwd`/sws_sites_list.csv")
  fi
}

function find_module_paths {
  # Update sitename to include suffix if specified
  if [ ! -z "$sitename_suffix" ]; then
    sitename_with_suffix="$sitename/$sitename_suffix"
  else
    sitename_with_suffix="$sitename"
  fi

  # Save module paths for both sites/default and sites/all
  module_all_path=$(find $server_webroot/$sitename_with_suffix/sites/all/modules/* -mindepth 1 -maxdepth 1 -type d -name $module_input 2>/dev/null)
  module_default_path=$(find $server_webroot/$sitename_with_suffix/sites/default/modules/* -mindepth 1 -maxdepth 1 -type d -name $module_input 2>/dev/null)
}

function compare_module_versions {
  # Evaluate version differences between sites/default and sites/all
  if echo $sites_default_version $sites_all_version | awk '{exit !($1 < $2)}'; then
    code_comparison="default_older"
  else
    code_comparison="code_differs"
  fi
}

function find_module_versions {
  # Assumes function will only be called if the module is present in both sites/default and sites/all
  sites_all_version=$(grep version $module_all_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
  sites_default_version=$(grep version $module_default_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
}

function find_module_code_differences {
  module_code_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path" 2> /dev/null)
  if [ -z "$module_code_difference" ]; then
    code_comparison="matches"
  else
    find_module_versions
    compare_module_versions
  fi
}

function compare_module_paths {
  # Skip to the next iteration, if module was not found in sites/default
  [ -z "$module_default_path" ] && continue
  if [ -z "$module_all_path" ]; then
    code_comparison="not_in_sites_all"
  else
    find_module_code_differences
  fi
}

function reduce_list_by_difference {
  # determine whether module differences match user input
  if (( `in_array "$code_comparison" "${difference_selection[@]}"` == 1 )); then
    site_difference_inclusion="included"
  else
    site_difference_inclusion="excluded"
  fi
}

function reduce_list_by_status {
  # determine whether module status matches user input
  if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
    site_status_inclusion="included"
  else
    site_status_inclusion="excluded"
  fi
}

function prepare_log_file {
  # if not log directory exists, create one
  if [ ! -d log ]; then mkdir log; fi
  echo "sitename,overall inclusion,default path,all path,status selection,status inclusion,module status,difference selection,difference inclusion,code comparison,default version,all version,diff" >> log/delete-modules-$module_input-$timestamp--selection.csv
}

function save_results_to_log {
  echo "$sitename,$site_inclusion,$module_default_path,$module_all_path,$status_selection,$site_status_inclusion,$module_status,$difference_selection,$site_difference_inclusion,$code_comparison,$sites_default_version,$sites_all_version,$module_code_difference" >> log/delete-modules-$module_input-$timestamp--selection.csv
}

function generate_sites_options {
  sites_options=()
  save_list_of_sites_on_server

  # Report review progress to user
  counter=0
  number_of_sites=$(wc -l < sws_sites_list.csv | sed 's/ //g')

  # Loop through the sites found in sites_list file
  for sitename in $(cat $sws_sites_list_path); do
    (( counter++ ))
    echo "$counter $sitename of $number_of_sites sites to check"

    # finding and comparison module versions and code differences happens within compare_module_paths
    find_module_paths
    compare_module_paths

    # find_module_status
    module_status=$(drush @$server_alias.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    reduce_list_by_status
    reduce_list_by_difference

    # if status AND difference have both been found in user input, consider the site a candidate
    if [ "$site_status_inclusion" == "included" ] && [ "$site_difference_inclusion" == "included" ]; then
      sites_options+=( ${sitename//\"} "$module_status | $code_comparison" off// )
      site_inclusion="included"
    fi

    save_results_to_log
  done
}
