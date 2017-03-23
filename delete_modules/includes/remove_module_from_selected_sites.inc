#!/bin/bash

source includes/common.inc

function check_site_loads {
  curl_result=`curl -v --silent http://biology.dev/ 2>&1 | grep stanford`
  if [ -z "$curl_result" ]; then
    site_loads_results="Site fails to load."
  else
    site_loads_results="Site loads."
  fi
  echo $site_loads_results
}

function remove_module_from_selected_sites {

}
