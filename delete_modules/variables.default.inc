#!/bin/bash

# This script was written with the assumption that you will
# be deleting modules in people1/2, sites1/2, sites-uat, or
# sites-dev.  Even so, there are variables specific to each server.
#
# MINIMUM_SIZE:
# Archives below this file size will be considered unreliable.
# Sites with unreliable archives will be skipped, and their
# sites/default modules NOT deleted.
# Example: 9000
#
# SWS_SITES_LIST_PATH:
# This file contains the sites you want to review for delete
# candidates.  If this variable is left blank, the script
# will assume any directories in your server's webroot should
# be reviewed for delete candidates. Please use the file's full path.
# Example: "/afs/ir/group/webservices/tools/stanford_upgrade_scripts/sws_sites_list.csv"
#
# SITE_ARCHIVE_PATH:
# Specify where the script should save site archives.
# Example: "/afs/ir/group/webservices/tmp"
#
# EXPECTED_HOSTNAME:
# This script is destructive.  Even when running it on sites-dev
# or sites-uat, you will be deleting the same site files (stored
# in AFS) that are currently being used on the live, production site.
# We ask you to provide an expected hostname, becase whether a site
# qualifies depends on the module's status, ie. enabled/disabled.
# And that may differ between environments.  If you accidentally run the
# script on sites-dev, you run the risk of deleting modules disabled on
# sites-dev but running the same shared code as enabled on sites.
# Example: "sites-uat.stanford.edu"
#
# SERVER_ALIAS:
# Your user will not able to access drush aliases available to the
# server.  If you do not already have drush aliases for sites servers,
# you'll want to save the template included in this repository to
# your AFS home directory, i.e. ~/.drush/aliases.drushrc.php. The prefix
# you choose must not conflict with the server's alias prefix.  For example,
# if you are on sites-uat, you cannot use @uat.
# Example: "suat" for sites-uat, "fsse" for sites1.
#
# SERVER_WEBROOT
# Include the full path to where sites on this server can be found.
# Example: "/var/www/"
#
# SITENAME_SUFFIX
# Sites on the Stanford Sites platform store their Drupal files within the
# public_html site directory.  For example, /var/www/ds_sws-uat-jsv/public_html
# is where you're going to find index.php, sites/default, sites/all, etc.
# If you are deleting modules from sites with this file structure, include
# the subdirectory name here.  Otherwise, leave empty.
# Example: "public_html"

minimum_size=9000
sws_sites_list_path="/afs/ir/group/webservices/tools/stanford_upgrade_scripts/delete_modules/sws_sites_list.csv"
site_archive_path="/afs/ir/group/webservices/tmp"
expected_hostname="sites-uat.stanford.edu"
server_alias="suat"
server_webroot="/var/www"
sitename_suffix="public_html"
