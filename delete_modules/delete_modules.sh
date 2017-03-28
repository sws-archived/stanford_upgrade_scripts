#!/bin/bash

source includes/common.inc
source includes/generate_sites_options.inc
source includes/remove_module_from_selected_sites.inc
source variables.inc


timestamp=$(date +%Y%m%d%H%M%S)
# which module do you want to delete?
module_input=$(whiptail --title "Delete a module in sites/default" --inputbox "Which module in sites/default would you like to delete?" 10 60 3>&1 1>&2 2>&3)
check_exit_status

# limit deletion of module to this status
status_selection=$(whiptail --title "Module Status" --checklist "Only delete sites/default modules with any (ie. OR) of the following status(es).  Note: we do not delete enabled modules." --notags 15 60 3 \
not_installed "" on \
disabled "" off \
enabled "" off 3>&1 1>&2 2>&3)
check_exit_status

# further limit deletion of module based on a comparison with the same module in sites/all
difference_selection=$(whiptail --title "sites/default Difference" --checklist "Only delete sites/default modules with any (ie. OR) of the following differences from sites/all/modules." --notags 15 60 5 \
matches "" off \
not_in_sites_all "" off \
default_older "" off \
same_version_code_differs "" off 3>&1 1>&2 2>&3)
check_exit_status

# returns a list of sites with a module in sites/default that matches the selection criteria
generate_sites_options
# users can limit the deletion of module to an arbitraty subset of sites meeting the selectoin criteria
if [ -z "${sites_options[*]}" ]; then
  echo "No sites meet your criteria" && exit
else
  sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection.  Sites appear on this list if they have met at least one of your status criteria AND at least one of your difference criteria." 25 60 "${#sites_options[@]}" "${sites_options[@]}" --notags 3>&1 1>&2 2>&3)
  check_exit_status
fi

whiptail --title "Confirmation" --yes-button "PROCEED" --no-button "Cancel"  --yesno "Please confirm that you would like to delete $module_input from ${sites_selection[*]}.  Only if its status is ${status_selection[*]} and difference is ${difference_selection[*]}." 10 60 3>&1 1>&2 2>&3
check_exit_status

# double check that the user chose PROCEED
if [ "$exitstatus" == 0 ]; then
  for site in "${sites_selection[@]}"; do
    # remove quotes
    site=$(echo $site | sed -e 's/^"//' -e 's/"$//')
    archive_site
    delete_uninstalled_module
    check_site_loads
  done
fi
