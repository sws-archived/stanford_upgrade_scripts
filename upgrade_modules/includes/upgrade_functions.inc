#!/bin/bash

source includes/common.inc

function find_module_locations {
  if [[ "$module_name" =~ .*stanford.* ]]; then
    grep_pattern="\[$module_name\]\[download\]\[tag\] \= .*"
  elif [[ "$module_name" =~ .*open_framework.* ]]; then
    grep_pattern="\[$module_name\]\[download\]\[tag\] \= .*"
  else
    grep_pattern="\[$module_name\]\[version\] \= .*"
  fi
  grep -r "$grep_pattern" . | while read -r module_location; do
    parse_module_location_results
  done
}

function parse_module_location_results {
  module_version=$(echo $module_location | sed 's|.*=||' | sed 's|"||g' | sed 's| ||g')
  base_directory=$(echo $module_location | sed 's|^./||;s|/.*||')
  if [[ "$base_directory" != "make" ]]; then
    product=$(echo $module_location | sed "s|^./$base_directory/product/||;s|/.*||")
  else
    product="self-service"
  fi
  echo "$module_name,$module_version,$base_directory,$product" >> ~/Sites/stanford_upgrade_scripts/upgrade_modules/module_locations.csv
  if ! grep -Fxq "$module_name,$module_version" ~/Sites/stanford_upgrade_scripts/upgrade_modules/modules_in_use.csv; then
    echo "$module_name,$module_version" >> ~/Sites/stanford_upgrade_scripts/upgrade_modules/modules_in_use.csv
  fi
}

function find_modules_in_profiles {
  while read module_name; do
    for profile in ${!stanford_profiles[@]}; do
      cd ~/Sites/$profile
      find_module_locations
    done
  done < ~/Sites/stanford_upgrade_scripts/upgrade_modules/module_list.csv
}

function find_replace_module_versions {
  if [[ $module_name == *"stanford"* ]]; then
    find . -name "*.make" -exec sed -i '' 's/'$module_name'\]\[download\]\[tag\] \= \"'$current_version'\"/'$module_name'\]\[download\]\[tag\] \= \"'$new_version'\"/g' "{}" \;
  elif [[ $module_name == *"open_framework"* ]]; then
    find . -name "*.make" -exec sed -i '' 's/'$module_name'\]\[download\]\[tag\] \= \"'$current_version'\"/'$module_name'\]\[download\]\[tag\] \= \"'$new_version'\"/g' "{}" \;
  else
    find . -name "*.make" -exec sed -i '' 's/'$module_name'\]\[version\] \= \"'$current_version'\"/'$module_name'\]\[version\] \= \"'$new_version'\"/g' "{}" \;
  fi
}

function update_stanford_profiles {
  pwd
  for profile in ${!stanford_profiles[@]}; do
    profile_master_branch="${stanford_profiles[$profile]}"
    cd ~/Sites/$profile
    git checkout $profile_master_branch
    git pull
    if [ `git branch --list "$new_uat_branch"` ]; then
      echo "git checkout $new_uat_branch"
      git checkout $new_uat_branch
    else
      echo "git checkout -b $new_uat_branch"
      git checkout -b $new_uat_branch
    fi
    while IFS=, read module_name current_version new_version; do
      find_replace_module_versions
    done < ~/Sites/stanford_upgrade_scripts/upgrade_modules/modules_to_upgrade.csv
    if [ `git rev-parse --abbrev-ref HEAD` == "$new_uat_branch" ]; then
      git add *
      git commit -m "Module version updates for $new_uat_branch"
    else
      echo "something appears to have gone wrong"
      exit
    fi
  done
}

