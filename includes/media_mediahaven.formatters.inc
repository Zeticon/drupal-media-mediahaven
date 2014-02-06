<?php

/**
 * @file media_youtube/includes/media_youtube.formatters.inc
 *
 * Formatters for Media: YouTube.
 */

/**
 * Implements hook_file_formatter_info().
 */
function media_mediahaven_file_formatter_info_alter(&$info)
{
	$info['file_image'] = array(
		'label' => t('Mediahaven Preview Image'),
		'file types' => array('image'),
		'default settings' => array(
		  'image_style' => '',
		),
		'view callback' => 'media_mediahaven_file_formatter_image_view',
		'settings callback' => 'media_mediahaven_file_formatter_image_settings',
		'mime types' => array('image/jpeg','image/mediahaven'),
	  );
	$info['file_field_image'] = array(
		'label' => t('Mediahaven Preview Image'),
		'file types' => array('image'),
		'default settings' => array(
		  'image_style' => '',
		),
		'view callback' => 'media_mediahaven_file_formatter_image_view',
		'settings callback' => 'media_mediahaven_file_formatter_image_settings',
		'mime types' => array('image/jpeg','image/mediahaven'),
	  );
	return $info;
}

function media_mediahaven_file_formatter_image_view($file, $display, $langcode) {
  $scheme = file_uri_scheme($file->uri);

  if ($scheme == 'mediahaven') {
    $wrapper = file_stream_wrapper_get_instance_by_uri($file->uri);
    $image_style = $display['settings']['image_style'];
    $valid_image_styles = image_style_options(FALSE);
    // @TODO: If autosubmit is removed and we allow view modes that insert
    // images in the WYSIWYG, add file->overrides handling.
    if (empty($image_style) || !isset($valid_image_styles[$image_style])) {
      $element = array(
        '#theme' => 'image',
        '#path' => $wrapper->getOriginalThumbnailPath(),
        '#alt' => isset($file->override['attributes']['alt']) ? $file->override['attributes']['alt'] : $file->filename,
      );
    }
    else {
      $element = array(
        '#theme' => 'image',
		/*'#theme' => 'image_style',
        '#style_name' => $image_style,*/
        '#path' => $wrapper->getOriginalThumbnailPath(),
        '#alt' => isset($file->override['attributes']['alt']) ? $file->override['attributes']['alt'] : $file->filename,
      );
    }

    return $element;
  }
}

/**
 * Implements hook_file_formatter_FORMATTER_settings().
 */
function media_mediahaven_file_formatter_image_settings($form, &$form_state, $settings) {
  $element = array();
  $element['image_style'] = array(
    '#title' => t('Image style'),
    '#type' => 'select',
    '#options' => image_style_options(FALSE),
    '#default_value' => $settings['image_style'],
    '#empty_option' => t('None (original image)'),
  );
  return $element;
}
