#!/bin/bash

source includes/common.inc
source variables.inc

function generate_sites_options {
  # if not log directory exists, create one
  if [ ! -d log ]; then mkdir log; fi

  # save to a file all sites on the specified server
  if [ -z "$sws_sites_list_path" ]; then
    find $server_webroot -type d -mindepth 1 -maxdepth 1 -exec basename {} \; > sws_sites_list.csv
  fi
  sites_options=()

  # log user input
  echo "status selected: $status_selection" >> log/delete-modules-$module_input-$timestamp.log
  echo "difference selected: $difference_selection" >> log/delete-modules-$module_input-$timestamp.log

  # report progress to user
  counter=0
  number_of_sites=$(wc -l < sws_sites_list.csv | sed 's/ //g')

  # loop through the sites found on this server
  while read sitename; do
    (( counter++ ))

    echo "$counter $sitename of $number_of_sites sites to check"

    # update sitename to include suffix if specified
    if [ ! -z "$sitename_suffix" ]; then sitename_with_suffix="$sitename/$sitename_suffix"; fi

    # save module paths for both sites/default and sites/all
    module_all_path=$(find $server_webroot/$sitename_with_suffix/sites/all/modules/*/ -type d -name "$module_input")
    module_default_path=$(find $server_webroot/$sitename_with_suffix/sites/default/modules/*/ -type d -name "$module_input")

    # save all information used to determine eligibility
    echo "sitename: $sitename" >> log/delete-modules-$module_input-$timestamp.log
    echo "all path: $module_all_path" >> log/delete-modules-$module_input-$timestamp.log
    echo "default path: $module_default_path" >> log/delete-modules-$module_input-$timestamp.log

    # skip to the next iteration, if module was not found in sites/default
    [ -z "$module_default_path" ] && continue

    # save module status
    module_status=$(drush @$server_alias.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    # start evaluating differences between sites/default and sites/all
    if [ -z "$module_all_path" ]; then module_difference_summary="not_in_sites_all"; fi

    # determine module version only if the same module was found in both sites/default and sites/all
    if [ ! -z "$module_all_path" ] && [ ! -z "$module_default_path" ]; then
      sites_all_version=$(grep version $module_all_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
      sites_default_version=$(grep version $module_default_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
    fi

    # evaluate version differences between sites/default and sites/all
    if (( $sites_default_version < $sites_all_version )); then
      module_difference_summary="default_older"
    elif (( $sites_default_version == $sites_all_version )); then
      echo "module versions might be equal" >> log/delete-modules-$module_input-$timestamp.log
      module_code_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path" 2> /dev/null)
      # check this only if we've already established an equality in module versions
      if [ ! -z "$module_code_difference" ]; then
	module_difference_summary="same_version_code_differs"
      elif [ ! -z "$module_all_path" ]; then
	module_difference_summary="matches"
      fi
    else
      echo "not able to compare module versions" >> log/delete-modules-$module_input-$timestamp.log
    fi

    # save findings to log
    echo "sites all version: $sites_all_version" >> log/delete-modules-$module_input-$timestamp.log
    echo "sites default version: $sites_default_version" >> log/delete-modules-$module_input-$timestamp.log

    echo "module code difference: $module_code_difference" >> log/delete-modules-$module_input-$timestamp.log
    echo "module status: $module_status" >> log/delete-modules-$module_input-$timestamp.log
    echo "module difference summary: $module_difference_summary" >> log/delete-modules-$module_input-$timestamp.log

    # determine whether module status matches user input
    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
      echo "status in array" >> log/delete-modules-$module_input-$timestamp.log
    else
      echo "status not in array" >> log/delete-modules-$module_input-$timestamp.log
    fi

    # determine whether module differences match user input
    if (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      echo "difference in array" >> log/delete-modules-$module_input-$timestamp.log
    else
      echo "difference not in array" >> log/delete-modules-$module_input-$timestamp.log
    fi

    # if status AND difference have both been found in user input, consider the site a candidate
    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )) && (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      sites_options+=( ${sitename//\"} "" off// )
      echo "$sitename included in sites_options array" >> log/delete-modules-$module_input-$timestamp.log
    fi
  done < /afs/ir/group/webservices/tools/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv
}
