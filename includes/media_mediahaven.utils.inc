<?php
/**
 * @file
 *
 * utility functions for the media_mediahaven module
 */
/*
 * Save a mediahaven media file into drupal
*/
function media_mediahaven_file_save($media_data) {

  $uri = file_stream_wrapper_uri_normalize("mediahaven://f/" . $media_data['fragmentId']);
  //drupal_set_message('uri to save '.$uri);
  $file = file_uri_to_object($uri, TRUE);
  if ($media_data['type']=='Image') {
    $file->filemime = 'image/jpeg';
  }
  elseif ($media_data['type']=='Video') {
    $file->filemime = 'video/mp4';

  }
  else  {
    $file->filemime = file_get_mimetype($media_data['original_filename']);
  }

  $file->filesize = @filesize($uri);
  if ($media_data['title']=='provided title') {
    $file->filename = $media_data['original_filename'];
  }
  else {
    $file->filename = $media_data['title'];
  }

  if ($file === FALSE) {
    drupal_set_message(t('An error occurred and no file was saved.'), 'error');
  }
  elseif (!property_exists($file, 'fid')) {
    $file = file_save($file);

    // I do not understand the block below
    //field_attach_submit('media', $file, $form, $form_state);
    // Make a copy of the file object to use as the media object
    // (file is a field of media and the base table). This is weird.
    $file->file = array();
    $file->file[LANGUAGE_NONE] = array();
    $file->file[LANGUAGE_NONE][0] = (array) $file + array('display' => TRUE);
    $file->is_new = TRUE;
    field_attach_insert('media', $file);
    drupal_set_message(t('Successfully added media'));
  }
  else {
    // Already exists
    drupal_set_message(t('Already exists'), 'warning');
    return FALSE;
    //drupal_goto("media/{$file->fid}/edit");
  }
  return $file;
}

function media_mediahaven_generate_metadata($metadata_file, $metadata) {

  $xml = simplexml_load_file(dirname(__FILE__) . '/../assets/metadata.template.xml');
  $xml->MDProperties[0]->Department = media_mediahaven_variable_get('department_id');

  global $user;
  $application_user = $user->name;
  if (!empty($user->mail)) {
    $application_user = $user->mail;
  }
  $xml->MDProperties[0]->applicatie_gebruiker = $application_user;
  $xml->MDProperties[0]->rights_owner = media_mediahaven_variable_get('rights_owner');
  $xml->MDProperties[0]->rights = media_mediahaven_variable_get('rights');
  $xml->MDProperties[0]->copyright_notice = media_mediahaven_variable_get('copyright');

  if (!empty($metadata['title'])) {
    $xml->title = $metadata['title'];
  }

  if (!empty($metadata['description'])) {
    $xml->description = $metadata['description'];
  }

  if (!empty($metadata['keywords'])) {
    foreach ($metadata['keywords'] as $keyword) {
      $keyword = trim($keyword);
      if (!empty($keyword)) {
        $xml->keywords[0]->addChild('keyword', $keyword);
      }
    }
  }
  $xml->asXML($metadata_file);
}

function media_mediahaven_rest_client() {
  return new MediaHavenRestClient(media_mediahaven_variable_get('url') . MEDIA_MEDIAHAVEN_RESTPOINT,
      media_mediahaven_variable_get('login'),
      media_mediahaven_variable_get('password'));
}

function media_mediahaven_mediadata_to_file($media_data) {
  // Check database for this uri
  $fragment_id = $media_data['fragmentId'];
  $uri = file_stream_wrapper_uri_normalize("mediahaven://f/$fragment_id");
  $file = file_uri_to_object($uri, TRUE);

  if ($media_data['type']=='Image') {
    $file->filemime = 'image/mediahaven';
    $file->type = 'image';
  }
  elseif ($media_data['type']=='Video') {
    $file->filemime = 'video/mediahaven';
    $file->type = 'video';
  }
  else {
    $file->filemime = 'todo';
    $file->type = 'image';
  }

  //$file->filemime = file_get_mimetype($uri);
  $file->filesize = @filesize($uri);
  $file->filename = $media_data['title'];
  return $file;
}

function media_mediahaven_uri_parse_fragmentid($uri) {
  $target = file_uri_target($uri);
  $parts = explode('/', $target);
  $index = 0;
  foreach ($parts as $part) {
    if ($part == 'f') {
      return $parts[++$index];
    }
    $index++;
  }
}
