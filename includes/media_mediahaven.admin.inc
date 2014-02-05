<?php

/**
 * Callback for /admin/configure/media/media_mediahaven.
 */
include_once ('MediaHavenRestClient.inc');
include_once ('media_mediahaven.variables.inc');

function _get_mediahaven_rest_client() {
  return new MediaHavenRestClient(media_mediahaven_variable_get('url') . MEDIA_MEDIAHAVEN_RESTPOINT,
      media_mediahaven_variable_get('login'),
      media_mediahaven_variable_get('password'));
}

function media_mediahaven_add($form, &$form_state = array(), $redirect = NULL)
{
	module_load_include('inc', 'media', 'includes/media.browser');
	
	$uuid = _get_parameter($_GET, 'uuid');

	if ($uuid !== null) {
		_submit_mob($form, $form_state);
		return;
	}
	
	$files = array();
	// Add the files to JS so that they are accessible inside the browser
	drupal_add_js(array('media' => array('files' => $files)), 'setting');

	// Add media browser javascript and CSS.
	drupal_add_js(drupal_get_path('module', 'media_mediahaven') . '/js/media-mediahaven.browser.js');
  
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

    $rest_client = _get_mediahaven_rest_client();
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

	if($search!='') $query = '+(' . $search . ')';

	if (isset($selected_facets)) {
		foreach($selected_facets as $selected_facet)
		{
			$query .= $selected_facet;
		}
	}

	$rest_client = _get_mediahaven_rest_client();
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

	//[mediaObjectId] => string
	//[title] => string
	//[description] => text
	//[date] => datetime
	//[previewImagePath] => string
	//[thumbnailImagePath] => string
	//[videoPath] => string
	//[keywords] => array
	//[type] => string
	//[authors] => array

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
	return l('<img width="' . $variables['width'] . '" height="' . $variables['height'] . '" src="' . $media_item['previewImagePath'] . '"/>'
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

	$rest_client = _get_mediahaven_rest_client();
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