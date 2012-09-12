<?php

/**
 * Create form to setting on langgo dc server
 */

function langgo_migrate_setting_form(){
  $form = array();
  global $databases;
  $options = array();
  
  foreach($databases as $database => $value ){
    if($database == 'default')      continue;
    $options[$database] = $value['default']['database'];
  }
  $form['host_remote'] = array(
    '#type' => 'textfield', 
    '#title' => t('Host remote'), 
    '#description' => t('Host name of live site'),
    '#default_value' => variable_get('langgo_migrate_host'),     
    '#required' => TRUE,
  );
  $form['user_remote'] = array(
    '#type' => 'textfield', 
    '#title' => t('User remote'), 
    '#description' => t('User name to access to live server via SSH'),
    '#default_value' => variable_get('langgo_migrate_user'),     
    '#required' => TRUE,
  );
  $form['db_url_remote'] = array(
    '#type' => 'textfield', 
    '#title' => t('String connect database'), 
    '#description' => t('String connect database.Ex mysql://drupal:drupal@localhost/drupal'),
    '#default_value' => variable_get('langgo_migrate_db_url'),     
    '#required' => TRUE,
  );
  
  $form['database_list'] = array(
    '#type' => 'select',
    '#title' => t('Choose data to recive sync'),
    '#default_value' => variable_get('langgo_migrate_database_sync'),
    '#options' => $options,
    '#attributes' => array('class' => array('menu-parent-select')),
  );
  $form['exclude_tables'] = array(
    '#type' => 'textarea',
    '#title' => t('Tables exclude'),
    '#description' => t('One table per line'),
    '#default_value' => langgo_migrate_load_exclude_tables_default(),
    '#rows' => 5,
  );
  
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Save'),
  );
  
  //dsm($databases);
  return $form;
}

/**
 * save value of form to variables when submit
 * @param type $form_state
 */
function langgo_migrate_setting_form_submit(&$form_state) {
  $tables = array();
  if ($form_state['exclude_tables']['#value']) {
    foreach (explode ("\n", str_replace("\r", '', $form_state['exclude_tables']['#value'])) as $table) {
      if ($table) {
        $tables[] = $table;
      }
    }
  }
  variable_set('langgo_migrate_tables_ex', $tables);    
  variable_set('langgo_migrate_db_url', $form_state['db_url_remote']['#value']);
  variable_set('langgo_migrate_user', $form_state['user_remote']['#value']);
  variable_set('langgo_migrate_host', $form_state['host_remote']['#value']);
  variable_set('langgo_migrate_database_sync', $form_state['database_list']['#value']);  
}


/**
 * get default value of exclude_tables from variable
 */
function langgo_migrate_load_exclude_tables_default(){
  if(variable_get('langgo_migrate_tables_ex', '') == ''){
    return '';
  }
  $tables = variable_get('langgo_migrate_tables_ex', '');
  $exculde_tables = '';
  foreach ($tables as $table) {
    $exculde_tables .= $table . PHP_EOL ;
  }
  return $exculde_tables;
}