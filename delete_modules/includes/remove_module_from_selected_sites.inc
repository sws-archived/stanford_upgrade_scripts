#!/bin/bash

source includes/common.inc

function check_site_loads {
  curl_result=`curl -v --silent http://$site.dev/ 2>&1 | grep stanford`
  if [ -z "$curl_result" ]; then
    read -p "Site does not appear to be loading.  If this is true, do you want to restore the archive? " -n 1 -r
    echo "\n"
    if [[ $REPLY =~ ^[Yy]$ ]]; then
      echo "drush @dev.$site arr $site_archive_path --destination=~/Sites/$site"
      drush @dev.$site arr "$site_archive_path" --destination="~/Sites/$site"
    else
      echo "Skipping site." && continue
    fi
  fi
}

function archive_site {
  site_archive_path="/tmp/delete-modules-$site-$timestamp.tar.gz"
  echo "archiving $site"
  drush @dev.$site ard --destination="$site_archive_path"
  echo "checking file size"
  if (( `wc -c < "$site_archive_path"` < $minimum_size )); then
    echo "The archive might not have saved correctly; its filesize is quite low."
  fi
  echo $site_archive_path
}

function uninstall_module {
  predelete_module_status=$(drush @dev.$site pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')
  if [ "$predelete_module_status" == "disabled" ]; then
    drush @dev.$site pm-uninstall -y $module
  elif [ "$predelete_module_status" == "enabled" ]; then
    drush @dev.$site dis -y $module
    drush @dev.$site pm-uninstall -y $module
  fi
  # drush @dev.$site rr
  drush @dev.$site cc all
  # drush updb ?
  final_module_status=$(drush @dev.$site pmi $module_input --format=list --fields=status --field-labels=0 | sed -e 's/ /_/')
  [ "$final_module_status" != "not_installed" ] && echo "skipping, module not yet uninstalled" && continue
}

function delete_uninstalled_module {
  module_default_path=$(find ~/Sites/$site/sites/default/modules -type d -name "$module_input")
  echo "rm -rf $module_default_path"
  if [ ! -z "$module_default_path" ]; then rm -rf "$module_default_path"; fi
  if [ -d "$module_default_path" ]; then
    echo "$module_input not deleted on $site."
  fi
}
