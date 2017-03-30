# [Upgrade Modules](https://github.com/SU-SWS/stanford_upgrade_scripts/tree/master/upgrade_modules)
Scripts to upgrade all profiles and product branches.
##### Version: 1.x

Maintainers: [kbrownell](https://github.com/kbrownell)

Every week we review Drupal's security update announcements to determine, whether we might need to patch our sites right away.  We strive to keep all modules in use on our site as updated and upgraded as possible.  Generally, that means every few months, we review all recent releases for contributed and custom built modules.  Determine what has changed.  Test these changes against our existing suite of Behat tests.  And then begin the long and hard process of updating our product profiles and site repositories.  The last step is particularly arduous.  For any one module upgrade, there are over 20 different places the module version needs to be changed.  This script was written to remove as much as possible human error and tedium from this last step in the process.  We should be able to take the spreadsheet where we collaborate and take notes on update candidates, export to a csv, and feed it in as a data source to upgrade_uat_branches.sh.

The script follows this pattern for making updates to the repositories in use on our sites.  First, it will update the module versions in our product profiles.  The data source can specify which current version should be upgraded to which new version, so products can have different start and end points.  For example, we recently updated stanford_capx from 7.x-3.0-beta2-php54 to 7.x-3.0-beta3-php54 for one product, from 7.x-2.1-php54 to 7.x-2.2-alpha2-php54 for one product, and 7.x-2.0-php54 to 7.x-2.2-alpha2-php54 for five of our products.  All in one go.  It will then build sites based on those updated profiles, applying any patches we have added in the past.  The script copies the module being updated to our live repositories.  Before pushing any changes, users are asked to check and approve these changes.  While this is a helpful stop gap, composer and Drupal 8, we love you and hope you save us from this situation.

Installation
---

1. Clone this repository to your local ~/Sites directory.
2. You'll need clones of Stanford-Drupal-Profile and stanford-jumpstart-deployer in the ~/Sites directory as well.
3. Check that you have an active Kerberos ticket.
4. Update modules_to_upgrade.csv to include the module and version pairs, you want upgraded.
5. Run ./upgrade_uat_branches.sh.
6. Make sure to check all changes before allowing the script to continue, as it takes quite a bit of work to clean up once changes have been pushed to remote repositories.
7. If you need to restart the script, you'll probably want to delete any branches it created and delete sites it built.

Areas for Improvement
---

1. It would be nice for this script to operate more like Ansible (perhaps it should be re-written in Ansible), which would allow users to re-start it at a particular task.
2. Right now, the script very much assumes a clear slate when it begins, for example, that no branches have been made with this date, and that sites have the module you're upgrading.  I've made some improvements to check a module directory exists before trying to move it, for example, but more work could be done in this area.

Contribution / Collaboration
---

You are welcome to contribute functionality, bug fixes, or documentation to this module. If you would like to suggest a fix or new functionality you may add a new issue to the GitHub issue queue or you may fork this repository and submit a pull request. For more help please see [GitHub's article on fork, branch, and pull requests](https://help.github.com/articles/using-pull-requests)
