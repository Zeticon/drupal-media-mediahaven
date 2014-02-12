<?php

/**
 * Callback for /admin/configure/media/media_mediahaven.
 */
include_once ('MediaHavenRestClient.inc');
include_once ('media_mediahaven.variables.inc');


function media_mediahaven_add($form, &$form_state = array(), $redirect = NULL)
{
	module_load_include('inc', 'media', 'includes/media.browser');

	$uuid = _get_parameter($_GET, 'uuid');

	if ($uuid !== null) {
		_submit_mob($form, $form_state);
		return;
	}

	$data['mediahaven_search'] = _generate_search($form_state);

	return array('mediahaven' => $data);
}

function _generate_search($state)
{
	$data = array(
		'#type' => 'fieldset',
		#'#title' => t('Search in Mediahaven'),
	);

	// used search string
	$search = _get_search_string($state);
	// selected facets
	$selected_facets = _get_selected_facets($state);
	$facets_output = _parse_selected_facets($selected_facets);

	$facets = _get_search_facets($selected_facets, $search);

	// search results
	$results = _get_search_results($search,$selected_facets);

	$data['search'] = array(
		'#type' => 'textfield',
		'#title' => 'Search',
		'#default_value' => $search !== null ? $search : ''
	);

	$data['selected_facets'] = $facets_output;

	$data['facets'] = _generate_facets($facets);

	$data['button'] = array(
		'#type' => 'button',
		'#value' => 'search',
		'#submit' => array('media_mediahaven_add_test')
	);

	$data['results'] = _generate_results($results);

	$data['pager'] = _generate_pager($results['totalNrOfResults'], 0, array('search' => $search));

	return $data;
}

function _get_parameter($container, $key, $default = null) {
	if (isset($container[$key])) {
		return $container[$key];
	}
	return $default;
}

function _get_search_string($state)
{
	$search = null;

	if (isset($state['values']['search'])) {
		$search = $state['values']['search'];
	//} elseif (isset($_POST['search'])) {
	//	$search = $_POST['search'];
	} elseif (isset($_GET['search'])) {
		$search = check_plain(filter_xss($_GET['search']));
	}
	return $search;
}

function _get_selected_facets($state)
{
	$selected_facets = array();
	// array('fa' => 'atom', 'fl' => 'label', 'fk' => 'keyword')

	if (isset($state['values']['search'])) {
		return $selected_facets;
	}

	if(isset($_GET['fa'])) $selected_facets = (array)$_GET['fa'];

	return $selected_facets;
}

function _get_search_facets($selected_facets, $search_term)
{
	$full_query = "";
	foreach($selected_facets as $selected_facet)
	{
		$full_query .= $selected_facet;
	}

	if (isset($search_term) && strlen($search_term)>0) {
		if (strlen($full_query) > 0) {
			$full_query .= ' ';
		}
		$full_query .= '+(' . $search_term . ')';
	}

    $rest_client = get_mediahaven_rest_client();
	$result = $rest_client->getFacets($full_query);

	$arr_facets_json = json_decode($result, TRUE);

	if($arr_facets_json && isset($arr_facets_json['facet']))
	{
		return $arr_facets_json['facet'];
	} else {
	    drupal_set_message($rest_client->error, 'error');
		return FALSE;
	}
}

function _parse_selected_facets($selected_facets)
{
	if(!$selected_facets) return FALSE;
	if(!$selected_facets[0]) return FALSE;

	$pattern = '/\+dtag\:\"root>([A-Za-z0-9 ]*)>([A-Za-z0-9 ]*)\"/';
	preg_match_all($pattern, $selected_facets[0], $matches);


	if(!empty($matches))
	{
		$selected_facet_list = array(
			'#theme' => 'item_list',
			'#type' => 'ul',
			'#title' => t('Selected Facets'),
			'#attributes' => array('class' => 'selected_facet_list'),
		);

		foreach($matches[0] as $key => $match)
		{
			$selected_facet_list['#items'][] = $matches[1][$key] . ' > ' . $matches[2][$key];
		}

		return $selected_facet_list;

	}

	return FALSE;
}

