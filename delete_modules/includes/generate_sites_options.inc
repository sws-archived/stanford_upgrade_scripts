#!/bin/bash

source includes/common.inc

# Assumes you have a list of all SWS sites.
# We should have the following variables,
# module_input: should be 1 module name
# status_selection: "not_installed" "disabled" "enabled"
# difference_selection: "matches" "not_in_sites_all" "code_differs"

function generate_sites_options {
  # find ~/Sites -type d -mindepth 1 -maxdepth 1 -exec basename {} \; > sws_sites_list.csv
  sites_options=()
  echo "status selected: $status_selection"
  echo "difference selected: $difference_selection"

  # loop through a list of SWS sites
  while read sitename; do
    module_all_path=$(find ~/Sites/$sitename/sites/all/modules -type d -name "$module_input")
    module_default_path=$(find ~/Sites/$sitename/sites/default/modules -type d -name "$module_input")

    echo "sitename: $sitename"
    echo "all path: $module_all_path"
    echo "default path: $module_default_path"

    [ -z "$module_default_path" ] && continue
    module_status=$(drush @dev.$sitename pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')

    if [ -z "$module_all_path" ]; then module_difference_summary="not_in_sites_all"; fi

    if [ ! -z "$module_all_path" ] && [ ! -z "$module_default_path" ]; then
      module_difference=$(diff -bqrd --exclude=.git* "$module_all_path" "$module_default_path")
    fi

    if [ -z "$module_difference" ]; then module_difference_summary="matches"; else module_difference_summary="code_differs"; fi

    echo "module difference: $module_difference"
    echo "module status: $module_status"
    echo "module difference summary: $module_difference_summary"


    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )); then
      echo "status in array"
    else
      echo "status not in array"
    fi

    if (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      echo "difference in array"
    else
      echo "difference not in array"
    fi

    if (( `in_array "$module_status" "${status_selection[@]}"` == 1 )) && (( `in_array "$module_difference_summary" "${difference_selection[@]}"` == 1 )); then
      # If everything checks out for this site, add it to sites_options array
      sites_options+=( ${sitename//\"} "" off// )
    fi
  done < ~/Sites/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv
}
