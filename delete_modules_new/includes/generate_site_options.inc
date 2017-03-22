#!/bin/bash

source common.inc

# Assumes you have a list of all SWS sites.
# We should have the following variables,
# module_input: should be 1 module name
# status_selection: "not_installed" "disabled" "enabled"
# difference_selection: "matches" "not_in_sites_all" "code_differs"

# loop through a list of SWS sites
while read sitename; do
  module_all_path=$(`find ~/Sites/$sitename/sites/all/modules -type d -name "$module_input"`)
  module_default_path=$(`find ~/Sites/$sitename/sites/default/modules -type d -name "$module_input"`)
  module_difference=$(`diff -bqrd "$module_all_path" "$module_default_path" >/dev/null`)
  module_status=$(`drush @dev.$sitename pmi --fields=status --field-labels=0`)

  if [ -z "$module_all_path" ]; then module_difference="not_in_sites_all"; fi
  if [ -z "$module_difference" ]; then module_difference="code differs"; else module_difference="matches"; fi
  if (( `in_array "$module_status" "${status_selection[@]"` )) && (( `in_array "$module_difference" "${difference_selection[@]}"` )); then
    # If everything checks out for this site, add it to sites_options array
    sites_options+=("$sitename")
  fi
do < sws_sites_list.csv

function code_difference_between_modules {
  
}