function _generate_facets($facets)
{
	if(empty($facets)) return array();

	// name => string (keyword)
	// title => string
	// results => int
	// values => array

	$facets_list = array(
		'#theme' => 'item_list',
		'#type' => 'ul',
		'#title' => t('Selectable Facets'),
		'#attributes' => array('class' => 'facet_list'),
	);

	foreach($facets as $facet_category)
	{
		$children = _generate_facets_link($facet_category['value']);

		$facet_category = array('data' 		=> $facet_category['title'],
								'id' 		=> $facet_category['name'],
								'class' 	=> array('facet_title'),
								'children'	=> $children
			);
		$facets_list['#items'][] = $facet_category;
	}

	return $facets_list;
}

function _generate_facets_link($facets)
{
	$max_facets = 5;
	//	label
	//	atom
	//	query
	//	count
	$facet_links = array();

	foreach($facets as $facet_item)
	{
		$facet_links[] = '<a href="' . '?fa[]=' . parse_facet_link($facet_item['query']) . '">'
							. htmlentities($facet_item['label'])
							. '</a> (' . $facet_item['count'] . ')';
	}
	if(count($facet_links)>$max_facets)
	{
		$facet_links = array_slice($facet_links,0,$max_facets);
	}
	return $facet_links;
}

function parse_facet_link($query)
{
	$pattern = '/\+\([A-Za-z0-9 ]*\)/';
	preg_match($pattern, $query, $matches);
	$query = urlencode($query);
	if(!empty($matches))
	{
		$query = $query . '&search=' . substr(trim($matches[0]),2,-1);
	}

	return $query;
}

function _get_search_results($search,$selected_facets)
{
	$startIndex = 0;
	$nrOfResults  = 25;
	$query = '';

	if(isset($_GET['page']))
	{
		$current_page = (int)filter_xss($_GET['page']);
		$startIndex = $current_page * $nrOfResults;
	}

    $query = $search;

	if (isset($selected_facets)) {
		foreach($selected_facets as $selected_facet)
		{
			$query .= $selected_facet;
		}
	}

	$rest_client = get_mediahaven_rest_client();
	$result = $rest_client->search($query, $startIndex, $nrOfResults);

	if ($result) {
		$arr_json = json_decode($result, TRUE);
	} else {
		drupal_set_message (t('Could not retrieve search results:' . $rest_client->error), 'error');
	}

	return $arr_json;
}

function _generate_results($results)
{
	if(isset($results['totalNrOfResults'])) unset($results['totalNrOfResults']);
	if(isset($results['startIndex'])) unset($results['startIndex']);
	$results = $results['mediaDataList'];

	$output = array(
		'#theme' => 'item_list',
		'#type' => 'ul',
		'#attributes' => array('class' => 'result_list'),
	);

	if(!empty($results))
	{
		foreach($results as $media_item)
		{
			$output['#items'][] = _generate_result_current($media_item);
		}
	} else {
		return FALSE;
	}


	return $output;
}

function _generate_result_current($media_item) {
	if($media_item['previewImagePath']=='') return FALSE;

	$variables = array();
	$variables['path'] = $media_item['previewImagePath'];
	$variables['width'] = media_mediahaven_variable_get('thumb_width');
	$variables['height'] = media_mediahaven_variable_get('thumb_height');

	//return theme('image', $variables);
	//current_path() . '?uuid=

	return l('<img width="' . $variables['width'] . '" height="' . $variables['height'] . '" src="' . $media_item['previewImagePath'] . '"/><span class="media_object_type_' . $media_item['type'] . '">' . $media_item['type'] . '</span>'
		, 'admin/content/media/add/media_mediahaven/'
		, array('attributes' => array('class' => 'anchor-class', 'rel' => $media_item['fragmentId'])
					, 'html' => TRUE
					, 'query' => array('uuid' =>  $media_item['fragmentId'] )));

}

function _generate_pager($num_results, $element = 0, $parameters = array()) {
	//zoekwaard uit link overschrijven met search
	if(isset($_POST['search'])) $_GET['search'] = filter_xss(check_plain($_POST['search']));

	pager_default_initialize($num_results, MEDIA_MEDIAHAVEN_PAGER, $element);
	return array('#markup' => theme('pager', array('parameters' => $parameters, 'element' => $element)));
}



