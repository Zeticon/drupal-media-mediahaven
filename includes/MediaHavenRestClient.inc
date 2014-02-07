<?php

class MediaHavenRestClient {

  public $base_rest_url;
  private $options = array();
  private $error = '';
 const SUPPORTED_MEDIATYPES_QUERY = '+(MediaObjectType:Video OR MediaObjectType:Image)';

  function __construct($base_rest_url, $login, $password) {
    $this->base_rest_url = $base_rest_url;
    $this->options = array(
        CURLOPT_HTTPAUTH => CURLAUTH_BASIC,
        CURLOPT_USERPWD => $login .  ":" . $password,
        CURLOPT_RETURNTRANSFER => 1,
        CURLOPT_COOKIEJAR => "/tmp/cookie.txt",
        CURLOPT_SSL_VERIFYPEER => false
    );

  }

  public function __get($property) {
    if (property_exists($this, $property)) {
      return $this->$property;
    }
  }

  private function init_curl() {
    $curl = curl_init();
    foreach($this->options as $option => $value) {
      curl_setopt($curl, $option, $value);
    }
    return $curl;
  }

  public function search($query, $startIndex, $nrOfResults){
    if($query!='') {
      $query = '+(' . $query . ') ';
    }
    if (strpos($query, self::SUPPORTED_MEDIATYPES_QUERY) == FALSE) {
      $query .= self::SUPPORTED_MEDIATYPES_QUERY;
    }
	$query = urlencode($query);

    $url = $this->base_rest_url. '/media?q=' . $query;
	//drupal_set_message('----> ' . $url);
	if (isset($startIndex) && strlen($startIndex) > 0) {
      $url .= '&startIndex=' . $startIndex;
    }
    if (isset($nrOfResults) && strlen($nrOfResults)) {
      $url .= '&nrOfResults=' . $nrOfResults;
    }

    $this->options[CURLOPT_URL] = $url;
	//drupal_set_message('----> ' . $this->options[CURLOPT_URL]);
    return $this->execute();
  }

  public function getMediaObject($id) {
    $this->options[CURLOPT_URL] = $this->base_rest_url. '/media/' . $id;
	//drupal_set_message('----> ' . $this->options[CURLOPT_URL]);
    return $this->execute();
  }

  public function getFacets($query) {
    if (strpos($query, self::SUPPORTED_MEDIATYPES_QUERY) == FALSE) {
      $query .= self::SUPPORTED_MEDIATYPES_QUERY;
    }
    $this->options[CURLOPT_URL] = $this->base_rest_url. '/facets?q=' . urlencode($query);
    //drupal_set_message('----> ' . $this->options[CURLOPT_URL]);
    return $this->execute();
  }

  public function upload($media_file, $metadata_file, $ingestspace_id) {
	if(media_mediahaven_variable_get('autopublish')==1)
	{
		$autopublish = 'true';
	} else {
		$autopublish = 'false';
	}

	$upload_settings = array(
        'file' => '@' . $media_file .';filename=' . substr($media_file, strrpos($media_file, '/')+1),
        'metadata' => '@' . $metadata_file,
        'ingestSpaceId' => $ingestspace_id,
		'autoPublish' => $autopublish,
    );

    $this->options[CURLOPT_URL] = $this->base_rest_url. '/media';
    $this->options[CURLOPT_POST] = 1;
    $this->options[CURLOPT_POSTFIELDS] = $upload_settings;

   drupal_set_message(serialize($this->options[CURLOPT_POSTFIELDS]));
   //file_unmanaged_copy($metadata_file, $metadata_file .'bck');
   return $this->execute();
  }

  private function execute() {
    $curl = $this->init_curl();
    $response = curl_exec($curl);
	$http_code = curl_getinfo($curl, CURLINFO_HTTP_CODE);
	$this->error = curl_error($curl);
	curl_close($curl);
    if (!$response) {
      return false;
    } else if ($http_code != 200) {
		$this->error = $response;
		return false;
	} else{
		return $response;
    }
  }
}


