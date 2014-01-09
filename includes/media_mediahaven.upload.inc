<?php
include_once ('MediaHavenRestClient.inc');
include_once ('media_mediahaven.admin.inc');

function media_mediahaven_upload($form, &$form_state = array(), $redirect = NULL) {
  $form['upload'] = array(
      '#type' => 'fieldset',
      '#title' => t('Upload a file to MediaHaven'),
      '#description' => t('Files uploaded through this form will be uploaded to MediaHaven and saved within your drupal installation'),
      '#collapsible' => TRUE,
  );



  if (module_exists('plupload')) {
    $form['upload']['uploaded_file'] = array(
        '#type' => 'plupload',
        '#title' => t('Upload files'),
        '#description' => t('This multi-upload widget uses Plupload library.'),
        //'#upload_validators' => array(
        //   'file_validate_extensions' => array('jpg jpeg gif png txt doc xls pdf ppt pps odt ods odp'),
        //  'my_custom_file_validator' => array('some validation criteria'),
        //
        '#plupload_settings' => array(
            'runtimes' => 'html5, html4',
            'chunk_size' => '1mb',
        ),
    );
  } else {
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
  $form['upload']['submit'] = array(
      '#name' => 'upload_button',
      '#type' => 'submit',
      '#value' => t('Process files'),
      '#submit' => array('media_mediahaven_upload_submit')
  );
  return $form;
}

function media_mediahaven_upload_submit($form, &$form_state = array(), $redirect = NULL) {
  foreach($form_state['values']['uploaded_file'] as $uploaded_file) {
    if ($uploaded_file['status'] == 'done') {
      $source = $uploaded_file['tmppath'];
      $destination =  file_directory_temp() . DIRECTORY_SEPARATOR . $uploaded_file['name'];
      drupal_set_message('current path '. $source);
      drupal_set_message('new path '. $destination);

      $destination = file_unmanaged_move($source, $destination);

      //upload the file to mediahaven
      drupal_set_message('before upload');
      $rest_client = _get_mediahaven_rest_client();
      $result = $rest_client->upload($destination);
      drupal_set_message('after upload');
      //manage file within drupal

      //clean up
      file_unmanaged_delete($destination);
    } else {
      drupal_set_message(t('No uploaded files were provided'));
    }
  }

}
