#!/bin/bash

source includes/common.inc

# give user the option of restoring site from most recent backup if it
# does not appear to be loading
function check_site_loads {
  curl_result=`curl -v --silent http://$site.dev/ 2>&1 | grep stanford`
  if [ -z "$curl_result" ]; then
    read -p "Site does not appear to be loading.  If this is true, do you want to restore the archive? " -n 1 -r
    echo "\n"
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      echo "drush @$server.$site arr $site_archive_path --destination=$server_webroot/$site/$sitename_suffix"
      drush @$server.$site arr "$site_archive_path" --destination="$server_webroot/$site/$sitename_suffix"
    else
      echo "Skipping site." && continue
    fi
  fi
}

# take an archive of the site and make sure resulting file is not empty
function archive_site {
  site_archive_path="/tmp/delete-modules-$site-$timestamp.tar.gz"
  echo "archiving $site"
  drush @dev.$site ard --destination="$site_archive_path"
  echo "checking file size"
  if (( `wc -c < "$site_archive_path"` < $minimum_size )); then
    echo "The archive might not have saved correctly; its filesize is quite low. Do you want to skip this site? " -n 1 -r
    echo "\n"
    [[ $REPLY =~ ^[Yy]$ ]] && echo "archive file size suspiciously low, $module_input not deleted on $site." >> log/delete_modules_$timestamp.log && continue

  fi
  echo $site_archive_path
}

# disable and uninstall module before proceeding
function uninstall_module {
  prelog/delete_modules_status=$(drush @$server.$site pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')
  if [ "$prelog/delete_modules_status" == "disabled" ]; then
    drush @$server.$site pm-uninstall -y $module
  elif [ "$prelog/delete_modules_status" == "enabled" ]; then
    drush @$server.$site dis -y $module
    drush @$server.$site pm-uninstall -y $module
  fi
  # drush @$server.$site rr
  drush @$server.$site cc all
  # drush updb ?
  final_module_status=$(drush @server.$site pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')
  [ "$final_module_status" != "not_installed" ] && echo "skipping, $module_input not yet uninstalled on $site"  >> log/delete_modules_$timestamp.log && continue
}

# delete module, assuming archive has been saved and module uninstalled
function delete_uninstalled_module {
  module_default_path=$(find $server_webroot/$site/$sitename_suffix/sites/default/modules -type d -name "$module_input")
  echo "rm -rf $module_default_path"
  if [ ! -z "$module_default_path" ]; then rm -rf "$module_default_path"; fi
  if [ -d "$module_default_path" ]; then
    echo "no sites/default path found, $module_input not deleted on $site." >> log/delete_modules_$timestamp.log
  fi
}
