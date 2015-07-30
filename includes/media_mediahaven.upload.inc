<?php

/**
 * @file
 * functions for uploading media files to mediahaven
 */

include_once 'MediaHavenRestClient.inc';
include_once 'media_mediahaven.admin.inc';
include_once 'media_mediahaven.utils.inc';

/**
 * Load the upload form.
 */
function media_mediahaven_upload($form, &$form_state = array(), $redirect = NULL) {
  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload a file to MediaHaven'),
    '#description' => t('Files uploaded through this form will be uploaded to MediaHaven and saved within your drupal installation'),
    '#collapsible' => TRUE,
  );
  module_load_install("plupload");
  $requirements = plupload_requirements("runtime");
  if($requirements['plupload']['severity']!=REQUIREMENT_OK){
    drupal_set_message(t('For easy uploading we recommend using the plupload library. Message: ').$requirements["plupload"]["description"]." ".l("More info","admin/reports/status"));
  }


  if (module_exists('plupload') && $requirements['plupload']['severity']==REQUIREMENT_OK) {
    $form['upload']['uploaded_file'] = array(
      '#type' => 'plupload',
      '#title' => t('Upload files'),
      '#description' => t('This multi-upload widget uses Plupload library.'),
      '#plupload_settings' => array(
        'runtimes' => 'html5, html4',
        'chunk_size' => '1mb',
      ),
    );
  }
  else {
    $form['upload']['uploaded_file'] = array(
      '#name' => 'uploaded_file',
      '#type' => 'file',
      '#title' => t('Choose a file'),
      '#title_display' => 'invisible',
      '#size' => 22,
      '#theme_wrappers' => array(),
      '#weight' => -10,
    );
  }
  $form['metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Metadata that will be applied to the uploaded files'),
    '#description' => t('The entered values will be applied to all files currently being uploaded.'),
    '#collapsible' => TRUE,
  );

  $form['metadata']['title'] = array(
    '#name' => 'title',
    '#type' => 'textfield',
    '#title' => t('Title of the media object(s)'),
    '#size' => 22,
  );

  $form['metadata']['description'] = array(
    '#name' => 'description',
    '#type' => 'textarea',
    '#title' => t('Description of the media object(s)'),
  );
  $form['metadata']['keywords'] = array(
    '#name' => 'keywords',
    '#type' => 'textarea',
    '#title' => t('Keywords'),
    '#description' => t('Enter multiple keywords separated by a comma (,)'),
  );

  $form['submit'] = array(
    '#name' => 'upload_button',
    '#type' => 'submit',
    '#value' => t('Process files'),
    '#submit' => array('media_mediahaven_upload_submit'),
  );
  return $form;
}

/**
 * Submit the upload form.
 */
function media_mediahaven_upload_submit($form, &$form_state = array(), $redirect = NULL) {
  foreach ($form_state['values']['uploaded_file'] as $uploaded_file) {
    if ($uploaded_file['status'] == 'done') {
      $source = $uploaded_file['tmppath'];
      $destination = file_directory_temp() . '/' . $uploaded_file['name'];

      $destination = file_unmanaged_move($source, $destination);

      $metadata = media_mediahaven_build_metadata($form_state['values']);

      $metadata_file = substr($destination, 0, strrpos($destination, '.')) . '.xml';
      media_mediahaven_generate_metadata($metadata_file, $metadata);

      $rest_client = media_mediahaven_rest_client();
      $result = $rest_client->upload($destination, $metadata_file, variable_get("ingestspace_id",'5ebefe86-279c-4e19-857c-23ec0e975278'));

      file_unmanaged_delete($destination);
      file_unmanaged_delete($metadata_file);

      if (!$result) {
        drupal_set_message(filter_xss('Processing files failed:' . $rest_client->error), 'error');
      }
      else {
        $result = json_decode($result, TRUE);
        $result['original_filename'] = $uploaded_file['name'];
        media_mediahaven_file_save($result);
      }
    }
    else {
      drupal_set_message(t('No uploaded files were provided'));
    }
  }
}

/**
 * Create the metadata array based on the form input.
 */
function media_mediahaven_build_metadata($form_values) {
  $metadata['title'] = $form_values['title'];
  $metadata['description'] = $form_values['description'];
  $metadata['keywords'] = explode(',', $form_values['keywords']);
  return $metadata;
}
