#!/bin/bash

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
  for profile in ${!stanford_profiles[@]}; do
    profile_master_branch="${stanford_profiles[$profile]}"
    cd ~/Sites/$profile
    git checkout $profile_master_branch
    git pull
    git checkout -b $new_uat_branch
    while IFS=, read module_name current_version new_version; do
      find_replace_module_versions
    done < ~/Documents/modules_to_upgrade.csv
    git add *
    git commit -m "Module version updates for $new_uat_branch"
    # git push --set-upstream origin $new_uat_branch
  done
}

function create_new_product_branch_in_gitolite_repository {
  cd ~/Sites/$last_stable_branch
  echo "git checkout $last_stable_branch$suffix"
  git checkout $last_stable_branch$suffix
  echo "git checkout -b $new_uat_product_branch"
  git checkout -b $new_uat_product_branch
  # git push --set-upstream origin $new_uat_product_branch
}

function build_product_site {
  echo "setup_product_sites $branch_name"
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
    if [[ "$module" == "stanford"* ]]; then
      mv ~/Sites/$new_uat_product_branch/sites/all/modules/stanford/$module ~/Sites/$last_stable_branch/sites/all/modules/stanford
    else
      mv ~/Sites/$new_uat_product_branch/sites/all/modules/contrib/$module ~/Sites/$last_stable_branch/sites/all/modules/contrib
    fi
    git status
    git add sites/all/modules/contrib/$module_name
    git status
    git commit -m "Update module $module_name-7.x-$new_version"
    # git push
  done
}
