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

	$facets = _get_search_facets($selected_facets, $search);

	// search results
	$results = _get_search_results($search,$selected_facets);

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

	if(isset($_GET['fa'])) $selected_facets = (array)filter_xss($_GET['fa']);

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
	$query = str_replace(' +(dtag:"root")','',$query);
	$r = explode('+(', $query);

	if (isset($r[1])){
		$r[1] = substr(trim($r[1]),0,-1);
		return urlencode(trim($r[0])) . '&search=' . $r[1];
	}
	return urlencode(trim($r[0]));
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
		, array('attributes' => array('class' => 'anchor-class')
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
function _submit_mob($form, &$form_state) {
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
	}
	$form_state['values']['redirect'] = $redirect;
}

function file_drupal_mime_types($file_ext) {

  $mime_types = array(
    'ez' => 'application/andrew-inset',
    'atom' => 'application/atom',
    'atomcat' => 'application/atomcat+xml',
    'atomsrv' => 'application/atomserv+xml',
    'cap|pcap' => 'application/cap',
    'cu' => 'application/cu-seeme',
    'tsp' => 'application/dsptype',
    'spl' => 'application/x-futuresplash',
    'hta' => 'application/hta',
    'jar' => 'application/java-archive',
    'ser' => 'application/java-serialized-object',
    'class' => 'application/java-vm',
    'hqx' => 'application/mac-binhex40',
    'cpt' => 'image/x-corelphotopaint',
    'nb' => 'application/mathematica',
    'mdb' => 'application/msaccess',
    'doc|dot' => 'application/msword',
    'bin' => 'application/octet-stream',
    'oda' => 'application/oda',
    'ogg|ogx' => 'application/ogg',
    'pdf' => 'application/pdf',
    'key' => 'application/pgp-keys',
    'pgp' => 'application/pgp-signature',
    'prf' => 'application/pics-rules',
    'ps|ai|eps' => 'application/postscript',
    'rar' => 'application/rar',
    'rdf' => 'application/rdf+xml',
    'rss' => 'application/rss+xml',
    'rtf' => 'application/rtf',
    'smi|smil' => 'application/smil',
    'wpd' => 'application/wordperfect',
    'wp5' => 'application/wordperfect5.1',
    'xhtml|xht' => 'application/xhtml+xml',
    'xml|xsl' => 'application/xml',
    'zip' => 'application/zip',
    'cdy' => 'application/vnd.cinderella',
    'kml' => 'application/vnd.google-earth.kml+xml',
    'kmz' => 'application/vnd.google-earth.kmz',
    'xul' => 'application/vnd.mozilla.xul+xml',
    'xls|xlb|xlt' => 'application/vnd.ms-excel',
    'cat' => 'application/vnd.ms-pki.seccat',
    'stl' => 'application/vnd.ms-pki.stl',
    'ppt|pps' => 'application/vnd.ms-powerpoint',
    'odc' => 'application/vnd.oasis.opendocument.chart',
    'odb' => 'application/vnd.oasis.opendocument.database',
    'odf' => 'application/vnd.oasis.opendocument.formula',
    'odg' => 'application/vnd.oasis.opendocument.graphics',
    'otg' => 'application/vnd.oasis.opendocument.graphics-template',
    'odi' => 'application/vnd.oasis.opendocument.image',
    'odp' => 'application/vnd.oasis.opendocument.presentation',
    'otp' => 'application/vnd.oasis.opendocument.presentation-template',
    'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
    'ots' => 'application/vnd.oasis.opendocument.spreadsheet-template',
    'odt' => 'application/vnd.oasis.opendocument.text',
    'odm' => 'application/vnd.oasis.opendocument.text-master',
    'ott' => 'application/vnd.oasis.opendocument.text-template',
    'oth' => 'application/vnd.oasis.opendocument.text-web',
    'docm' => 'application/vnd.ms-word.document.macroEnabled.12',
    'docx' => 'application/vnd.openxmlformats-officedocument.wordprocessingml.document',
    'dotm' => 'application/vnd.ms-word.template.macroEnabled.12',
    'dotx' => 'application/vnd.openxmlformats-officedocument.wordprocessingml.template',
    'potm' => 'application/vnd.ms-powerpoint.template.macroEnabled.12',
    'potx' => 'application/vnd.openxmlformats-officedocument.presentationml.template',
    'ppam' => 'application/vnd.ms-powerpoint.addin.macroEnabled.12',
    'ppsm' => 'application/vnd.ms-powerpoint.slideshow.macroEnabled.12',
    'ppsx' => 'application/vnd.openxmlformats-officedocument.presentationml.slideshow',
    'pptm' => 'application/vnd.ms-powerpoint.presentation.macroEnabled.12',
    'pptx' => 'application/vnd.openxmlformats-officedocument.presentationml.presentation',
    'xlam' => 'application/vnd.ms-excel.addin.macroEnabled.12',
    'xlsb' => 'application/vnd.ms-excel.sheet.binary.macroEnabled.12',
    'xlsm' => 'application/vnd.ms-excel.sheet.macroEnabled.12',
    'xlsx' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
    'xltm' => 'application/vnd.ms-excel.template.macroEnabled.12',
    'xltx' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.template',
    'cod' => 'application/vnd.rim.cod',
    'mmf' => 'application/vnd.smaf',
    'sdc' => 'application/vnd.stardivision.calc',
    'sds' => 'application/vnd.stardivision.chart',
    'sda' => 'application/vnd.stardivision.draw',
    'sdd' => 'application/vnd.stardivision.impress',
    'sdf' => 'application/vnd.stardivision.math',
    'sdw' => 'application/vnd.stardivision.writer',
    'sgl' => 'application/vnd.stardivision.writer-global',
    'sxc' => 'application/vnd.sun.xml.calc',
    'stc' => 'application/vnd.sun.xml.calc.template',
    'sxd' => 'application/vnd.sun.xml.draw',
    'std' => 'application/vnd.sun.xml.draw.template',
    'sxi' => 'application/vnd.sun.xml.impress',
    'sti' => 'application/vnd.sun.xml.impress.template',
    'sxm' => 'application/vnd.sun.xml.math',
    'sxw' => 'application/vnd.sun.xml.writer',
    'sxg' => 'application/vnd.sun.xml.writer.global',
    'stw' => 'application/vnd.sun.xml.writer.template',
    'sis' => 'application/vnd.symbian.install',
    'vsd' => 'application/vnd.visio',
    'wbxml' => 'application/vnd.wap.wbxml',
    'wmlc' => 'application/vnd.wap.wmlc',
    'wmlsc' => 'application/vnd.wap.wmlscriptc',
    'wk' => 'application/x-123',
    '7z' => 'application/x-7z-compressed',
    'abw' => 'application/x-abiword',
    'dmg' => 'application/x-apple-diskimage',
    'bcpio' => 'application/x-bcpio',
    'torrent' => 'application/x-bittorrent',
    'cab' => 'application/x-cab',
    'cbr' => 'application/x-cbr',
    'cbz' => 'application/x-cbz',
    'cdf' => 'application/x-cdf',
    'vcd' => 'application/x-cdlink',
    'pgn' => 'application/x-chess-pgn',
    'cpio' => 'application/x-cpio',
    'csh' => 'text/x-csh',
    'deb|udeb' => 'application/x-debian-package',
    'dcr|dir|dxr' => 'application/x-director',
    'dms' => 'application/x-dms',
    'wad' => 'application/x-doom',
    'dvi' => 'application/x-dvi',
    'rhtml' => 'application/x-httpd-eruby',
    'flac' => 'application/x-flac',
    'pfa|pfb|gsf|pcf|pcf.Z' => 'application/x-font',
    'mm' => 'application/x-freemind',
    'gnumeric' => 'application/x-gnumeric',
    'sgf' => 'application/x-go-sgf',
    'gcf' => 'application/x-graphing-calculator',
    'gtar|tgz|taz' => 'application/x-gtar',
    'hdf' => 'application/x-hdf',
    'phtml|pht|php' => 'application/x-httpd-php',
    'phps' => 'application/x-httpd-php-source',
    'php3' => 'application/x-httpd-php3',
    'php3p' => 'application/x-httpd-php3-preprocessed',
    'php4' => 'application/x-httpd-php4',
    'ica' => 'application/x-ica',
    'ins|isp' => 'application/x-internet-signup',
    'iii' => 'application/x-iphone',
    'iso' => 'application/x-iso9660-image',
    'jnlp' => 'application/x-java-jnlp-file',
    'js' => 'application/x-javascript',
    'jmz' => 'application/x-jmol',
    'chrt' => 'application/x-kchart',
    'kil' => 'application/x-killustrator',
    'skp|skd|skt|skm' => 'application/x-koan',
    'kpr|kpt' => 'application/x-kpresenter',
    'ksp' => 'application/x-kspread',
    'kwd|kwt' => 'application/x-kword',
    'latex' => 'application/x-latex',
    'lha' => 'application/x-lha',
    'lyx' => 'application/x-lyx',
    'lzh' => 'application/x-lzh',
    'lzx' => 'application/x-lzx',
    'frm|maker|frame|fm|fb|book|fbdoc' => 'application/x-maker',
    'mif' => 'application/x-mif',
    'wmd' => 'application/x-ms-wmd',
    'wmz' => 'application/x-ms-wmz',
    'com|exe|bat|dll' => 'application/x-msdos-program',
    'msi' => 'application/x-msi',
    'nc' => 'application/x-netcdf',
    'pac' => 'application/x-ns-proxy-autoconfig',
    'nwc' => 'application/x-nwc',
    'o' => 'application/x-object',
    'oza' => 'application/x-oz-application',
    'p7r' => 'application/x-pkcs7-certreqresp',
    'crl' => 'application/x-pkcs7-crl',
    'pyc|pyo' => 'application/x-python-code',
    'qtl' => 'application/x-quicktimeplayer',
    'rpm' => 'application/x-redhat-package-manager',
    'sh' => 'text/x-sh',
    'shar' => 'application/x-shar',
    'swf|swfl' => 'application/x-shockwave-flash',
    'sit|sitx' => 'application/x-stuffit',
    'sv4cpio' => 'application/x-sv4cpio',
    'sv4crc' => 'application/x-sv4crc',
    'tar' => 'application/x-tar',
    'tcl' => 'application/x-tcl',
    'gf' => 'application/x-tex-gf',
    'pk' => 'application/x-tex-pk',
    'texinfo|texi' => 'application/x-texinfo',
    '~|%|bak|old|sik' => 'application/x-trash',
    't|tr|roff' => 'application/x-troff',
    'man' => 'application/x-troff-man',
    'me' => 'application/x-troff-me',
    'ms' => 'application/x-troff-ms',
    'ustar' => 'application/x-ustar',
    'src' => 'application/x-wais-source',
    'wz' => 'application/x-wingz',
    'crt' => 'application/x-x509-ca-cert',
    'xcf' => 'application/x-xcf',
    'fig' => 'application/x-xfig',
    'xpi' => 'application/x-xpinstall',
    'au|snd' => 'audio/basic',
    'mid|midi|kar' => 'audio/midi',
    'mpga|mpega|mp2|mp3|m4a' => 'audio/mpeg',
    'm3u' => 'audio/x-mpegurl',
    'oga|spx' => 'audio/ogg',
    'sid' => 'audio/prs.sid',
    'aif|aiff|aifc' => 'audio/x-aiff',
    'gsm' => 'audio/x-gsm',
    'wma' => 'audio/x-ms-wma',
    'wax' => 'audio/x-ms-wax',
    'ra|rm|ram' => 'audio/x-pn-realaudio',
    'ra' => 'audio/x-realaudio',
    'pls' => 'audio/x-scpls',
    'sd2' => 'audio/x-sd2',
    'wav' => 'audio/x-wav',
    'alc' => 'chemical/x-alchemy',
    'cac|cache' => 'chemical/x-cache',
    'csf' => 'chemical/x-cache-csf',
    'cbin|cascii|ctab' => 'chemical/x-cactvs-binary',
    'cdx' => 'chemical/x-cdx',
    'cer' => 'chemical/x-cerius',
    'c3d' => 'chemical/x-chem3d',
    'chm' => 'chemical/x-chemdraw',
    'cif' => 'chemical/x-cif',
    'cmdf' => 'chemical/x-cmdf',
    'cml' => 'chemical/x-cml',
    'cpa' => 'chemical/x-compass',
    'bsd' => 'chemical/x-crossfire',
    'csml|csm' => 'chemical/x-csml',
    'ctx' => 'chemical/x-ctx',
    'cxf|cef' => 'chemical/x-cxf',
    'emb|embl' => 'chemical/x-embl-dl-nucleotide',
    'spc' => 'chemical/x-galactic-spc',
    'inp|gam|gamin' => 'chemical/x-gamess-input',
    'fch|fchk' => 'chemical/x-gaussian-checkpoint',
    'cub' => 'chemical/x-gaussian-cube',
    'gau|gjc|gjf' => 'chemical/x-gaussian-input',
    'gal' => 'chemical/x-gaussian-log',
    'gcg' => 'chemical/x-gcg8-sequence',
    'gen' => 'chemical/x-genbank',
    'hin' => 'chemical/x-hin',
    'istr|ist' => 'chemical/x-isostar',
    'jdx|dx' => 'chemical/x-jcamp-dx',
    'kin' => 'chemical/x-kinemage',
    'mcm' => 'chemical/x-macmolecule',
    'mmd|mmod' => 'chemical/x-macromodel-input',
    'mol' => 'chemical/x-mdl-molfile',
    'rd' => 'chemical/x-mdl-rdfile',
    'rxn' => 'chemical/x-mdl-rxnfile',
    'sd|sdf' => 'chemical/x-mdl-sdfile',
    'tgf' => 'chemical/x-mdl-tgf',
    'mcif' => 'chemical/x-mmcif',
    'mol2' => 'chemical/x-mol2',
    'b' => 'chemical/x-molconn-Z',
    'gpt' => 'chemical/x-mopac-graph',
    'mop|mopcrt|mpc|dat|zmt' => 'chemical/x-mopac-input',
    'moo' => 'chemical/x-mopac-out',
    'mvb' => 'chemical/x-mopac-vib',
    'asn' => 'chemical/x-ncbi-asn1-spec',
    'prt|ent' => 'chemical/x-ncbi-asn1-ascii',
    'val|aso' => 'chemical/x-ncbi-asn1-binary',
    'pdb|ent' => 'chemical/x-pdb',
    'ros' => 'chemical/x-rosdal',
    'sw' => 'chemical/x-swissprot',
    'vms' => 'chemical/x-vamas-iso14976',
    'vmd' => 'chemical/x-vmd',
    'xtel' => 'chemical/x-xtel',
    'xyz' => 'chemical/x-xyz',
    'gif' => 'image/gif',
    'ief' => 'image/ief',
    'jpeg|jpg|jpe' => 'image/jpeg',
    'pcx' => 'image/pcx',
    'png' => 'image/png',
    'svg|svgz' => 'image/svg+xml',
    'tiff|tif' => 'image/tiff',
    'djvu|djv' => 'image/vnd.djvu',
    'wbmp' => 'image/vnd.wap.wbmp',
    'ras' => 'image/x-cmu-raster',
    'cdr' => 'image/x-coreldraw',
    'pat' => 'image/x-coreldrawpattern',
    'cdt' => 'image/x-coreldrawtemplate',
    'ico' => 'image/x-icon',
    'art' => 'image/x-jg',
    'jng' => 'image/x-jng',
    'bmp' => 'image/x-ms-bmp',
    'psd' => 'image/x-photoshop',
    'pnm' => 'image/x-portable-anymap',
    'pbm' => 'image/x-portable-bitmap',
    'pgm' => 'image/x-portable-graymap',
    'ppm' => 'image/x-portable-pixmap',
    'rgb' => 'image/x-rgb',
    'xbm' => 'image/x-xbitmap',
    'xpm' => 'image/x-xpixmap',
    'xwd' => 'image/x-xwindowdump',
    'eml' => 'message/rfc822',
    'igs|iges' => 'model/iges',
    'msh|mesh|silo' => 'model/mesh',
    'wrl|vrml' => 'model/vrml',
    'ics|icz' => 'text/calendar',
    'css' => 'text/css',
    'csv' => 'text/csv',
    '323' => 'text/h323',
    'html|htm|shtml' => 'text/html',
    'uls' => 'text/iuls',
    'mml' => 'text/mathml',
    'asc|txt|text|pot' => 'text/plain',
    'rtx' => 'text/richtext',
    'sct|wsc' => 'text/scriptlet',
    'tm|ts' => 'text/texmacs',
    'tsv' => 'text/tab-separated-values',
    'jad' => 'text/vnd.sun.j2me.app-descriptor',
    'wml' => 'text/vnd.wap.wml',
    'wmls' => 'text/vnd.wap.wmlscript',
    'bib' => 'text/x-bibtex',
    'boo' => 'text/x-boo',
    'h++|hpp|hxx|hh' => 'text/x-c++hdr',
    'c++|cpp|cxx|cc' => 'text/x-c++src',
    'h' => 'text/x-chdr',
    'htc' => 'text/x-component',
    'c' => 'text/x-csrc',
    'd' => 'text/x-dsrc',
    'diff|patch' => 'text/x-diff',
    'hs' => 'text/x-haskell',
    'java' => 'text/x-java',
    'lhs' => 'text/x-literate-haskell',
    'moc' => 'text/x-moc',
    'p|pas' => 'text/x-pascal',
    'gcd' => 'text/x-pcs-gcd',
    'pl|pm' => 'text/x-perl',
    'py' => 'text/x-python',
    'etx' => 'text/x-setext',
    'tcl|tk' => 'text/x-tcl',
    'tex|ltx|sty|cls' => 'text/x-tex',
    'vcs' => 'text/x-vcalendar',
    'vcf' => 'text/x-vcard',
    '3gp' => 'video/3gpp',
    'dl' => 'video/dl',
    'dif|dv' => 'video/dv',
    'fli' => 'video/fli',
    'gl' => 'video/gl',
    'mpeg|mpg|mpe' => 'video/mpeg',
    'mp4' => 'video/mp4',
    'ogv' => 'video/ogg',
    'qt|mov' => 'video/quicktime',
    'mxu' => 'video/vnd.mpegurl',
    'lsf|lsx' => 'video/x-la-asf',
    'mng' => 'video/x-mng',
    'asf|asx' => 'video/x-ms-asf',
    'wm' => 'video/x-ms-wm',
    'wmv' => 'video/x-ms-wmv',
    'wmx' => 'video/x-ms-wmx',
    'wvx' => 'video/x-ms-wvx',
    'avi' => 'video/x-msvideo',
    'movie' => 'video/x-sgi-movie',
    'ice' => 'x-conference/x-cooltalk',
    'sisx' => 'x-epoc/x-sisx-app',
    'vrm|vrml|wrl' => 'x-world/x-vrml',
    'xps' => 'application/vnd.ms-xpsdocument',
  );

  return $mime_types[$file_ext];
}
