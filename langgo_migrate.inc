<?php

/**
 * TIP: While usually you'll create true migrations - processes that copy data
 * from some source into Drupal - you can also define processing steps for either
 * the import or rollback stages that take other actions. In this case, we want
 * to disable auto_nodetitle while the migration steps run. We'll re-enable it
 * over in LanggoMigrateFinishMigration.
 */
class LanggoMigratePrepMigration extends MigrationBase {
  // Track whether the auto_nodetitle was originally enabled so we know whether
  // to re-enable it. This is public so LanggoMigrateFinishMigration can reference it.
  public static $wasEnabled = FALSE;

  public function __construct() {
    // Because we're derived directly from migrationBase rather than AdvancedExampleMigration,
    // we must specify the group again here.
    parent::__construct(MigrateGroup::getInstance('wine', array('default')));
    $this->description = t('If auto_nodetitle is present, disable it for the duration');
  }
  // Define isComplete(), returning a boolean, to indicate whether dependent
  // migrations may proceed
  public function isComplete() {
    // If Auto Node Title is disabled, other migrations are free to go
    if (module_exists('auto_nodetitle')) {
      return FALSE;
    }
    else {
      return TRUE;
    }
  }
  // Implement any action you want to occur during an import process in an
  // import() method (alternatively, if you have an action which you want to
  // run during rollbacks, define a rollback() method).
  public function import() {
    if (module_exists('auto_nodetitle')) {
      self::$wasEnabled = TRUE;
      module_disable(array('auto_nodetitle'));
      self::displayMessage(t('Disabled auto_nodetitle module'), 'success');
    }
    else {
      self::$wasEnabled = FALSE;
      self::displayMessage(t('Auto_nodetitle is already disabled'), 'success');
    }
    // Must return one of the MigrationBase RESULT constants
    return MigrationBase::RESULT_COMPLETED;
  }
}



/**
 * TIP: Files can be migrated directly by themselves, by using the MigrateDestinationFile
 * class. This will copy the files themselves from the source, and set up the
 * Drupal file tables appropriately.
 */
class LanggoMigrateFileCopyMigration extends AdvancedExampleMigration {
  public function __construct() {
    parent::__construct();
    $this->description = t('Profile images');
    $this->dependencies = array('LanggoMigratePrep');
    $this->map = new MigrateSQLMap($this->machineName,
        array('imageid' => array(
                'type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Image ID.'
                )
             ),
        MigrateDestinationFile::getKeySchema()
    );
    $query = db_select('migrate_example_wine_files', 'wf')
             ->fields('wf', array('imageid', 'url'))
             ->isNull('wineid');
    $this->source = new MigrateSourceSQL($query);

    $this->destination = new MigrateDestinationFile();

    // In the simplest case, just map the incoming URL to 'value'.
    $this->addFieldMapping('value', 'url');

    $this->addUnmigratedDestinations(array('fid', 'uid', 'timestamp',
      'destination_dir', 'destination_file', 'source_dir', 'preserve_files',
      'file_replace'));
    $this->removeFieldMapping('pathauto');
  }
}


class LanggoMigrateRoleMigration extends Migration {
  public function __construct() {
    parent::__construct(MigrateGroup::getInstance('Langgo', array('default')));
    $this->description = t('XML feed (multi items) of roles (positions)');

    $this->softDependencies = array('LanggoMigrateFileCopy');

//    $fields = array(
//      'name' => t('Position name'),
//    );

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'rid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationRole::getKeySchema()
    );
    $query = db_select(OUR_SOURCE_DATABASE .'.role', 'r')
      ->fields('r', array('rid', 'name'))
      ->orderBy('r.rid', 'ASC');
    $this->source = new MigrateSourceSQL($query);//, $source_fields);
    
    $this->destination = new MigrateDestinationRole();

    $this->addFieldMapping('name', 'name');
         //->xpath('name');
    $this->addUnmigratedDestinations(array('weight'));
  }
}

class LanggoMigrateUserMigration extends Migration {
  public function __construct() {
    parent::__construct();
    $this->description = t('LanggoMigrate Drinkers of the world');
    $this->dependencies = array('LanggoMigrateRole');//, 'LanggoMigratePrep', 'LanggoMigrateFileCopy');
    $this->map = new MigrateSQLMap($this->machineName,
        array('uid' => array(
                'type' => 'int',
                'unsigned' => TRUE,
                'not null' => TRUE,
                'description' => 'Account ID.'
                )
             ),
        MigrateDestinationUser::getKeySchema()
    );
    $source_fields = array(
      'uid' => t('User ID'),
      'roles' => t('The set of roles assigned to a user.'),
    );
    $query = db_select(OUR_SOURCE_DATABASE . '.users', 'u')
             ->fields('u', array('uid', 'name', 'pass', 'mail', 'created', 'access', 'login', 'status', 'init'));
    $this->source = new MigrateSourceSQL($query,$source_fields);
    $this->destination = new MigrateDestinationUser();

    // Mapped fields
    $this->addSimpleMappings(array('name', 'status', 'mail'));
    $this->addFieldMapping('created', 'created')
         ->description('See prepare method');
    $this->addFieldMapping('access', 'access')
         ->description('See prepare method');
    $this->addFieldMapping('login', 'login')
         ->description('See prepare method');
    $this->addFieldMapping('pass', 'pass');
    $this->addFieldMapping('roles', 'roles')
         ->separator(',')
         ->sourceMigration('LanggoMigrateRole');
    //$this->addFieldMapping('signature', 'sig');
    //$this->addFieldMapping('signature_format')
      //   ->defaultValue($this->basicFormat->format);
    $this->addFieldMapping('init', 'init');
    $this->addFieldMapping('status', 'status');
    //$this->addFieldMapping('field_migrate_example_gender', 'sex')
      //   ->description(t('Map from M/F to 0/1 in prepare method'));
    //$this->addFieldMapping('picture', 'imageid')
      //   ->sourceMigration('LanggoMigrateFileCopy');

    // Unmapped source fields

    // Unmapped destination fields
    $this->addUnmigratedDestinations(array('theme', 'timezone', 'language',
      'is_new', 'field_migrate_example_favbeers', 'role_names'));
  }

  public function prepare(stdClass $account, stdClass $row) {
   
    $account->created = strtotime($account->created);
    $account->access = strtotime($account->access);
    $account->login = strtotime($account->login);
  }

}
