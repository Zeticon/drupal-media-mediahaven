<?php

/**
 * @file
 * A Rest Client class for interaction with the MediaHaven REST-API
 */

class MediaHavenRestClient {

  public $baseRestUrl;
  protected $options = array();
  protected $error = '';
  const SUPPORTED_MEDIATYPES_QUERY = '+(MediaObjectType:Video OR MediaObjectType:Image)';

  /**
   * Constructor.
   */
  public function __construct($baseRestUrl, $login, $password) {
    $this->baseRestUrl = $baseRestUrl;
    $this->options = array(
      CURLOPT_HTTPAUTH => CURLAUTH_BASIC,
      CURLOPT_USERPWD => $login . ":" . $password,
      CURLOPT_RETURNTRANSFER => 1,
      CURLOPT_COOKIEJAR => "/tmp/cookie.txt",
      CURLOPT_SSL_VERIFYPEER => FALSE,
      CURLOPT_SSL_VERIFYHOST => FALSE,
      CURLOPT_SSLVERSION => 3,
    );
  }

  /**
   * Retrieve property.
   */
  public function __get($property) {
    if (property_exists($this, $property)) {
      return $this->$property;
    }
  }

  /**
   * Initialize curl with the provided options.
   */
  protected function initCurl() {
    $curl = curl_init();
    foreach ($this->options as $option => $value) {
      curl_setopt($curl, $option, $value);
    }
    return $curl;
  }

  /**
   * Search for a specific term.
   */
  public function search($query, $start_index, $nr_of_results) {
    if ($query != '') {
      $query = '+(' . $query . ') ';
    }
    if (strpos($query, self::SUPPORTED_MEDIATYPES_QUERY) == FALSE) {
      $query .= self::SUPPORTED_MEDIATYPES_QUERY;
    }
    $query = urlencode($query);

    $url = $this->baseRestUrl . '/media?q=' . $query;
    if (isset($start_index) && strlen($start_index) > 0) {
      $url .= '&startIndex=' . $start_index;
    }
    if (isset($nr_of_results) && strlen($nr_of_results)) {
      $url .= '&nrOfResults=' . $nr_of_results;
    }

    $this->options[CURLOPT_URL] = $url;
    return $this->execute();
  }

  /**
   * Retrieve a mediaobject based on an id.
   */
  public function getMediaObject($id) {
    $this->options[CURLOPT_URL] = $this->baseRestUrl . '/media/' . $id;
    //var_dump($this->options);
    return $this->execute();
  }

  /**
   * Load the facets with a certain query.
   */
  public function getFacets($query) {
    if (empty($query)) {
      $query = '*';
    }

    if (strpos($query, self::SUPPORTED_MEDIATYPES_QUERY) == FALSE) {
      $query .= self::SUPPORTED_MEDIATYPES_QUERY;
    }
    $this->options[CURLOPT_URL] = $this->baseRestUrl . '/facets?q=' . urlencode($query);
    return $this->execute();
  }

  /**
   * Upload media file to webservice.
   */
  public function upload($media_file, $metadata_file, $ingestspace_id) {
    if (media_mediahaven_variable_get('autopublish') == 1) {
      $autopublish = 'true';
    }
    else {
      $autopublish = 'false';
    }

    $upload_settings = array(
      'file' => '@' . $media_file . ';filename=' . substr($media_file, strrpos($media_file, '/') + 1),
      'metadata' => '@' . $metadata_file,
      'ingestSpaceId' => $ingestspace_id,
      'autoPublish' => $autopublish,
    );

    $this->options[CURLOPT_URL] = $this->baseRestUrl . '/media';
    $this->options[CURLOPT_POST] = 1;
    $this->options[CURLOPT_POSTFIELDS] = $upload_settings;

    return $this->execute();
  }

  /**
   * Execute the commands to the webservice.
   * @return boolean|mixed
   *   False when incorrect response from webservice.
   *   The response when correct response from webservice.
   */
  protected function execute() {
    $curl = $this->initCurl();
    $response = curl_exec($curl);
    $http_code = curl_getinfo($curl, CURLINFO_HTTP_CODE);
    $this->error = curl_error($curl);
    curl_close($curl);
    if (!$response) {
      return FALSE;
    }
    elseif ($http_code != 200) {
      $this->error = $response;
      return FALSE;
    }
    else {
      return $response;
    }
  }
}
