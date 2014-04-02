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
        l('copy to clipboard', 'javascript:void()', array('external' => TRUE, 'attributes' => array('class' => 'pf-hash-clipboard'))),
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

function privatefiles_entity_form($form, &$form_state, PrivateFile $entity = FALSE) {
  $form_state['privatefile'] = $entity;

  $form['security_level'] = array(
    '#title' => t('Security Level'),
    '#type' => 'radios',
    '#options' => array(
      PRIVATEFILE_USER_SECURED => t('User Level'),
      PRIVATEFILE_ROLE_SECURED => t('Role Level'),
      PRIVATEFILE_URL_SECURED => t('Public (only those with the link)')
    ),
    '#default_value' => !!$entity ? $entity->security_level : PRIVATEFILE_URL_SECURED,
    '#required' => TRUE
  );

  $form['users'] = array(
    '#title' => t('User Accounts'),
    '#type' => 'textfield',
    '#autocomplete_path' => 'user/autocomplete',
    '#states' => array(
      'visible' => array(
        ':input[name="security_level"]' => array('value' => PRIVATEFILE_USER_SECURED)
      )
    ),
    '#default_value' => !!$entity ? implode(', ', $entity->users) : NULL
  );


  $form['roles'] = array(
    '#title' => t('Roles'),
    '#type' => 'select',
    '#multiple' => TRUE,
    '#options' => user_roles(),
    '#states' => array(
      'visible' => array(
        ':input[name="security_level"]' => array('value' => PRIVATEFILE_ROLE_SECURED)
      )
    ),
    '#default_value' => !!$entity ? $entity->roles : array()
  );

  $form['expires'] = array(
    '#title' => t('Set an expiration date'),
    '#type' => 'checkbox',
    '#default_value' => (!!$entity && $entity->expiration_date->getTimestamp() > $this->created)
  );

  $default_date = !!$entity && $entity->expiration_date->getTimestamp() > $this->created
    ? $entity->expiration_date : new DateTime();

  $default_expiration = array(
    'month' => $default_date->format('n'),
    'day' => $default_date->format('j'),
    'year' => $default_date->format('Y')
  );

  $form['expiration_date'] = array(
    '#title' => t('Expiration Date'),
    '#type' => 'date',
    '#default_value' => $default_expiration,
    '#states' => array(
      'visible' => array(
        ':input[name="expires"]' => array('checked' => TRUE)
      )
    )
  );

  $form['file'] = array(
    '#title' => t('File'),
    '#type' => 'managed_file',
    '#default_value' => !!$entity ? $entity->fid : NULL,
    '#upload_location' => 'private://'
  );

  return $form;
}

function privatefiles_entity_form_submit($form, &$form_state) {
  $file = $form_state['privatefile'];
  $values = $form_state['values'];
  $values['fid'] = $values['file']['fid'];
  unset($values['file']);

  if ($values['expires']) {
    $exp_date = new DateTime();
    $exp_date->setDate(
      $values['expiration_date']['year'],
      $values['expiration_date']['month'],
      $values['expiration_date']['day']
    );
    $values['expiration_date'] = $exp_date;
  }
  else {
    $values['expiration_date'] = NULL;
  }
  unset($values['expires']);

  if (!!$file) {
    foreach ($values as $key => $value) {
      $file->$key = $value;
    }
  }
  else {
    $file = PrivateFileController::create($values);
  }
  $file->save();
}

function privatefiles_delete($form, &$form_state, PrivateFile $entity) {
  $form_state['privatefile'] = $entity;
  return confirm_form($form, t('Are you sure you want to delete this private file?'), 'admin/structure/privatefiles');
}

function privatefiles_delete_submit($form, $form_state) {
  $entity = $form_state['privatefile'];
  $entity->delete();
}