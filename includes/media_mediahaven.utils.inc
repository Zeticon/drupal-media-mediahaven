<?php

/**
 * @file
 * utility functions for the media_mediahaven module
 */

/**
 * Save a mediahaven media file into drupal.
 */
function media_mediahaven_file_save($media_data) {

  $uri = file_stream_wrapper_uri_normalize("mediahaven://f/" . $media_data['fragmentId']);
  // drupal_set_message('uri to save '.$uri);
  $file = file_uri_to_object($uri, TRUE);
  if ($media_data['type'] == 'Image') {
    $file->filemime = 'image/jpeg';
  }
  elseif ($media_data['type'] == 'Video') {
    $file->filemime = 'video/mp4';
  }
  else {
    $file->filemime = file_get_mimetype($media_data['original_filename']);
  }

  $file->filesize = @filesize($uri);
  if ($media_data['title'] == 'provided title') {
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
    // field_attach_submit('media', $file, $form, $form_state);
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
    // Already exists.
    drupal_set_message(t('Already exists'), 'warning');
    return FALSE;
    // drupal_goto("media/{$file->fid}/edit");
  }
  return $file;
}

/**
 * Generate the metadata XML file based on user input.
 */
function media_mediahaven_generate_metadata($metadata_file, $metadata) {

  $xml = simplexml_load_file(dirname(__FILE__) . '/../assets/metadata.template.xml');
  $xml->MDProperties[0]->Department = variable_get('media_mediahaven_department_id','cc6ea4c0-c7e9-44e3-906e-8b689d95c8f2');

  global $user;
  $application_user = $user->name;
  if (!empty($user->mail)) {
    $application_user = $user->mail;
  }
  $xml->MDProperties[0]->applicatie_gebruiker = $application_user;
  $xml->MDProperties[0]->rights_owner = variable_get('media_mediahaven_rights_owner','Zeticon');
  $xml->MDProperties[0]->rights = variable_get('media_mediahaven_rights','Full rights on reproduction');
  $xml->MDProperties[0]->copyright_notice = variable_get('media_mediahaven_copyright','Not for commercial use');

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


/**
 * Initialize the MediaHaven REST client.
 */
function media_mediahaven_rest_client() {
  return new MediaHavenRestClient(variable_get('media_mediahaven_url','https://integration.mediahaven.com') . MEDIA_MEDIAHAVEN_RESTPOINT,
    variable_get('media_mediahaven_login','apikey'),
    variable_get('media_mediahaven_password','apikey')
  );
}

/**
 * Create a file based on the media_data from MediaHaven.
 */
function media_mediahaven_mediadata_to_file($media_data) {
  // Check database for this uri.
  $fragment_id = $media_data['fragmentId'];
  $uri = file_stream_wrapper_uri_normalize("mediahaven://f/$fragment_id");
  $file = file_uri_to_object($uri, TRUE);

  if ($media_data['type'] == 'Image') {
    $file->filemime = 'image/mediahaven';
    $file->type = 'image';
  }
  elseif ($media_data['type'] == 'Video') {
    $file->filemime = 'video/mediahaven';
    $file->type = 'video';
  }
  else {
    $file->filemime = 'todo';
    $file->type = 'image';
  }

  $file->filesize = @filesize($uri);
  $file->filename = $media_data['title'];
  return $file;
}

/**
 * Parse the fragment_id out of the uri.
 */
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

/**
 * create the embed uri for a fragment.
 */
function media_mediahaven_embed_url($fragment_id) {
  $base_url = variable_get('media_mediahaven_url','https://integration.mediahaven.com');
  if (substr($base_url, -1) != '/') {
        $base_url .= '/';
      }    
  $base_url .= 'index.php/embed/' . $fragment_id;
  
  return $base_url;
}
