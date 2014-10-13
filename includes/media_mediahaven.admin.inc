<?php

/**
 * @file
 * Functions for administration of the module
 *
 * Callback for /admin/configure/media/media_mediahaven.
 */

include_once 'MediaHavenRestClient.inc';
include_once 'media_mediahaven.variables.inc';

/**
 * Create a form to add media from MediaHaven.
 */
function media_mediahaven_add($form, &$form_state = array(), $redirect = NULL) {
  module_load_include('inc', 'media', 'includes/media.browser');

  $uuid = media_mediahaven_parameter($_GET, 'uuid');

  if ($uuid !== NULL) {
    media_mediahaven_submit_mob($form, $form_state);
    return;
  }

  $data['mediahaven_search'] = media_mediahaven_generate_search($form_state);

  return array('mediahaven' => $data);
}

/**
 * Generate search page.
 */
function media_mediahaven_generate_search($state) {
  $data = array(
    '#type' => 'fieldset',
  );

  $search = media_mediahaven_search_string($state);
  $selected_facets = media_mediahaven_selected_facets($state);
  $facets_output = media_mediahaven_parse_selected_facets($selected_facets);

  $facets = media_mediahaven_search_facets($selected_facets, $search);

  $results = media_mediahaven_search_results($search, $selected_facets);

  $data['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Search'),
    '#default_value' => $search !== NULL ? $search : '',
  );

  $data['selected_facets'] = $facets_output;

  $data['facets'] = media_mediahaven_generate_facets($facets);

  $data['button'] = array(
    '#type' => 'button',
    '#value' => t('Search'),
    '#submit' => array('media_mediahaven_add_test'),
  );

  if (isset($results['totalNrOfResults'])) {
    $data['total_results'] = array(
      '#type' => 'html_tag',
      '#tag' => 'h3',
      '#value' => t('Total Results:') . $results['totalNrOfResults'],
    );
  }

  $data['results'] = media_mediahaven_generate_results($results);

  $data['pager'] = media_mediahaven_generate_pager($results['totalNrOfResults'], 0, array('search' => $search));

  return $data;
}

/**
 * Retrieve parameter from container.
 */
function media_mediahaven_parameter($container, $key, $default = NULL) {
  if (isset($container[$key])) {
    return $container[$key];
  }
  return $default;
}

/**
 * Return entered search string.
 */
function media_mediahaven_search_string($state) {
  $search = NULL;

  if (isset($state['values']['search'])) {
    $search = $state['values']['search'];
  }
  elseif (isset($_GET['search'])) {
    $search = check_plain(filter_xss($_GET['search']));
  }
  return $search;
}

/**
 * Return selected facets.
 */
function media_mediahaven_selected_facets($state) {
  $selected_facets = array();

  if (isset($state['values']['search'])) {
    return $selected_facets;
  }

  if (isset($_GET['fa'])) {
    $selected_facets = (array) $_GET['fa'];
  }

  return $selected_facets;
}

/**
 * Search with selected facets and a search term.
 */
function media_mediahaven_search_facets($selected_facets, $search_term) {
  $full_query = "";
  foreach ($selected_facets as $selected_facet) {
    $full_query .= $selected_facet;
  }

  if (isset($search_term) && strlen($search_term) > 0) {
    if (strlen($full_query) > 0) {
      $full_query .= ' ';
    }
    $full_query .= '+(' . $search_term . ')';
  }

  $rest_client = media_mediahaven_rest_client();
  $result = $rest_client->getFacets($full_query);

  $arr_facets_json = json_decode($result, TRUE);

  if ($arr_facets_json && isset($arr_facets_json['facet'])) {
    return $arr_facets_json['facet'];
  }
  else {
    drupal_set_message($rest_client->error, 'error');
    return FALSE;
  }
}

/**
 * Parse the selected facets.
 */
function media_mediahaven_parse_selected_facets($selected_facets) {
  if (!$selected_facets) {
    return FALSE;
  }
  if (!$selected_facets[0]) {
    return FALSE;
  }

  $pattern = '/\+dtag\:\"root>([A-Za-z0-9 ]*)>([A-Za-z0-9 ]*)\"/';
  preg_match_all($pattern, $selected_facets[0], $matches);

  if (!empty($matches)) {
    $selected_facet_list = array(
      '#theme' => 'item_list',
      '#type' => 'ul',
      '#title' => t('Selected Facets'),
      '#attributes' => array('class' => 'selected_facet_list'),
    );

    foreach ($matches[0] as $key => $match) {
      $selected_facet_list['#items'][] = $matches[1][$key] . ' > ' . $matches[2][$key];
    }

    return $selected_facet_list;
  }

  return FALSE;
}

