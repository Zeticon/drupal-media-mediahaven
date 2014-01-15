<?php
/*
 * Save a mediahaven media file into drupal
 */
function media_mediahaven_file_save($media_data) {
    $uri = file_stream_wrapper_uri_normalize("mediahaven://f/".$media_data['fragmentId']);
    drupal_set_message('uri to save '.$uri);
	$file = file_uri_to_object($uri, TRUE);
	if($media_data['type']=='Image')
	{
		$file->filemime = 'image/jpeg';
	} elseif($media_data['type']=='Video')
	{
		$file->filemime = 'video/mp4';
	}

	//$file->filemime = file_get_mimetype($uri);
	$file->filesize = @filesize($uri);
	$file->filename = $media_data['title'];

	if ($file === FALSE) {
		drupal_set_message(t('An error occurred and no file was saved.'), 'error');
		$redirect = "media_mediahaven_add";
	} elseif (!property_exists($file, 'fid')) {
		$file = file_save($file);
		$redirect = "media/{$file->fid}/edit";

		// I do not understand the block below
		//field_attach_submit('media', $file, $form, $form_state);
		// Make a copy of the file object to use as the media object
		// (file is a field of media and the base table). This is weird.
		$media = clone($file);
		$file->file = array();
		$file->file[LANGUAGE_NONE] = array();
		$file->file[LANGUAGE_NONE][0] = (array) $file + array('display' => TRUE);
		$file->is_new = TRUE;
		field_attach_insert('media', $file);
		drupal_set_message(t('Successfully added media'));
	} else {
		// Already exists
		drupal_set_message(t('Already exists'), 'warning');
		return false;
		//drupal_goto("media/{$file->fid}/edit");
	}
    return $file;

}

function media_mediahaven_generate_metadata($metadata_file, $metadata) {

  $xml = simplexml_load_file(dirname(__FILE__).'/../assets/metadata.template.xml');
  $xml->MDProperties[0]->Department = media_mediahaven_variable_get('department_id');

  global $user;
  $application_user = $user->name;
  if (!empty($user->mail)) {
    $application_user = $user->mail;
  }
  $xml->MDProperties[0]->applicatie_gebruiker = $application_user;

  if(!empty($metadata['title'])) {
    $xml->title = $metadata['title'];
  }
 if(!empty($metadata['description'])) {
    $xml->description = $metadata['description'];
  }

  return $xml->asXML($metadata_file);
}
