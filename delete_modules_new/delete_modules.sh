#!/bin/bash

source includes/generate_sites_options.sh

# generate sws_sites_list.csv


timestamp=$(date +%Y%m%d%H%M%S)
minimum_size=9000

status_options=("not_installed" "disabled" "enabled")
difference_options=("matches" "not_in_sites_all" "code_differs")

environment="dev"
sitename_prefix="/Users/kelliebrownell/Sites"
# Example of a sitename suffix might be public_html on sites1/2 servers
sitename_suffix=""

module_input=$(whiptail --title "Delete a module in sites/default" --inputbox "Which module in sites/default would you like to delete?" 10 60 3>&1 1>&2 2>&3)
exitstatus=$?
if [ $existatus == 0 ]; then

  # select status
  status_selection=$(whiptail --title "Module Status" --checklist "Only delete sites/default modules with the following status(es).  Note: we do not delete enabled modules." 15 60 2 "${status_options[@]}" 3>&1 1>&2 2>&3)
  if [ $exitstatus == 0 ]; then
    echo "${status_selection[*]}"
  else
    exit
  fi

  # select difference
  difference_selection=$(whiptail --title "sites/default Difference" --checklist "Only delete sites/default modules with the following differences from sites/all/modules." 15 60 3 "${difference_options[@]}" 3>&1 1>&2 2>&3)
  if [ $exitstatus == 0 ]; then
    echo "${difference_selection[*]}"
  else
    exit
  fi

  # select sites
  generate_sites_options
  sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection." 25 60 10 "${sites_options[@]}" 3>&1 1>&2 2>&3)
  if [ $exitstatus == 0 ]; then
    echo "${sites_selection[*]}"
  else
    exit
  fi

else
  exit
fi
