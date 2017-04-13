#!/bin/bash

source includes/common.inc

# Warn user when a site does not appear to have our theme's Stanford logo
function check_site_loads {
  site_for_url=$(echo $site | sed -e 's/ds_//')
  site_url="https://$expected_hostname/$site_for_url/"
  curl_result=$(curl -L --silent https://sites1.stanford.edu/jumpstart-chembio/ | grep 'top-logo')
  if [ -z "$curl_result" ]; then
    echo "WARNING: Not able to determine whether the site is loading.  Visit $site_url to see how it's doing."
    echo "Check: fail" >> log/delete-modules-$module_input-$timestamp--deletion.log
  else
    echo "Check: success" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi
}

function check_module_deleted {
  # Record if module still appears in sites/default after attempting to delete it
  if [ -d "$module_default_path" ]; then
    echo "ERROR: failed to delete $module_default_path" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi
}

# Delete module, assuming archive has saved successfully
function delete_module_from_sites_default {
  # Find path to sites/default module
  module_default_path=$(find $server_webroot/$site_with_suffix/sites/default/modules/* -mindepth 1 -maxdepth 1 -type d -name "$module_input")

  # Only remove directory if path includes module name
  if [ ! -z "$module_default_path" ] && [[ "$module_default_path" == *"$module_input"* ]] && [[ "$archive_status" == "success" ]]; then
    rm -rf "$module_default_path"
    echo "Deleted: $module_default_path" >> log/delete-modules-$module_input-$timestamp--deletion.log

    # Clear cache, rebuild registry and update database
    drush @$server_alias.$site rr
    drush @$server_alias.$site updb -y
    drush @$server_alias.$site cc all

    check_site_loads
    check_module_deleted
  else
    echo "Skip: module not deleted, no path found" >> log/delete-modules-$module_input-$timestamp--deletion.log
  fi
}

function check_archive_size {
  archive_file_size=$(wc -c < "$site_archive_directory/$site/delete-modules-$module_input-$timestamp.sql")
  echo "File size: $archive_file_size."
  if (( $archive_file_size > $minimum_size )); then
    archive_status="success"
    echo "Archive path: $site_archive_file" >> log/delete-modules-$module_input-$timestamp--deletion.log
  else
    archive_status="suspicious"
    echo "The archive might not have saved correctly; its filesize is quite low. Do you want to skip this site? " -n 1 -r
  fi
}

# Archive site and make sure resulting file meets the user specified threshold
function archive_site_and_delete_module_upon_success {
  module_default_path=$(find $server_webroot/$site_with_suffix/sites/default/modules/* -mindepth 1 -maxdepth 1 -type d -name "$module_input")
  if [ ! -d "$site_archive_directory/$site" ]; then mkdir $site_archive_directory/$site; fi
  cp -r $module_default_path $site_archive_directory/$site/delete-modules-$module_input-$timestamp
  if [ ! -d "$site_archive_directory/$site/delete-modules-$module_input-$timestamp" ]; then
    echo "Skip: Copy of module appears to have failed." && continue
    archive_status="failed"
  fi

  drush @$server_alias.$site sql-dump --result-file="$site_archive_directory/$site/delete-modules-$module_input-$timestamp.sql"
  if [ -f "$site_archive_directory/$site/delete-modules-$module_input-$timestamp.sql" ]; then
    check_archive_size
  else
    echo "Skip: Database backup appears to have failed."
    archive_status="failed"
  fi
  if [[ "$archive_status" == "success" ]]; then
    delete_module_from_sites_default
  fi
  echo "Archive status: $archive_status" >> log/delete-modules-$module_input-$timestamp--deletion.log
}
