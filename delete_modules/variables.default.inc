#!/bin/bash

# archives below this file size will be considered unreliable
minimum_size=9000
sws_sites_list_path="/afs/ir/group/webservices/tools/stanford_upgrade_scripts/sws_sites_list.csv"

# used for drush alias
expected_hostname="sites-uat.stanford.edu"
server_alias="suat"
script_path="/afs/ir/group/webservices/tools/stanford_upgrade_scripts/delete_modules"
# absolute path to server's webroot, no trailing slash
server_webroot="/var/www"
# example of a sitename suffix might be public_html on sites1/2 servers
sitename_suffix="public_html"
