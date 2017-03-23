#!/bin/bash

source includes/common.inc
source includes/generate_sites_options.inc
source includes/remove_module_from_selected_sites.inc

timestamp=$(date +%Y%m%d%H%M%S)
minimum_size=9000

environment="dev"
sitename_prefix="/Users/kelliebrownell/Sites"
# Example of a sitename suffix might be public_html on sites1/2 servers
sitename_suffix=""

module_input=$(whiptail --title "Delete a module in sites/default" --inputbox "Which module in sites/default would you like to delete?" 10 60 3>&1 1>&2 2>&3)
check_exit_status

status_selection=$(whiptail --title "Module Status" --checklist "Only delete sites/default modules with the following status(es).  Note: we do not delete enabled modules." --notags 15 60 3 \
not_installed "" on \
disabled "" off \
enabled "" off 3>&1 1>&2 2>&3)
check_exit_status

difference_selection=$(whiptail --title "sites/default Difference" --checklist "Only delete sites/default modules with the following differences from sites/all/modules." --notags 15 60 5 \
matches "" off \
not_in_sites_all "" off \
default_older "" off \
default_newer "" off \
same_version_code_differs "" off 3>&1 1>&2 2>&3)
check_exit_status

generate_sites_options
if [ -z "${sites_options[*]}" ]; then
  echo "No sites meet your criteria" && exit
else
  sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection." 25 60 "${#sites_options[@]}" "${sites_options[@]}" --notags 3>&1 1>&2 2>&3)
  check_exit_status
fi

whiptail --title "Confirmation" --yes-button "PROCEED" --no-button "Cancel"  --yesno "Please confirm that you would like to delete $module_input from ${sites_selection[*]}.  Only if its status is ${status_selection[*]} and difference is ${difference_selection[*]}." 10 60 3>&1 1>&2 2>&3
check_exit_status

# Double check that the user chose PROCEED
if [ "$exitstatus" == 0 ]; then
  for site in "${sites_selection[@]}"; do
    # remove quotes
    site=$(echo $site | sed -e 's/^"//' -e 's/"$//')
    archive_site
    uninstall_module
    delete_uninstalled_module
    check_site_loads
  done
fi
