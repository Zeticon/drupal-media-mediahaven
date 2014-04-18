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
   * constructor
   * @param unknown $baseRestUrl
   * @param unknown $login
   * @param unknown $password
   */
  function __construct($baseRestUrl, $login, $password) {
    $this->baseRestUrl = $baseRestUrl;
    $this->options = array(
        CURLOPT_HTTPAUTH => CURLAUTH_BASIC,
        CURLOPT_USERPWD => $login . ":" . $password,
        CURLOPT_RETURNTRANSFER => 1,
        CURLOPT_COOKIEJAR => "/tmp/cookie.txt",
        CURLOPT_SSL_VERIFYPEER => FALSE,
        CURLOPT_SSL_VERIFYHOST => FALSE,
        CURLOPT_SSLVERSION => 3
    );

  }

  /**
   *
   * @param unknown $property
   */
  public function __get($property) {
    if (property_exists($this, $property)) {
      return $this->$property;
    }
  }

  /**
   *
   * @return resource
   */
  private function initCurl() {
    $curl = curl_init();
    foreach ($this->options as $option => $value) {
      curl_setopt($curl, $option, $value);
    }
    return $curl;
  }

  /**
   * Search for a specific term
   * @param unknown $query
   * @param unknown $startIndex
   * @param unknown $nr_of_results
   * @return Ambigous <boolean, mixed>
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
   * retrieve a mediaobject based on an id
   * @param unknown $id
   * @return Ambigous <boolean, mixed>
   */
  public function getMediaObject($id) {
    $this->options[CURLOPT_URL] = $this->baseRestUrl . '/media/' . $id;
    return $this->execute();
  }

  /**
   * load the facets with a certain query
   * @param unknown $query
   * @return Ambigous <boolean, mixed>
   */
  public function getFacets($query) {
    if (strpos($query, self::SUPPORTED_MEDIATYPES_QUERY) == FALSE) {
      $query .= self::SUPPORTED_MEDIATYPES_QUERY;
    }
    $this->options[CURLOPT_URL] = $this->baseRestUrl . '/facets?q=' . urlencode($query);
    return $this->execute();
  }

  /**
   * upload media file to webservice
   * @param unknown $media_file
   * @param unknown $metadata_file
   * @param unknown $ingestspace_id
   * @return Ambigous <boolean, mixed>
   */
  public function upload($media_file, $metadata_file, $ingestspace_id) {
    if (media_mediahaven_variable_get('autopublish') == 1) {
      $autopublish = 'true';
    }
    else {
      $autopublish = 'false';
    }

    $upload_settings = array(
        'file' => '@' . $media_file . ';filename=' . substr($media_file, strrpos($media_file, '/')+1),
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
   *
   * @return boolean|mixed
   */
  private function execute() {
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
    else{
      return $response;
    }
  }
}


