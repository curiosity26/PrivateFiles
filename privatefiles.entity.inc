<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 4/1/14
 * Time: 3:26 PM
 */

define('PRIVATEFILE_USER_SECURED', 'user');
define('PRIVATEFILE_ROLE_SECURED', 'role');
define('PRIVATEFILE_URL_SECURED', 'url');

class PrivateFile {
  public $id,
    $fid,
    $created,
    $security_level = PRIVATEFILE_URL_SECURED,
    $users = array(),
    $roles = array(),
    $hash;

  public function __construct(array $values = array()) {
    if (!empty($values)) {
      foreach ($values as $key => $value) {
        $this->$key = $value;
      }
      $this->hash();
    }
  }

  public function save() {
    return PrivateFileController::save($this);
  }

  public function delete() {
    return PrivateFileController::delete($this);
  }

  public function hash() {
    if (!!$this->id && !!$this->fid && !!$this->created) {
      $this->hash = privatefiles_encode($this);
      return $this->hash;
    }
    return FALSE;
  }
}

class PrivateFileController {
  public function create(array $values) {
    return new PrivateFile($values);
  }

  static public function load($ids, $reset = FALSE) {
    $files = &drupal_static(__FUNCTION__);
    $ret = array();
    $ids = (array)$ids;

    if ($reset) {
      $pfiles = db_select('privatefiles', 'pf')
        ->fields('pf', array('id', 'fid', 'created'))
        ->condition('id', $ids, 'IN')
        ->execute()
        ->fetchAllAssoc('id');
      if (count($pfiles)) {
        foreach ($pfiles as $id => $file) {
          $files[$id] = new PrivateFile($file);
        }
      }
      else {
        $files = array();
      }
      return $files;
    }
    else {
      foreach ($ids as $id) {
        if ($reset || empty($files) || empty($files[$id])) {
          $values = db_select('privatefiles', 'pf')
            ->fields('pf', array('id', 'fid', 'created'))
            ->condition('id', $id)
            ->execute()
            ->fetchAssoc();
          if (count($values)) {
            $files[$id] = new PrivateFile((array)$values);
          }
        }
        if (isset($files[$id])) {
          $ret[$id] = $files[$id];
        }
      }
    }

    return $ret;
  }

  static public function save(PrivateFile $file) {
    $mfile = file_load($file->fid);
    if ($mfile == 0) {
      throw new DrupalUpdateException(t('PrivateFile file id must a valid file.'));
    }

    if (empty($file->created)) {
      $file->created = microtime();
    }

    if ($file->id != 0) {
      drupal_write_record('private_file', $file, array('id'));
    }
    else {
      drupal_write_record('private_file', $file);
    }

    return $file;
  }

  static public function delete(PrivateFile $file) {
    return db_delete('privatefiles')->condition('id', $file->id)->execute();
  }
}