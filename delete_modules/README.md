# [Delete Modules](https://github.com/SU-SWS/stanford_upgrade_scripts/tree/master/delete_modules)
Scripts to delete modules saved to sites/default based on user selected criteria.
##### Version: 1.x

Maintainers: [kbrownell](https://github.com/kbrownell)

We maintain a number of sites with modules stored (and running) from sites/default.  In many cases, the very same modules can be found in sites/all.  This script should help us clean up these duplications and discrepencies, so that more sites align with our supported stack, ie. [Stanford-Drupal-Profile](https://github.com/SU-SWS/stanford-drupal-profile) and [stanford-jumpstart-deployer](https://github.com/SU-SWS/stanford-jumpstart-deployer).

Installation
---

1. This repository can be found in /afs/ir/group/webservices/tools/stanford_upgrade_scripts/delete_modules.
2. If you aren't already using drush aliases on our sites platform, save the example aliases.drushrc.php file to your AFS home directory.
3. Double check the variables last used in the variables.inc file are the variables you want to use now. If this is your first time running the script, copy example.variables.inc to variables.inc.
4. If you only want to delete modules from a small subset of sites on the server, create a file listing only those sites and enter the file path under the sws_sites_list_path variable.  An example is saved to this repository with the test site ds_sws-uat-jsv.
5. Run ./delete_modules.sh.
6. All logs can be found in the logs directory.
7. Even if the script says a site loads, you might want to double check that it is loading in your browser.

Contribution / Collaboration
---

You are welcome to contribute functionality, bug fixes, or documentation to this module. If you would like to suggest a fix or new functionality you may add a new issue to the GitHub issue queue or you may fork this repository and submit a pull request. For more help please see [GitHub's article on fork, branch, and pull requests](https://help.github.com/articles/using-pull-requests)
