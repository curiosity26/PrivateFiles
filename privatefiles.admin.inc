<?php
/**
 * Created by PhpStorm.
 * User: alexboyce
 * Date: 4/1/14
 * Time: 9:15 PM
 */

function privatefiles_list() {
  $query = db_select('privatefiles', 'pf')
    ->fields('pf', array('id'));

  $query->extend('PagerDefault')->limit(25)->orderBy('created', 'DESC');
  $ids = $query->execute()->fetchAllAssoc('id');
  $files = PrivateFileController::load(array_values($ids));
  $rows = array();

  foreach ($files as $id => $file) {
    $mf = file_load($file->fid);
    if (!!$mf) {
      $rows[] = array(
        $id,
        $mf->filename,
        l($file->hash, 'private/files/'.$file->hash),
        l('copy to clipboard', 'javascript:void', array('external' => TRUE, 'attributes' => array('class' => 'pf-hash-clipboard'))),
        l('edit', 'admin/structure/privatefiles/'.$id.'/edit'),
        l('delete', 'admin/structure/privatefiles/'.$id.'/delete')
      );
    }
  }

  $output = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => array(t('ID'), t('Filename'), t('Link'), array('data' => t('Operations'), 'colspan' => 3)),
    '#suffix' => theme_pager(array('element' => 0)),
    '#attached' => array(
      'js' => array(drupal_get_path('module', 'privatefiles').'/js/privatefiles.js')
    )
  );

  return $output;
}