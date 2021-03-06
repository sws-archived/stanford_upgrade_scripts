#!/bin/bash -i

source includes/common.inc
source includes/generate_sites_options.inc
source includes/remove_module_from_selected_sites.inc
source variables.inc
timestamp=$(date +%Y%m%d%H%M%S)
datestamp=$(date +%Y%m%d)

# Check to be sure this script is being run on the server expected.
server_hostname=$(echo `hostname`)
if [[ "$expected_hostname" != "$server_hostname" ]]; then echo "server expected $expected_hostname and server hostname $server_hostname differ." && exit; fi

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

LINES=$(tput lines)
COLUMNS=$(tput cols)

# Just because the site's module matches a user's status and difference criteria, does not mean they wish
# the module to be deleted at this time.  Given users the option to delete the module from a subset of
# site candidates.
if [ -z "${sites_options[*]}" ]; then
  echo "No sites meet your criteria" && exit
else
  sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection.  Sites appear on this list if they have met at least one of your status criteria AND at least one of your difference criteria." 25 80 15 "${sites_options[@]}" --notags --scrolltext 3>&1 1>&2 2>&3)
  check_exit_status
fi

# This is a destructive process.  Forewarn the user that they have been given a second chance to check their work
# and should be prepared to take responsibility for any consequences (both good and bad).  For example, if a site
# fails to load after deleting the module, they'll want to contact the PM responsible for client communications and
# see whether the team agrees to attempt a restore of the site from backup.
authorized_by=$(whiptail --title "Authorization to Proceed" --inputbox "Please confirm that you would like to delete $module_input from ${sites_selection[*]} by submitting your SUNetID for the record." 10 60 3>&1 1>&2 2>&3)
check_exit_status

# If the user submits two members of the SWS developer team, go forward with deleting the sites/default version of their chosen module
# from all selected sites.  But first, we will save an archive of the site.  And afterwards, run a very simple
# check to be sure the site still loads.
sws_developers=("kbrownel" "jbickar" "sheamck" "pookmish" "ggarvey")
if (( `in_array "$authorized_by" "${sws_developers[@]}"` == 1 )) && [ "$authorized_by" == `whoami` ] ; then
  echo "Authorized by: $authorized_by" >> log/delete-modules-$module_input-$timestamp--deletion.log
  for site in `echo $sites_selection | sed -e 's/"//g'`; do
    if [ ! -z "$sitename_suffix" ]; then site_with_suffix="$site/$sitename_suffix"; else site_with_suffix="$site"; fi

    # Log deletion process
    echo "Site: $site" >> log/delete-modules-$module_input-$timestamp--deletion.log

    # Archive and delete module from sites/default
    archive_site_and_delete_module_upon_success
  done
else
  echo "You do not appear to be authorized to perform this action, or you have entered a SUNetID other than your own.  Please visit: https://ethics.stanford.edu."
  exit
fi
