<?php

/**
 * Callback for /admin/configure/media/media_mediahaven.
 */
include ('media_mediahaven.config.inc');

function media_mediahaven_add($form, &$form_state = array(), $redirect = NULL)
{
	$uuid = _get_parameter($_GET, 'uuid');

	if ($uuid !== null) {
		_submit_mob($form, $form_state);
		return;
	}

	$data = _generate_tabs();
	$data['mediahaven_search'] = _generate_search($form_state);
	$data['mediahaven_library'] = _generate_library();

	//TSW hack
	//$form_state['method'] = 'get';

	return array('mediahaven' => $data);

}

function _generate_tabs()
{
	$data = array(
		'#type' => 'vertical_tabs'
	);
	return $data;
}

function _generate_library()
{
	$data = array(
		'#type' => 'fieldset',
		'#title' => t('My Mediahaven Library'),
	);

	return $data;
}

function _generate_search($state)
{
	$data = array(
		'#type' => 'fieldset',
		'#title' => t('Search in Mediahaven'),
	);

	// used search string
	$search = _get_search_string($state);
	$facet_query = '';
	if (isset($_GET['facet_query'])) {
		$facet_query = $_GET['facet_query'];
	}
	// selected facets
	$facets = _get_search_facets($facet_query, $search);

	// search results
	$results = _get_search_results($search,$facet_query);

	$data['search'] = array(
		'#type' => 'textfield',
		'#title' => 'Search',
		'#default_value' => $search !== null ? $search : ''
	);

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
	//print_r($state['values']['search']);
	if (isset($state['values']['search'])) {
		$search = $state['values']['search'];
	//} elseif (isset($_POST['search'])) {
	//	$search = $_POST['search'];
	} elseif (isset($_GET['search'])) {
		$search = $_GET['search'];
	}
	return $search;
}

function _get_search_facets($query, $search_term)
{
	$facets = null;

	$crl = curl_init();
	$url = WEBSERVICE_BASE_URL ."/facets";

	if (isset($query) && strlen($query) > 0) {
		$url .= '?q=' . $query;
	}
	if (isset($search_term) && strlen($search_term)>0) {
		if (!strpos($url, '?q=')) {
			$url .= '?q=';
		} else {
			$url .= ' ';
		}
		$url .= $search_term;

	}
	drupal_set_message('facet search url:' . $url);

	curl_setopt($crl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($crl, CURLOPT_USERPWD, WEBSERVICE_LOGIN . ":" . WEBSERVICE_PASS);
	curl_setopt($crl, CURLOPT_URL, $url);
	curl_setopt($crl, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($crl, CURLOPT_COOKIEJAR, "/tmp/cookie.txt");
	curl_setopt($crl, CURLOPT_SSL_VERIFYPEER, false);

	$result = curl_exec($crl);

	$arr_facets_json = json_decode($result, TRUE);

	if(isset($arr_facets_json['facets']))
	{
		return $arr_facets_json['facets'];
	} else {
		return FALSE;
	}
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
		'#attributes' => array('class' => 'facet_list'),
	);

	foreach($facets as $facet_category)
	{
		$children = _generate_facets_link($facet_category['values']);

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
		$facet_links[] = '<a href="' . '?facet_query=' . urlencode($facet_item['query']) . '">'
							. $facet_item['label']
							. '</a> (' . $facet_item['count'] . ')';
	}
	if(count($facet_links)>$max_facets)
	{
		$facet_links = array_slice($facet_links,0,$max_facets);
		$facet_links[] = '<a style="color:#C00;" href="">meer facets tonen...</a>';
	}
	return $facet_links;
}

function _get_search_results($search,$facet_query)
{
	$startIndex = 0;
	$nrOfResults  = 25;

	if(isset($_GET['page']))
	{
		$current_page = (int)$_GET['page'];
		$startIndex = $current_page * $nrOfResults;
	}

	//echo 'zoekterm: ' . $search . '<br/>';
	//echo 'facets: ' . $facets . '<br/>';

	$crl = curl_init();
	$query = $search;
	if (isset($facet_query)) {
		$query .= ' ' . $facet_query;
	}
	$url = WEBSERVICE_BASE_URL . "/media/search?q=" . $query. '&startIndex=' . $startIndex . '&nrOfResults=' . $nrOfResults;

	curl_setopt($crl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($crl, CURLOPT_USERPWD, WEBSERVICE_LOGIN . ":" . WEBSERVICE_PASS);
	curl_setopt($crl, CURLOPT_URL, $url);
	curl_setopt($crl, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($crl, CURLOPT_COOKIEJAR, "/tmp/cookie.txt");
	curl_setopt($crl, CURLOPT_SSL_VERIFYPEER, false);

	$result = curl_exec($crl);

	if ($result) {
		$arr_json = json_decode($result, TRUE);
	} else {
		echo (curl_error($crl));
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

	foreach($results as $media_item)
	{
		$output['#items'][] = _generate_result_current($media_item);
	}

	return $output;
}

function _generate_result_current($media_item) {
	if($media_item['previewImagePath']=='') return FALSE;

	$variables = array();
	$variables['path'] = $media_item['previewImagePath'];
	$variables['width'] = 150;
	$variables['height'] = 150;

	//return theme('image', $variables);
	//current_path() . '?uuid=
	return l('<img width="150" height="150" src="' . $media_item['previewImagePath'] . '"/>'
		, 'admin/content/media/add/media_mediahaven/'
		, array('attributes' => array('class' => 'anchor-class')
					, 'html' => TRUE
					, 'query' => array('uuid' =>  $media_item['mediaObjectId'] )));

}

function _generate_pager($num_results, $element = 0, $parameters = array()) {
	//zoekwaard uit link overschrijven met search
	if(isset($_POST['search'])) $_GET['search'] = $_POST['search'];

	pager_default_initialize($num_results, MEDIA_MEDIAHAVEN_PAGER, $element);
	return array('#markup' => theme('pager', array('parameters' => $parameters, 'element' => $element)));
}



// save file
function _submit_mob($form, &$form_state) {
	$uuid = _get_parameter($_GET, 'uuid');
	$uri = file_stream_wrapper_uri_normalize("mediahaven://f/$uuid");
	$redirect = 'media_mediahaven_add';

	// alle info van het mediaobject ophalen
	$crl = curl_init();
	$url = WEBSERVICE_BASE_URL . "media/" . $uuid;

	curl_setopt($crl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($crl, CURLOPT_USERPWD, WEBSERVICE_LOGIN . ":" . WEBSERVICE_PASS);
	curl_setopt($crl, CURLOPT_URL, $url);
	curl_setopt($crl, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($crl, CURLOPT_COOKIEJAR, "/tmp/cookie.txt");
	curl_setopt($crl, CURLOPT_SSL_VERIFYPEER, false);

	$result = curl_exec($crl);

	$arr_json = json_decode($result, TRUE);


	// Check database for this uri
	$file = file_uri_to_object($uri, TRUE);
	drupal_set_message(var_dump($file));
	$file = file_save($file);
	/*if ($file === FALSE) {
		drupal_set_message(t('An error occurred and no file was saved.'), 'error');
		$redirect = "media_mediahaven_add";
	} else if (property_exists($file, 'is_new')) {
		$file = file_save($file);
		$redirect = "media/{$file->fid}/edit";

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
	}*/
	$form_state['values']['redirect'] = $redirect;
}