function push_profile_branch_changes {
  cd ~/Sites/stanford-jumpstart-deployer
  # unclear why it is saying these branches are behind remote counterparts
  # and must be pulled.  Why I try to pull it says, no remote counterparts.
  if [ `git rev-parse --abbrev-ref HEAD` == "$new_uat_branch" ]; then
    echo "git push --set-upstream origin $new_uat_branch"
    git push --set-upstream origin $new_uat_branch
  else
    git checkout $new_uat_branch
    echo "git push --set-upstream origin $new_uat_branch"
    git push --set-upstream origin $new_uat_branch
  fi
  cd ~/Sites/Stanford-Drupal-Profile
  if [ `git rev-parse --abbrev-ref HEAD` == "$new_uat_branch" ]; then
    echo "git push --set-upstream origin $new_uat_branch"
    git push --set-upstream origin $new_uat_branch
  else
    git checkout $new_uat_branch
    echo "git push --set-upstream origin $new_uat_branch"
    git push --set-upstream origin $new_uat_branch
  fi
}

function create_new_product_branch_in_gitolite_repository {
  cd ~/Sites/$last_stable_branch
  echo "git checkout $last_stable_branch$suffix"
  git checkout $last_stable_branch$suffix
  echo "git checkout -b $new_uat_product_branch"
  if [ `git branch --list "$new_uat_product_branch"` ]; then
    git checkout $new_uat_product_branch
    echo "existing branch checked out"
  else
    git checkout -b $new_uat_product_branch
  fi
  git push --set-upstream origin $new_uat_product_branch
}

function build_product_site {
  echo "setup_product_sites $new_uat_branch_name"
  # download site files as branch_name
  if [ "$product" == "group" ] || [ "$product" == "dept" ]; then
    drush make -y --force-complete ~/Sites/Stanford-Drupal-Profile/make/$product.make ~/Sites/$new_uat_product_branch
  elif [ "$product" == "jumpstart-engineering" ] || [ "$product" == "jumpstart-vpsa" ]; then
    drush make -y --force-complete ~/Sites/stanford-jumpstart-deployer/development/product/$product/$product.make ~/Sites/$new_uat_product_branch
  else
    drush make -y --force-complete ~/Sites/stanford-jumpstart-deployer/production/product/$product/$product.make ~/Sites/$new_uat_product_branch
  fi
  # grab profile from profiles directory in build site
  ## cd ~/Sites/$branch_name/profiles
  ## profile=$(ls | grep jumpstart)
  ## cd ~/Sites/$branch_name
  ## database_name=$(echo "$branch_name" | sed -e 's/-/_/g')
  ## drush @dev.$branch_name si -y "$profile" --db-url=mysql://root:midwayupon@127.0.0.1/$database_name
  # disable webauth in preparation for testing
  ## drush @dev.$branch_name dis -y webauth
  ## sed -ie "" "s|# RewriteBase /|RewriteBase /|" ~/Sites/$branch_name/.htaccess
}

function move_commit_modules {
  for module in ${modules_being_upgraded[@]}; do
    cd ~/Sites/$last_stable_branch
    git checkout $new_uat_product_branch
    git status
    rm -rf ~/Sites/$last_stable_branch/sites/all/modules/stanford/$module
    if [[ "$module" == "stanford"* ]] && [ -d ~/Sites/$new_uat_product_branch/sites/all/modules/stanford/$module ]; then
      mv ~/Sites/$new_uat_product_branch/sites/all/modules/stanford/$module ~/Sites/$last_stable_branch/sites/all/modules/stanford
      new_version=$(sed -n -e 's/^version = \(.*\)/\1/p' ~/Sites/$last_stable_branch/sites/all/modules/stanford/$module/$module.info)
      git add sites/all/modules/stanford/$module_name
    elif [ -d ~/Sites/$new_uat_product_branch/sites/all/modules/contrib/$module ]; then
      mv ~/Sites/$new_uat_product_branch/sites/all/modules/contrib/$module ~/Sites/$last_stable_branch/sites/all/modules/contrib
      new_version=$(sed -n -e 's/^version = \(.*\)/\1/p' ~/Sites/$last_stable_branch/sites/all/modules/contrib/$module/$module.info)
      git add sites/all/modules/contrib/$module_name
    else
      echo "Not able to find module in built site"
      exit
    fi
    git status
    echo "$module"
    git commit -m "Update module $module-$new_version"
  done
}

function push_product_branch_changes {
  cd ~/Sites/$last_stable_branch
  git push $new_uat_product_branch
}
