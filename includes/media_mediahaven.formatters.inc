<?php

/**
 * @file media_mediahaven/includes/media_mediahaven.formatters.inc
 *
 * Formatters for Media: Mediahaven.
 */

/**
 * Implements hook_file_formatter_info().
 */
function media_mediahaven_file_formatter_info_alter(&$info) {
  $info['file_image'] = array(
      'label' => t('Mediahaven Preview Image'),
      'file types' => array('image'),
      'default settings' => array(
          'image_style' => '',
      ),
      'view callback' => 'media_mediahaven_file_formatter_image_view',
      'settings callback' => 'media_mediahaven_file_formatter_image_settings',
      'mime types' => array('image/jpeg', 'image/mediahaven'),
  );

  $info['file_field_image'] = array(
      'label' => t('Mediahaven Preview Image'),
      'file types' => array('image'),
      'default settings' => array(
          'image_style' => '',
      ),
      'view callback' => 'media_mediahaven_file_formatter_image_view',
      'settings callback' => 'media_mediahaven_file_formatter_image_settings',
      'mime types' => array('image/jpeg', 'image/mediahaven'),
  );

  //[file_field_media_large_icon]
  //[file_field_styles_file_square_thumbnail]
  //[file_field_styles_file_thumbnail]
  //[file_field_styles_file_large]
  //[file_field_styles_file_medium]
  //[file_field_styles_file_original]
  //[file_image]

  $info['file_field_file_video'] = array(
      'label' => t('Mediahaven Preview Image'),
      'file types' => array('video'),
      'default settings' => array(
          'image_style' => 'thumbnail',
      ),
      'view callback' => 'media_mediahaven_file_formatter_video_view',
      'settings callback' => 'media_mediahaven_file_formatter_video_settings',
      'mime types' => array('video/mp4', 'video/mediahaven'),
  );


  $info['file_field_media_large_icon'] = array(
      'label' => t('Mediahaven Preview Image'),
      'file types' => array('video'),
      'default settings' => array(
          'image_style' => '',
      ),
      'view callback' => 'media_mediahaven_file_formatter_image_view',
      'settings callback' => 'media_mediahaven_file_formatter_image_settings',
      'mime types' => array('video/mp4', 'video/mediahaven'),
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
          '#suffix' => '<span class="media_object_type_' . ucfirst($file->type) . '">' . ucfirst($file->type) . '</span>',
      );
    }
    else {
      $element = array(
          '#theme' => 'image',
          '#path' => $wrapper->getOriginalThumbnailPath(),
          '#alt' => isset($file->override['attributes']['alt']) ? $file->override['attributes']['alt'] : $file->filename,
          '#suffix' => '<span class="media_object_type_' . ucfirst($file->type) . '">' . ucfirst($file->type) . '</span>',
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

function media_mediahaven_file_formatter_video_view($file, $display, $langcode) {
  $scheme = file_uri_scheme($file->uri);

  if ($scheme == 'mediahaven') {
    $wrapper = file_stream_wrapper_get_instance_by_uri($file->uri);
    $image_style = $display['settings']['image_style'];

    $element = array(
        '#theme' => 'media_mediahaven_video',
        '#uri' => $file->uri,
        '#options' => array(),
    );

    return $element;
  }
}

function media_mediahaven_file_formatter_video_settings($form, &$form_state, $settings) {
  $element = array();
  $element['video_style'] = array(
      '#title' => t('Video style'),
      '#type' => 'select',
      '#options' => image_style_options(FALSE),
      '#default_value' => $settings['video_style'],
      '#empty_option' => t('None (original video)'),
  );
  return $element;
}

function media_mediahaven_field_formatter_prepare_view($entity_type, $entities, $field, $instances, $langcode, &$items) {

}

function media_mediahaven_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {

}