#!/bin/bash

source includes/common.inc
source includes/generate_sites_options.inc
source includes/remove_module_from_selected_sites.inc
source variables.inc
timestamp=$(date +%Y%m%d%H%M%S)

# Check to be sure this script is being run on the server expected.
server_hostname=$(echo `hostname`)
if [[ "$expected_hostname" != "$server_hostname" ]]; then echo "server expected $expected_hostname and server hostname $server_hostname differ."; fi

# Save user input on which module to delete.  Can only accept one module at this time.
module_input=$(whiptail --title "Delete a module in sites/default" --inputbox "Which module in sites/default would you like to delete?" 10 60 3>&1 1>&2 2>&3)
check_exit_status

# Save user input on which status of modules can be deleted.  Accepts multiple choices.
status_selection=$(whiptail --title "Module Status" --checklist "Only delete sites/default modules with any (ie. OR) of the following status(es).  Note: we do not delete enabled modules." --notags 15 60 3 \
not_installed "" on \
disabled "" off \
enabled "" off 3>&1 1>&2 2>&3)
check_exit_status

# Save user input on what difference to the same module in sites/all might qualify the module files in sites/default for deletion.
difference_selection=$(whiptail --title "sites/default Difference" --checklist "Only delete sites/default modules with any (ie. OR) of the following differences from sites/all/modules." --notags 15 60 5 \
matches "" off \
not_in_sites_all "" off \
default_older "" off \
code_differs "" off 3>&1 1>&2 2>&3)
check_exit_status

# Returns a list of sites with a module in sites/default that matches the selection criteria.
prepare_log_file
generate_sites_options

# Just because the site's module matches a user's status and difference criteria, does not mean they wish
# the module to be deleted at this time.  Given users the option to delete the module from a subset of
# site candidates.
if [ -z "${sites_options[*]}" ]; then
  echo "No sites meet your criteria" && exit
else
  sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection.  Sites appear on this list if they have met at least one of your status criteria AND at least one of your difference criteria." 25 60 "${#sites_options[@]}" "${sites_options[@]}" --notags 3>&1 1>&2 2>&3)
  check_exit_status
fi

# This is a destructive process.  Forewarn the user that they have been given a second chance to check their work
# and should be prepared to take responsibility for any consequences (both good and bad).  For example, if a site
# fails to load after deleting the module, they'll want to contact the PM responsible for client communications and
# see whether the team agrees to attempt a restore of the site from backup.
whiptail --title "Confirmation" --yes-button "PROCEED" --no-button "Cancel"  --yesno "Please confirm that you would like to delete $module_input from ${sites_selection[*]}.  Only if its status is ${status_selection[*]} and difference is ${difference_selection[*]}." 10 60 3>&1 1>&2 2>&3
check_exit_status

# If the user selects PROCEED, go forward with deleting the sites/default version of their chosen module
# from all selected sites.  But first, we will save an archive of the site.  And afterwards, run a very simple
# check to be sure the site still loads.
if [ "$exitstatus" == 0 ]; then
  for site in "${sites_selection[@]}"; do
    # remove quotes from sites_selection values
    site=$(echo $site | sed -e 's/^"//' -e 's/"$//')
    if [ ! -z "$sitename_suffix" ]; then site_with_suffix="$site/$sitename_suffix"; else site_with_suffix="$site"; fi

    # Log deletion process
    echo "Site: $site" >> log/delete-modules-$module_input-$timestamp--deletion.log

    # Archive and delete module from sites/default
    archive_site
    delete_module_from_sites_default
    check_site_loads
    echo "\n" >> log/delete-modules-$module_input-$timestamp--deletion.log
  done
fi
