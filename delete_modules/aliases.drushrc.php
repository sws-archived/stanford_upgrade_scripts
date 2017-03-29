<?php
## STANFORD WILDCARD #######################################################


// The command you just typed in shell.
$command = $_SERVER['argv'];

// Look at every argument...
foreach ($command as $arg){
  // There aren't many cases where there are '.'...
  // 0 => "@sse"
  // 1 => "ds_history"
  $test = explode('.',$arg);

  // $first = "@sse"
  // $test[0] = "ds_history"
  $first = array_shift($test);

  switch($first) {
    case "@sppl":
      // Set the project to be whatever the alias was
      $project_alias = str_replace('@', '', $arg);
      $project_name = array_pop($test);
      $shortname = str_replace("dp_", "", $project_name);
      $remote_host = "people1.stanford.edu";
      $root = '/var/www/'. $project_name . '/public_html';
      $ssh_options = '';
      $remote_user = 'kbrownel';
      break;
    case "@fsse":
      // Set the project to be whatever the alias was
      $project_alias = str_replace('@', '', $arg);
      $project_name = array_pop($test);
      $shortname = str_replace("ds_", "", $project_name);
      $remote_host = "sites1.stanford.edu";
      $root = '/var/www/'. $project_name . '/public_html';
      $ssh_options = '';
      $remote_user = 'kbrownel';
      break;
    case "@sdev":
      // Set the project to be whatever the alias was
      $project_alias = str_replace('@', '', $arg);
      $project_name = array_pop($test);
      $shortname = str_replace("ds_", "", $project_name);
      $remote_host = "sites-dev.stanford.edu";
      $root = '/var/www/'. $project_name . '/public_html';
      $ssh_options = '';
      $remote_user = 'kbrownel';
      break;
    case "@suat":
      // Set the project to be whatever the alias was
      $project_alias = str_replace('@', '', $arg);
      $project_name = array_pop($test);
      $shortname = str_replace("ds_", "", $project_name);
      $remote_host = "sites-uat.stanford.edu";
      $root = '/var/www/'. $project_name . '/public_html';
      $ssh_options = '';
      $remote_user = 'kbrownel';
      break;
  }
}

// project alias; this will be sse.ds_foo, uat.ds_foo, or ppl.dp_sunetid
$aliases[$project_alias] = array(
  'remote-host' => $remote_host,
  'remote-user' => $remote_user,
  'root' => $root,
  'uri' => 'default',
  'ssh-options' => $ssh_options,
  'path-aliases' => array(
    '%dump-dir' => '/tmp/',
  ),
);
