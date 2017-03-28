#!/bin/bash

source includes/common.inc

function generate_sites_options {
  # if not log directory exists, create one
  if [ ! -d log ]; then mkdir log; fi

  # save to a file all sites on the specified server
  if [ -z "$sws_sites_list_path" ]; then
    find $server_webroot -type d -mindepth 1 -maxdepth 1 -exec basename {} \; > sws_sites_list.csv
  fi
  sites_options=()

  # log user input
  echo "status selected: $status_selection" >> log/delete_modules_$timestamp.log
  echo "difference selected: $difference_selection" >> log/delete_modules_$timestamp.log

  # report progress to user
  counter=0
  number_of_sites=$(wc -l < sws_sites_list.csv | sed 's/ //g')

  # loop through the sites found on this server
  while read sitename; do
    (( counter++ ))
    echo "$counter of $number_of_sites sites to check"

    # update sitename to include suffix if specified
    if [ ! -z "$site_suffix" ]; then sitename="$sitename/$site_suffix"

    # save module paths for both sites/default and sites/all
    module_all_path=$(find $server_webroot/$sitename/sites/all/modules -type d -name "$module_input")
    module_default_path=$(find $server_webroot/$sitename/sites/default/modules -type d -name "$module_input")

    # save all information used to determine eligibility
    echo "sitename: $sitename" >> log/delete_modules_$timestamp.log
    echo "all path: $module_all_path" >> log/delete_modules_$timestamp.log
    echo "default path: $module_default_path" >> log/delete_modules_$timestamp.log

    # skip to the next iteration, if module was not found in sites/default
    [ -z "$module_default_path" ] && continue

    # save module status
    module_status=$(drush @$server.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    # start evaluating differences found between sites/default and sites/all
    if [ -z "$module_all_path" ]; then module_difference_summary="not_in_sites_all"; fi

    # only check module versions if the user selected default_older
    if ((`in_array "default_older" "${difference_selection[@]}"` == 1 )); then

      # determine module version only if the same module was found in both sites/default and sites/all
      if [ ! -z "$module_all_path" ] && [ ! -z "$module_default_path" ]; then
	sites_all_version=$(grep version $module_all_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
	sites_default_version=$(grep version $module_default_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
      fi

      # let's assume versions can be operated on as floats
      # so no handling of -dev, -alpha, etc. releases
      # handling of these stability related suffixes would require
      # pattern matching the strings below, weighing them, and then
      # finding and comparing and numbers that follow, ie. -alpha2e
      # try: if the numbers match, then look at these suffixes.
      declare A drupal_module_verson_suffix=(
	["dev"]=""
	["rc"]=""
	["alpha"]=""
	["beta"]=""
      )

      # evaluate version differences between sites/default and sites/all
      if (( `echo "$sites_default_version < $sites_all_version" | bc` == 1 )); then
	module_difference_summary="default_older"
      elif (( `echo "$sites_default_version > $sites_all_version" | bc` == 1 )); then
	module_difference_summary="default_newer"
      elif (( `echo "$sites_default_version == $sites_all_version" | bc` == 1 )); then
	echo "module versions might be equal" >> log/delete_modules_$timestamp.log
      else
	echo "not able to compare module versions" >> log/delete_modules_$timestamp.log
      fi

      # save findings to log
      echo "sites all version: $sites_all_version" >> log/delete_modules_$timestamp.log
      echo "sites default version: $sites_default_version" >> log/delete_modules_$timestamp.log
fi

      module_code_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path" 2> /dev/null)
      # check this only if we've already established an equality in module versions
      if [ ! -z "$module_code_difference" ]; then
	module_difference_summary="same_version_code_differs"
      else
	module_difference_summary="matches"


    echo "module code difference: $module_code_difference" >> log/delete_modules_$timestamp.log
    echo "module status: $module_status" >> log/delete_modules_$timestamp.log
    echo "module difference summary: $module_difference_summary" >> log/delete_modules_$timestamp.log

    # determine whether module status matches user input
    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
      echo "status in array" >> log/delete_modules_$timestamp.log
    else
      echo "status not in array" >> log/delete_modules_$timestamp.log
    fi

    # determine whether module differences match user input
    if (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      echo "difference in array" >> log/delete_modules_$timestamp.log
    else
      echo "difference not in array" >> log/delete_modules_$timestamp.log
    fi

    # if status AND difference have both been found in user input, consider the site a candidate
    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )) && (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      sites_options+=( ${sitename//\"} "" off// )
      echo "$sitename included in sites_options array" >> log/delete_modules_$timestamp.log
    fi
  done < /Users/kelliebrownell/Sites/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv
}
