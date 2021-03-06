<?php

/**
 * @file
 * This file contains custom theming logic
 */

/**
 * Wraps an element in an extra div.
 */
function theme_my_element_wrapper($element) {
  $output = '<div class="element-wrapper">';
  $output .= $element['#children'];
  $output .= '</div>';
  return $output;
}

/**
 * Some generic theming function.
 */
function theme_my_custom_html($variables) {
  $foo = $variables['foo'];
  $output = '<div class="type-foo clearfix">';
  $output .= $foo;
  $output .= '</div>';
  return $output;
}

/**
 * Show thumbnail for a MediaHaven file.
 */
function theme_media_mediahaven_thumbnail($variables) {
  $label = '';
  $element = $variables['element'];

  // Wrappers to go around the thumbnail.
  $attributes = array(
    'title' => $element['#file']->filename,
    'class' => 'media-item',
    'data-fid' => $element['#file']->fid,
  );
  $prefix = '<div ' . drupal_attributes($attributes) . '><div class="media-thumbnail">';
  $suffix = '</div></div>';

  // Arguments for the thumbnail link.
  $thumb = '<img src="' . str_replace('mediahaven://f/', variable_get('media_mediahaven_url','https://integration.mediahaven.com') . '/index.php/embed/image?img=',
    $element['#file']->uri
  ) . '&w=100&h=100">';

  $target = 'file/' . $element['#file']->fid . '/edit';
  $options = array(
    'query' => drupal_get_destination(),
    'html' => TRUE,
    'attributes' => array('title' => t('Click to edit details')),
  );

  // Element should be a field renderable array. This should be improved.
  if (!empty($element['#show_names']) && $element['#file']->filename) {
    $label = '<div class="label-wrapper"><label class="media-filename">' . $element['#file']->filename . '</label></div>';
  }

  $output = $prefix;
  if (!empty($element['#add_link'])) {
    $output .= l($thumb, $target, $options);
  }
  else {
    $output .= $thumb;
  }
  $output .= $label . $suffix;
  return $output;
}

/**
 * Preprocess function for theme('media_mediahaven_video').
 */
function media_mediahaven_preprocess_media_mediahaven_video(&$variables) {
  $umid = substr(strrchr($variables['uri'], "/"), 1);
  $variables['umid'] = $umid;

  $variables['mediahaven_url'] =variable_get('media_mediahaven_url','https://integration.mediahaven.com');
  $variables['video_width'] = 640;
  $variables['video_height'] = 360;
  return $variables;
}
