#!/bin/bash

source includes/common.inc

# Warn user when a site does not appear to have our theme's Stanford logo
function check_site_loads {
  site_for_url=$(echo $site | sed -e 's/ds_//')
  site_url="https://$expected_hostname/$site_for_url/"
  curl_result=$(curl --silent $site_url | grep '<div id="top-logo" class="span4"><a href="http://www.stanford.edu">')
  if [ -z "$curl_result" ]; then
    echo "WARNING: Not able to determine whether the site is loading.  Visit $site_url to see how it's doing."
    eco "Check: fail" >> log/delete-modules-$module_input-$timestamp--deletion.log
  else
    echo "Check: success" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi
}

# Archive site and make sure resulting file meets the user specified threshold
function archive_site {
  site_archive_path="/afs/ir/group/webservices/tmp/delete-modules-$site-$timestamp.tar.gz"
  echo "Archiving $site."
  drush @$server_alias.$site ard --destination="$site_archive_path"
  echo "Checking file size."
  if (( `wc -c < "$site_archive_path"` < $minimum_size )); then
    echo "The archive might not have saved correctly; its filesize is quite low. Do you want to skip this site? " -n 1 -r
    echo "\n"
    [[ $REPLY =~ ^[Yy]$ ]] && echo "Skip: archive suspcious" >> log/delete-modules-$module_input-$timestamp--deletion.log && continue

  fi
  echo $site_archive_path
}

# Delete module, assuming archive has saved successfully
function delete_module_from_sites_default {
  # Find path to sites/default module
  module_default_path=$(find $server_webroot/$site_with_suffix/sites/default/modules/*/ -mindepth 1 -maxdepth 1 -type d -name "$module_input")

  # Only remove directory if path includes module name
  if [ ! -z "$module_default_path" ] && [[ "$module_default_path" == *"$module_input"* ]]; then
    rm -rf "$module_default_path"
    echo "Deleted: $module_default_path" >> log/delete-modules-$module_input-$timestamp--deletion.log
  else
    echo "Skip: module not deleted, no path found" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi

  # Clear cache, rebuild registry and update database
  drush @$server_alias.$site rr
  drush @$server_alias.$site updb -y
  drush @$server_alias.$site cc all

  # Record if module still appears in sites/default after attempting to delete it
  if [ -d "$module_default_path" ]; then
    echo "ERROR: failed to delete $module_default_path" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi
}
