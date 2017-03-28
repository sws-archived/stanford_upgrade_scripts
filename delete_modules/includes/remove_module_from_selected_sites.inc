#!/bin/bash

source includes/common.inc

# give user the option of restoring site from most recent backup if it
# does not appear to be loading
function check_site_loads {
  site_url="https://$server.stanford.edu/$site"
  curl_result=`curl -v --silent http://$site_url/ 2>&1 | grep stanford`
  if [ -z "$curl_result" ]; then
    echo "WARNING: not able to determine whether the site is loading.  Visit $site_url to see how it's doing." 2>&1 | tee log/delete_modules_$module_input_$timestamp.log
  fi
}

# take an archive of the site and make sure resulting file is not empty
function archive_site {
  site_archive_path="/afs/ir/group/webservices/tmp/delete-modules-$site-$timestamp.tar.gz"
  echo "archiving $site"
  drush @$server_alias.$site ard --destination="$site_archive_path"
  echo "checking file size"
  if (( `wc -c < "$site_archive_path"` < $minimum_size )); then
    echo "The archive might not have saved correctly; its filesize is quite low. Do you want to skip this site? " -n 1 -r
    echo "\n"
    [[ $REPLY =~ ^[Yy]$ ]] && echo "archive file size suspiciously low, $module_input not deleted on $site." >> log/delete_modules_$module_input_$timestamp.log && continue

  fi
  echo $site_archive_path
}

# delete module, assuming archive has been saved and module uninstalled
function delete_uninstalled_module {
  module_default_path=$(find $server_webroot/$site/$sitename_suffix/sites/default/modules -type d -name "$module_input")
  echo "rm -rf $module_default_path"
  if [ ! -z "$module_default_path" ]; then rm -rf "$module_default_path"; fi
  drush @$server_alias.$site rr
  drush @$server_alias.$site dbup -y
  if [ -d "$module_default_path" ]; then
    echo "no sites/default path found, $module_input not deleted on $site." >> log/delete_modules_$module_input_$timestamp.log
  fi
}
