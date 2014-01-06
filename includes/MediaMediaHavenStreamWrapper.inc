<?php

/**
 *  @file media_youtube/includes/MediaMediaHavenStreamWrapper.inc
 *
 *  Create a MediaHaven Stream Wrapper class for the Media/Resource module.
 */

/**
 *  Create an instance like this:
 *  $youtube = new MediaMediaHavenStreamWrapper('mediahaven://f/[video-code]');
 */

class MediaMediaHavenStreamWrapper extends MediaReadOnlyStreamWrapper {



  /**
   * Returns a url in the format "TODO".
   *
   * Overrides interpolateUrl() defined in MediaReadOnlyStreamWrapper.
   * This is an exact copy of the function in MediaReadOnlyStreamWrapper,
   * here in case that example is redefined or removed.
   */
  function interpolateUrl() {
    $this->base_url =  media_mediahaven_variable_get('url');
	return  'http://10.10.3.140/beeldbanktuur/index.php/embed/image?img=b6f010a0ca1c4b9dbc29f27da0a206dc22d5b890a7d6405e9821d3c6923df17b9ea931e0e09f44e2824201c27efb9631';
	  if ($parameters = $this->get_parameters()) {
      return $this->base_url . 'index.php/embed/?' . http_build_query($parameters);
    }
  }

  static function getMimeType($uri, $mapping = NULL) {
    return 'application/octet-stream';//TODO What is this used for? do wee need to set specific mime types for the different object types?
  }

  function getTarget($f) {
    return FALSE;
  }

  function getOriginalThumbnailPath() {
    $parts = $this->get_parameters();
#    return 'http://todo/vi/' . check_plain($parts['v']) . '/0.jpg';
    return 'http://10.10.3.140/beeldbanktuur/index.php/embed/image?img=b6f010a0ca1c4b9dbc29f27da0a206dc22d5b890a7d6405e9821d3c6923df17b9ea931e0e09f44e2824201c27efb9631';
  }

  function getLocalThumbnailPath() {
    $parts = $this->get_parameters();
    $local_path = file_default_scheme() . '://todo/' . check_plain($parts['v']) . '.jpg';
    if (!file_exists($local_path)) {
      $dirname = drupal_dirname($local_path);
      file_prepare_directory($dirname, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      $response = drupal_http_request($this->getOriginalThumbnailPath());
      if (!isset($response->error)) {
        file_unmanaged_save_data($response->data, $local_path, TRUE);
      }
      else {
        @copy($this->getOriginalThumbnailPath(), $local_path);
      }
    }
    return $local_path;
  }
}