// save file
function _submit_mob(&$form, &$form_state) {
	$uuid = _get_parameter($_GET, 'uuid');

	$redirect = 'media_mediahaven_add';

	// alle info van het mediaobject ophalen

	$rest_client = get_mediahaven_rest_client();
	$result = $rest_client->getMediaObject($uuid);

	if ($result) {
	  $arr_json = json_decode($result, TRUE);
	} else {
	  drupal_set_message($rest_client->error, 'error');
	  return false;
	}

	// Check database for this uri
	$uri = file_stream_wrapper_uri_normalize("mediahaven://f/$uuid");
	$file = file_uri_to_object($uri, TRUE);
	if($arr_json['type']=='Image')
	{
		$file->filemime = 'image/jpeg';
	} elseif($arr_json['type']=='Video')
	{
		$file->filemime = 'video/mp4';
	}

	//$file->filemime = file_get_mimetype($uri);
	$file->filesize = @filesize($uri);
	$file->filename = $arr_json['title'];

	if ($file === FALSE) {
		drupal_set_message(t('An error occurred and no file was saved.'), 'error');
		$redirect = "media_mediahaven_add";
	} elseif (!property_exists($file, 'fid')) {
		$file = file_save($file);

		if (module_exists('overlay') && path_is_admin($_GET['q'])) {
		// The overlay is visualized.

		} else {
			$redirect = "media/{$file->fid}/edit";
		}

		// I do not understand the block below
		field_attach_submit('media', $file, $form, $form_state);
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
		//drupal_goto("media/{$file->fid}/edit");
	}
	$form_state['values']['redirect'] = $redirect;
	$form['#attributes']['target'] = '_parent';

}

