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
    $expiration_date,
    $hash;

  public function __construct(array $values = array()) {
    if (!empty($values)) {
      foreach ($values as $key => $value) {
        if ($key === 'expiration_date') {
          $this->setExpirationDate($value);
          continue;
        }
        $this->$key = $value;
      }
      $this->hash();
    }
  }

  public function setExpirationDate($date) {
    if ($date instanceof DateTime) {
      $this->expiration_date = $date;
    }
    elseif (is_numeric($date)) {
      $this->expiration_date = new DateTime($date);
    }
    elseif (is_string($date)) {
      $this->expiration_date = DateTime::createFromFormat(DateTime::ISO8601, $date);
    }
    else {
      $this->expiration_date = new DateTime($this->created);
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
  static protected $entityCache = array();

  static public function create(array $values) {
    return new PrivateFile($values);
  }

  static public function load($ids, $reset = FALSE) {
    $ret = array();
    $ids = (array)$ids;

    if ($reset) {
      self::resetCache($ids);
      $pfiles = db_select('privatefiles', 'pf')
        ->fields('pf')
        ->condition('id', $ids, 'IN')
        ->execute()
        ->fetchAllAssoc('id');
      if (count($pfiles)) {
        foreach ($pfiles as $id => $file) {
          $ret[$id] = self::create($file);
        }
        self::cacheSet($ret);
      }
      else {
        $ret = array();
      }
      return $ret;
    }
    else {
      //$ret = self::cacheGet($ids);
      $ret = array();
      $no_cache_ids = array_diff_key(array_flip($ids), $ret);
      foreach (array_values($no_cache_ids) as $id) {
        $values = db_select('privatefiles', 'pf')
          ->fields('pf')
          ->condition('id', $id)
          ->execute()
          ->fetchAssoc();
        dpm($values, 'Values');
        if (count($values)) {
          $ret[$id] = self::create((array)$values);
        }
      }
    }

    return count($ret) === 1 ? array_shift($ret) : $ret;
  }

  static public function cacheGet($ids) {
    if (!empty(self::$entityCache)) {
      if ($ids) {
        return array_intersect_key(self::$entityCache, array_flip($ids));
      }
      else {
        return self::$entityCache;
      }
    }
    return array();
  }

  static public function cacheSet(array $entities) {
    self::resetCache(array_keys($entities));
    self::$entityCache += $entities;
  }

  static public function resetCache(array $ids = NULL) {
    if (isset($ids)) {
      foreach ($ids as $id) {
        unset(self::$entityCache[$id]);
      }
    }
    else {
      self::$entityCache = array();
    }
  }

  static public function save(PrivateFile $file) {
    $mfile = file_load($file->fid);
    if (!$mfile || !file_valid_uri($mfile->uri)) {
      throw new DrupalUpdateException(t('PrivateFile file id must a valid file.'));
    }

    if (!$file->created) {
      $file->created = REQUEST_TIME;
    }

    $exp_date = $file->expiration_date;
    $file->expiration_date = $exp_date->format(DateTime::ISO8601); // Prepare for DB Save

    if ($file->id != 0) {
      $success = drupal_write_record('privatefiles', $file, array('id'));
    }
    else {
      $success = drupal_write_record('privatefiles', $file);
    }

    $file->expiration_date = $exp_date;
    if ($success !== FALSE) {
      self::cacheSet(array($file->id => $file));
      file_usage_add($mfile, 'privatefiles', 'privatefile', $file->id);
      return $file;
    }

    return FALSE;
  }

  static public function delete(PrivateFile $file) {
    $mfile = file_load($file->fid);
    file_usage_delete($mfile, 'privatefiles', 'privatefile', $file->id);
    return db_delete('privatefiles')->condition('id', $file->id)->execute();
  }
}