/**
 * Generate facets.
 */
function media_mediahaven_generate_facets($facets) {
  if (empty($facets)) {
    return array();
  }

  $facets_list = array(
    '#theme' => 'item_list',
    '#type' => 'ul',
    '#title' => t('Selectable Facets'),
    '#attributes' => array('class' => 'facet_list'),
  );

  foreach ($facets as $facet_category) {
    $children = media_mediahaven_generate_facets_link($facet_category['value']);

    $facet_category = array(
      'data' => $facet_category['title'],
      'id' => $facet_category['name'],
      'class' => array(
        'facet_title'),
      'children' => $children,
    );
    $facets_list['#items'][] = $facet_category;
  }

  return $facets_list;
}

/**
 * Generate links for facets.
 */
function media_mediahaven_generate_facets_link($facets) {
  $max_facets = 5;
  $facet_links = array();

  foreach ($facets as $facet_item) {
    $facet_links[] = '<a href="' . '?fa[]=' . media_mediahaven_parse_facet_link($facet_item['query']) . '">' . htmlentities($facet_item['label']) . '</a> (' . $facet_item['count'] . ')';
  }
  if (count($facet_links) > $max_facets) {
    $facet_links = array_slice($facet_links, 0, $max_facets);
  }
  return $facet_links;
}

/**
 * Parse the facet link.
 */
function media_mediahaven_parse_facet_link($query) {
  $pattern = '/\+\([A-Za-z0-9 ]*\)/';
  preg_match($pattern, $query, $matches);
  $query = urlencode($query);
  if (!empty($matches)) {
    $query = $query . '&search=' . substr(trim($matches[0]), 2, -1);
  }

  return $query;
}

/**
 * Perform the search.
 *
 * @param unknown $search
 *   the query entered.
 * @param unknown $selected_facets
 *   the selected facets.
 *
 * @return mixed
 *   the search results.
 */
function media_mediahaven_search_results($search, $selected_facets) {
  $start_index = 0;
  $nr_of_results = 25;
  $query = '';

  if (isset($_GET['page'])) {
    $current_page = (int) filter_xss($_GET['page']);
    $start_index = $current_page * $nr_of_results;
  }

  $query = $search;

  if (isset($selected_facets)) {
    foreach ($selected_facets as $selected_facet) {
      $query .= $selected_facet;
    }
  }

  $rest_client = media_mediahaven_rest_client();
  $result = $rest_client->search($query, $start_index, $nr_of_results);

  if ($result) {
    $arr_json = json_decode($result, TRUE);
  }
  else {
    drupal_set_message(t('Could not retrieve search results:') . filter_xss($rest_client->error), 'error');
  }

  return $arr_json;
}

/**
 * Organize results from search engine to visualize them.
 */
function media_mediahaven_generate_results($results) {
  if (isset($results['totalNrOfResults'])) {
    unset($results['totalNrOfResults']);
  }

  if (isset($results['startIndex'])) {
    unset($results['startIndex']);
  }

  $results = $results['mediaDataList'];

  $output = array(
    '#theme' => 'item_list',
    '#type' => 'ul',
    '#attributes' => array('class' => 'result_list'),
  );

  if (!empty($results)) {
    foreach ($results as $media_item) {
      $output['#items'][] = media_mediahaven_generate_result_current($media_item);
    }
  }
  else {
    return FALSE;
  }

  return $output;
}

/**
 * Show search result.
 */
function media_mediahaven_generate_result_current($media_item) {
  if ($media_item['previewImagePath'] == '') {
    return FALSE;
  }

  $variables = array();
  $variables['path'] = $media_item['previewImagePath'];
  $variables['width'] = media_mediahaven_variable_get('thumb_width');
  $variables['height'] = media_mediahaven_variable_get('thumb_height');

  return l('<img width="' . $variables['width'] .
      '" height="' . $variables['height'] . '" src="' .
      $media_item['thumbnailImagePath'] . '"/>
      <span class="media_object_type_' . $media_item['type'] . '">'
      . t($media_item['type']) . '</span>', 'admin/content/media/add/media_mediahaven/',
      array(
        'attributes' => array('class' => 'anchor-class', 'rel' => $media_item['fragmentId']),
        'html' => TRUE,
        'query' => array('uuid' => $media_item['fragmentId']),
      )
  );
}