/**
 * Page callback: Current posts settings
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

  $form['media_mediahaven_settings'][media_mediahaven_variable_name('login')] =
  array(
      '#type' => 'textfield',
      '#title' => t('MediaHaven login'),
      '#default_value' => media_mediahaven_variable_get('login'),
      '#size' => 25,
      '#description' => t('The login to be used by the module to communicate with MediaHaven.'),
      '#required' => TRUE,
  );
  $form['media_mediahaven_settings'][media_mediahaven_variable_name('password')] =
  array(
      '#type' => 'textfield',
      '#title' => t('MediaHaven password'),
      '#default_value' => media_mediahaven_variable_get('password'),
      '#size' => 25,
      '#description' => t('The password to be used by the module to communicate with MediaHaven.'),
      '#required' => TRUE,
  );

  $form['media_mediahaven_settings'][media_mediahaven_variable_name('url')] =
  array(
      '#type' => 'textfield',
      '#title' => t('MediaHaven url'),
      '#default_value' => media_mediahaven_variable_get('url'),
      '#size' => 50,
      '#description' => t('The MediaHaven url.'),
      '#required' => TRUE,
  );

  //second form to store upload settings: departement to be used, auto publish, ...
  $form['media_mediahaven_upload_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('MediaHaven Upload settings'),
      '#description' => t('Manage the configuration that will be used when you upload files to MediaHaven'),
      '#collapsible' => TRUE,
  );

  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('ingestspace_id')] =
  array(
      '#type' => 'textfield',
      '#title' => t('Ingestspace id'),
      '#default_value' => media_mediahaven_variable_get('ingestspace_id'),
      '#size' => 50,
      '#description' => t('The MediaHaven ingestspace that will be used to upload files to.'),
      '#required' => TRUE,
  );

  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('autopublish')] =
  array(
      '#type' => 'checkbox',
      '#title' => t('Auto publish'),
      '#default_value' => media_mediahaven_variable_get('autopublish'),
      '#size' => 50,
      '#description' => t('When enabled files will be automatically published after processing.
          If disabled a user will have to manually publish them using the
          mediahaven media management software'),

  );
  $form['media_mediahaven_upload_settings'][media_mediahaven_variable_name('department_id')] =
  array(
      '#type' => 'textfield',
      '#title' => t('Department id'),
      '#default_value' => media_mediahaven_variable_get('department_id'),
      '#size' => 50,
      '#description' => t('The default department where uploaded files will belong to.
          This will be used when autopublishing is enabled'),
      '#required' => TRUE,
  );

	$form['media_mediahaven_rights_settings'] = array(
	  '#type' => 'fieldset',
	  '#title' => t('MediaHaven Rights settings'),
	  '#description' => t('Manage rights and copyright notice for media from Mediahaven'),
	  '#collapsible' => TRUE,
	);
	$form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('rights_owner')] =
	array(
		'#type' => 'textfield',
		'#title' => t('Rights owner'),
		'#default_value' => media_mediahaven_variable_get('rights_owner'),
		'#size' => 50,
		'#description' => t('The rights owner for media uploaded to Mediahaven.'),
		'#required' => TRUE,
	);
	$form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('rights')] =
	array(
		'#type' => 'textfield',
		'#title' => t('Rights'),
		'#default_value' => media_mediahaven_variable_get('rights'),
		'#size' => 50,
		'#description' => t('The rights for using and ditributing the uploaded media.'),
		'#required' => TRUE,
	);
	$form['media_mediahaven_rights_settings'][media_mediahaven_variable_name('copyright')] =
	array(
		'#type' => 'textfield',
		'#title' => t('Copyright Notice'),
		'#default_value' => media_mediahaven_variable_get('copyright'),
		'#size' => 50,
		'#description' => t('Copyright notice for media uploaded to Mediahaven.'),
		'#required' => TRUE,
	);


	//third form to store the thumbnail settings: dimensions
	$form['media_mediahaven_thumbnail_settings'] = array(
	  '#type' => 'fieldset',
	  '#title' => t('MediaHaven Thumbnail settings'),
	  '#description' => t('Manage the configuration that will to dispaly the MediaHaven thumbnails'),
	  '#collapsible' => TRUE,
	);
	$form['media_mediahaven_thumbnail_settings'][media_mediahaven_variable_name('thumb_width')] =
	array(
		'#type' => 'textfield',
		'#title' => t('Thumb_width'),
		'#default_value' => media_mediahaven_variable_get('thumb_width'),
		'#size' => 5,
		'#description' => t('Width of the thumbnails (in px)'),
		'#required' => TRUE,
	);
	$form['media_mediahaven_thumbnail_settings'][media_mediahaven_variable_name('thumb_height')] =
	array(
		'#type' => 'textfield',
		'#title' => t('Thumb_height'),
		'#default_value' => media_mediahaven_variable_get('thumb_height'),
		'#size' => 5,
		'#description' => t('Height of the thumbnails (in px)'),
		'#required' => TRUE,
	);

  return system_settings_form($form);
}


/**
 * Implements validation from the Form API.
 *
 * @param $form
 *   A structured array containing the elements and properties of the form.
 * @param $form_state
 *   An array that stores information about the form's current state
 *   during processing.
 */
function media_mediahaven_form_validate($form, &$form_state){

	//if($form_state['input']['media_mediahaven__ingestspace_id'])
	$form_state['values']['media_mediahaven__ingestspace_id'] = trim($form_state['values']['media_mediahaven__ingestspace_id']);
	if($form_state['values']['media_mediahaven__ingestspace_id'])
	{
		if(!validate_uuid($form_state['values']['media_mediahaven__ingestspace_id']))
		{
			form_set_error('media_mediahaven__ingestspace_id', t('Ingestspace id is not a valid uuid.'));
		}
	}

	//if($form_state['input']['media_mediahaven__department_id'])
	$form_state['values']['media_mediahaven__department_id'] = trim($form_state['values']['media_mediahaven__department_id']);
	if($form_state['values']['media_mediahaven__department_id'])
	{
		if(!validate_uuid($form_state['values']['media_mediahaven__department_id']))
		{
			form_set_error('media_mediahaven__department_id', t('Department id is not a valid uuid.'));
		}
	}
}

/**
 * Validate uuid
 */
function validate_uuid($uuid = FALSE)
{
	if($uuid === FALSE) return FALSE;

	if (preg_match('/^\{?[A-Za-z0-9]{8}-[A-Za-z0-9]{4}-[A-Za-z0-9]{4}-[A-Za-z0-9]{4}-[A-Za-z0-9]{12}\}?$/', $uuid)) {
		return TRUE;
	} else {
		return FALSE;
	}
}