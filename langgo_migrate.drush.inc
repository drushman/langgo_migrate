<?php

/**
 * define drush command "lm-sql-sync"
 */
function langgo_migrate_drush_command() {
  $items = array();
  $items['langgo-sync-from-6'] = array(
    'description' => 'sync database via sql-sync.',
  );
  return $items;
}

/**
 * callback implementation for lm-sql-sync command 
 */
function drush_langgo_migrate_langgo_sync_from_6() {
  $error = 'Your need config';
  $not_error = TRUE;
  drush_print_r(variable_get('langgo_migrate_database_sync'));
  if (is_null(variable_get('langgo_migrate_database_sync'))) {
    $error .= 'String connect database ';
    $not_error = FALSE;
  }
  if (is_null(variable_get('langgo_migrate_db_url'))) {
    $error .= 'User remote, ';
    $not_error = FALSE;
  }
  if (is_null(variable_get('langgo_migrate_user'))) {
    $error .= 'User remote, ';
    $not_error = FALSE;
  }
  if (is_null(variable_get('langgo_migrate_host'))) {
    $error .= 'Host remote ';
    $not_error = FALSE;
  }
  if (!$not_error) {
    $error .= '. Please check your config at admin/config/langgo/langgo-migrate';
    drush_print_r($error);
    return;
  }
  $tables = variable_get('langgo_migrate_tables_ex', '');
  $exculde_tables = '';
  foreach ($tables as $table) {
    $exculde_tables .= '--ignore-table=langgo_duy.' . $table . ' ';
  }
  $ssh_su = variable_get('langgo_migrate_user');
  $ssh_host = variable_get('langgo_migrate_host');
  $database_sync = variable_get('langgo_migrate_database_sync');
  $db_url = drush_convert_db_from_db_url(variable_get('langgo_migrate_db_url', ''));
  $db_su = '';
  $db_su_pw = '';
  $db_host = '';
  $db_name = '';
  $check = !empty($db_url['username']);
  $check &= !empty($db_url['password']);
  $check &= !empty($db_url['host']);
  $check &= !empty($db_url['database']);
  if ($check) {
    $db_su = $db_url['username'];
    $db_su_pw = $db_url['password'];
    $db_host = $db_url['host'];
    $db_name = $db_url['database'];
  } else {
    $error = 'String connection incorrect.';
    drush_print_r($error);
    return;
  }
  $cmd = 'ssh '.$ssh_su .'@'. $ssh_host;
  $cmd .= ' \'mysqldump -u'.$db_su .' -p'. $db_su_pw .' -h'. $db_host .' '. $db_name  .' ';
  $cmd .= $exculde_tables . '> .tmp/langgo_migrate.sql\'';
  drush_op_system($cmd);
  $cmd = 'scp '.$ssh_su .'@'. $ssh_host .':.tmp/langgo_migrate.sql .';
  drush_op_system($cmd);
  $cmd = 'drush sql-cli --database='.$database_sync. ' < langgo_migrate.sql';
  drush_op_system($cmd);
}