/**
 * Generate paging functionality.
 */
function media_mediahaven_generate_pager($num_results, $element = 0, $parameters = array()) {
  if (isset($_POST['search'])) {
    $_GET['search'] = filter_xss(check_plain($_POST['search']));
  }

  pager_default_initialize($num_results, MEDIA_MEDIAHAVEN_PAGER, $element);
  return array(
    '#markup' => theme(
        'pager',
        array(
          'parameters' => $parameters,
          'element' => $element,
        )
    ),
  );
}


/**
 * Submit the selected media object.
 */
function media_mediahaven_submit_mob(&$form, &$form_state) {
  $uuid = media_mediahaven_parameter($_GET, 'uuid');

  $redirect = 'media_mediahaven_add';

  $rest_client = media_mediahaven_rest_client();
  $result = $rest_client->getMediaObject($uuid);

  if ($result) {
    $arr_json = json_decode($result, TRUE);
  }
  else {
    drupal_set_message($rest_client->error, 'error');
    return FALSE;
  }

  $uri = file_stream_wrapper_uri_normalize("mediahaven://f/$uuid");
  $file = file_uri_to_object($uri, TRUE);
  if ($arr_json['type'] == 'Image') {
    $file->filemime = 'image/jpeg';
  }
  elseif ($arr_json['type'] == 'Video') {
    $file->filemime = 'video/mp4';
  }

  $file->filesize = @filesize($uri);
  $file->filename = $arr_json['title'];

  if ($file === FALSE) {
    drupal_set_message(t('An error occurred and no file was saved.'), 'error');
    $redirect = "media_mediahaven_add";
  }
  elseif (!property_exists($file, 'fid')) {
    $file = file_save($file);

    if (module_exists('overlay') && path_is_admin($_GET['q'])) {
      // The overlay is visualized.
    }
    else {
      $redirect = "media/{$file->fid}/edit";
    }

    field_attach_submit('media', $file, $form, $form_state);
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
    drupal_set_message(t('Already exists'), 'warning');
  }
  $form_state['values']['redirect'] = $redirect;
  $form['#attributes']['target'] = '_parent';
}

/**
 * Page callback: Current posts settings.
 *
 * @see current_posts_menu()
 */
