#!/bin/bash

source includes/common.inc

function generate_sites_options {
  # find ~/Sites -type d -mindepth 1 -maxdepth 1 -exec basename {} \; > sws_sites_list.csv
  sites_options=()
  echo "status selected: $status_selection" >> delete_modules_$timestamp.log
  echo "difference selected: $difference_selection" >> delete_modules_$timestamp.log

  counter=0
  number_of_sites=$(wc -l < sws_sites_list.csv | sed 's/ //g')
  # loop through a list of SWS sites
  while read sitename; do
    (( counter++ ))
    echo "$counter of $number_of_sites sites to check"

    module_all_path=$(find ~/Sites/$sitename/sites/all/modules -type d -name "$module_input")
    module_default_path=$(find ~/Sites/$sitename/sites/default/modules -type d -name "$module_input")

    echo "sitename: $sitename" >> delete_modules_$timestamp.log
    echo "all path: $module_all_path" >> delete_modules_$timestamp.log
    echo "default path: $module_default_path" >> delete_modules_$timestamp.log

    [ -z "$module_default_path" ] && continue
    module_status=$(drush @dev.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    if [ -z "$module_all_path" ]; then module_difference_summary="not_in_sites_all"; fi

    if [ ! -z "$module_all_path" ] && [ ! -z "$module_default_path" ]; then
      sites_all_version=$(grep version $module_all_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
      sites_default_version=$(grep version $module_default_path/$module_input.info | sed -e 's|version = \"\(.*\)\"|\1|' | cut -d "-" -f 2-)
    fi

    # let's assume versions can be operated on as floats
    # so no handling of -dev, -alpha, etc. releases
    if (( `echo "$sites_default_version < $sites_all_version" | bc` == 1 )); then
      module_difference_summary="default_older"
    elif (( `echo "$sites_default_version > $sites_all_version" | bc` == 1 )); then
      module_difference_summary="default_newer"
    elif (( `echo "$sites_default_version == $sites_all_version" | bc` == 1 )); then
      module_code_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path" 2> /dev/null)
      # check this only if we've already established an equality in module versions
      if [ ! -z "$module_code_difference" ]; then
	module_difference_summary="same_version_code_differs"
      else
	module_difference_summary="matches"
      fi
    else
      echo "not able to compare module versions" >> delete_modules_$timestamp.log
    fi

    echo "sites all version: $sites_all_version" >> delete_modules_$timestamp.log
    echo "sites default version: $sites_default_version" >> delete_modules_$timestamp.log
    echo "module code difference: $module_code_difference" >> delete_modules_$timestamp.log
    echo "module status: $module_status" >> delete_modules_$timestamp.log
    echo "module difference summary: $module_difference_summary" >> delete_modules_$timestamp.log


    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
      echo "status in array" >> delete_modules_$timestamp.log
    else
      echo "status not in array" >> delete_modules_$timestamp.log
    fi

    if (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      echo "difference in array" >> delete_modules_$timestamp.log
    else
      echo "difference not in array" >> delete_modules_$timestamp.log
    fi

    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )) && (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      # If everything checks out for this site, add it to sites_options array
      sites_options+=( ${sitename//\"} "" off// )
      echo "$sitename included in sites_options array" >> delete_modules_$timestamp.log
    fi
  done < ~/Sites/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv
}
