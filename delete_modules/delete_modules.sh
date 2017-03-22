#!/bin/bash

source includes/generate_sites_options.inc

timestamp=$(date +%Y%m%d%H%M%S)
minimum_size=9000

environment="dev"
sitename_prefix="/Users/kelliebrownell/Sites"
# Example of a sitename suffix might be public_html on sites1/2 servers
sitename_suffix=""

module_input=$(whiptail --title "Delete a module in sites/default" --inputbox "Which module in sites/default would you like to delete?" 10 60 3>&1 1>&2 2>&3)
status_selection=$(whiptail --title "Module Status" --checklist "Only delete sites/default modules with the following status(es).  Note: we do not delete enabled modules." --notags 15 60 3 \
not_installed "" on \
disabled "" off \
enabled "" off 3>&1 1>&2 2>&3)

difference_selection=$(whiptail --title "sites/default Difference" --checklist "Only delete sites/default modules with the following differences from sites/all/modules." --notags 15 60 3 \
matches "" off \
not_in_sites_all "" off \
code_differs "" off 3>&1 1>&2 2>&3)

generate_sites_options
sites_selection=$(whiptail --title "Select Sites" --checklist "Only delete the sites/default copy of $module_input from the following sites. Press <space> to make your selection." 25 60 "${#sites_options[@]}" "${sites_options[@]}" --notags 3>&1 1>&2 2>&3)