function media_mediahaven_form($form, &$form_state) {
  $form['media_mediahaven_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaHaven API'),
    '#description' => t('You will first need to apply for an API Developer Key'),
    '#collapsible' => TRUE,
  );

  $form['media_mediahaven_settings'][media_mediahaven_variable_name('login')] = array(
    '#type' => 'textfield',
    '#title' => t('MediaHaven login'),
    '#default_value' => media_mediahaven_variable_get('login'),
    '#size' => 25,
    '#description' => t('The login to be used by the module to communicate with MediaHaven.'),
    '#required' => TRUE,
  );
  $form['media_mediahaven_settings'][media_mediahaven_variable_name('password')] = array(
    '#type' => 'textfield',
    '#title' => t('MediaHaven password'),
    '#default_value' => media_mediahaven_variable_get('password'),
    '#size' => 25,
    '#description' => t('The password to be used by the module to communicate with MediaHaven.'),
    '#required' => TRUE,
  );

  $form['media_mediahaven_settings'][media_mediahaven_variable_name('url')] = array(
    '#type' => 'textfield',
    '#title' => t('MediaHaven url'),
    '#default_value' => media_mediahaven_variable_get('url'),
    '#size' => 50,
    '#description' => t('The MediaHaven url.'),
    '#required' => TRUE,
  );

  $form['media_mediahaven_upload_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaHaven Upload settings'),
    '#description' => t('Manage the configuration that will be used when you upload files to MediaHaven'),
    '#collapsible' => TRUE,
  );

  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('ingestspace_id')] = array(
    '#type' => 'textfield',
    '#title' => t('Ingestspace id'),
    '#default_value' => media_mediahaven_variable_get('ingestspace_id'),
    '#size' => 50,
    '#description' => t('The MediaHaven ingestspace that will be used to upload files to.'),
    '#required' => FALSE,
  );

  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('autopublish')] = array(
    '#type' => 'checkbox',
    '#title' => t('Auto publish'),
    '#default_value' => media_mediahaven_variable_get('autopublish'),
    '#size' => 50,
    '#description' => t('When enabled files will be automatically published after processing.
          If disabled a user will have to manually publish them using the
          mediahaven media management software'),
  );
  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('department_id')] = array(
    '#type' => 'textfield',
    '#title' => t('Department id'),
    '#default_value' => media_mediahaven_variable_get('department_id'),
    '#size' => 50,
    '#description' => t('The default department where uploaded files will belong to.
          This will be used when autopublishing is enabled'),
    '#required' => FALSE,
  );

  $form['media_mediahaven_rights_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaHaven Rights settings'),
    '#description' => t('Manage rights and copyright notice for media from Mediahaven'),
    '#collapsible' => TRUE,
  );
  $form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('rights_owner')] = array(
    '#type' => 'textfield',
    '#title' => t('Rights owner'),
    '#default_value' => media_mediahaven_variable_get('rights_owner'),
    '#size' => 50,
    '#description' => t('The rights owner for media uploaded to Mediahaven.'),
    '#required' => TRUE,
  );
  $form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('rights')] = array(
    '#type' => 'textfield',
    '#title' => t('Rights'),
    '#default_value' => media_mediahaven_variable_get('rights'),
    '#size' => 50,
    '#description' => t('The rights for using and ditributing the uploaded media.'),
    '#required' => TRUE,
  );
  $form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('copyright')] = array(
    '#type' => 'textfield',
    '#title' => t('Copyright Notice'),
    '#default_value' => media_mediahaven_variable_get('copyright'),
    '#size' => 50,
    '#description' => t('Copyright notice for media uploaded to Mediahaven.'),
    '#required' => TRUE,
  );

  // Third form to store the thumbnail settings: dimensions.
  $form['media_mediahaven_thumbnail_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaHaven Thumbnail settings'),
    '#description' => t('Manage the configuration that will to dispaly the MediaHaven thumbnails'),
    '#collapsible' => TRUE,
  );
  $form['media_mediahaven_thumbnail_settings'][media_mediahaven_variable_name('thumb_width')] = array(
    '#type' => 'textfield',
    '#title' => t('Thumb_width'),
    '#default_value' => media_mediahaven_variable_get('thumb_width'),
    '#size' => 5,
    '#description' => t('Width of the thumbnails (in px)'),
    '#required' => TRUE,
  );
  $form['media_mediahaven_thumbnail_settings'][media_mediahaven_variable_name('thumb_height')] = array(
    '#type' => 'textfield',
    '#title' => t('Thumb_height'),
    '#default_value' => media_mediahaven_variable_get('thumb_height'),
    '#size' => 5,
    '#description' => t('Height of the thumbnails (in px)'),
    '#required' => TRUE,
  );
  $form['media_mediahaven_upload_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload settings'),
    '#collapsible' => TRUE,
  );

  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('upload_choice')] = array(
    '#type' => 'checkbox',
    '#title' => t('Add uploaded files to MediaHaven by default?'),
    '#default_value' => media_mediahaven_variable_get('upload_choice', FALSE),
  );

  return system_settings_form($form);
}

/**
 * Implements validation from the Form API.
 */
function media_mediahaven_form_validate($form, &$form_state) {

  if ($form_state['values'][media_mediahaven_variable_name('ingestspace_id')]) {
    $form_state['values'][media_mediahaven_variable_name('ingestspace_id')] = trim($form_state['values'][media_mediahaven_variable_name('ingestspace_id')]);
    if (!media_mediahaven_validate_uuid($form_state['values'][media_mediahaven_variable_name('ingestspace_id')])) {
      form_set_error(media_mediahaven_variable_name('ingestspace_id'), t('Ingestspace id is not a valid uuid.'));
    }
  }

  if ($form_state['values'][media_mediahaven_variable_name('department_id')]) {
    $form_state['values'][media_mediahaven_variable_name('department_id')] = trim($form_state['values'][media_mediahaven_variable_name('department_id')]);
    if (!media_mediahaven_validate_uuid($form_state['values'][media_mediahaven_variable_name('department_id')])) {
      form_set_error(media_mediahaven_variable_name('department_id'), t('Department id is not a valid uuid.'));
    }
  }
}

/**
 * Validate uuid.
 */
function media_mediahaven_validate_uuid($uuid = FALSE) {
  if ($uuid === FALSE) {
    return FALSE;
  }

  if (preg_match('/^\{?[A-Za-z0-9]{8}-[A-Za-z0-9]{4}-[A-Za-z0-9]{4}-[A-Za-z0-9]{4}-[A-Za-z0-9]{12}\}?$/', $uuid)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}
