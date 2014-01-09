<?php

class MediaHavenRestClient {

  public $base_rest_url;
  private $options = array();
  private $error = '';

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
	$query = urlencode($query);
    $url = $this->base_rest_url. '/media/search?q=' . $query;
	drupal_set_message('----> ' . $url);
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
    $this->options[CURLOPT_URL] = $this->base_rest_url. '/facets?q=' . urlencode($query);
    //drupal_set_message('----> ' . $this->options[CURLOPT_URL]);
    return $this->execute();
  }

  private function execute() {
    $curl = $this->init_curl();
    $response = curl_exec($curl);
    if (!$response) {
      $error = curl_error($curl);
      return false;
    } else {
      return $response;
    }
  }
}


