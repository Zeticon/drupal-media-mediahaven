<?php
/**
 * @file
 *   This file contains custom theming logic
 */

/**
 * Wraps an element in an extra div.
 */
function theme_my_element_wrapper($element) {
  $element = $variables['element'];
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
  $ouptut .= '</div>';
  return $output;
}

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

	$thumb_width = media_mediahaven_variable_get('thumb_width');
	$thumb_height = media_mediahaven_variable_get('thumb_height');
	
  // Arguments for the thumbnail link.
   $thumb = '<img src="' . str_replace('mediahaven://f/',media_mediahaven_variable_get('url') . '/index.php/embed/image?img=',$element['#file']->uri) . '">';
  //$thumb = $element['#children'];

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