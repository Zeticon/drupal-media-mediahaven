<?php
/**
 * @file
 * media_youtube/includes/MediaMediaHavenStreamWrapper.inc
 *
 * Create a MediaHaven Stream Wrapper class for the Media/Resource module.
 */

/**
 * Create an instance like this:
 * $youtube = new MediaMediaHavenStreamWrapper('mediahaven://f/[video-code]');
 */
class MediaMediaHavenStreamWrapper extends MediaReadOnlyStreamWrapper {

  /**
   * Returns a url in the format "TODO".
   *
   * Overrides interpolateUrl() defined in MediaReadOnlyStreamWrapper.
   * This is an exact copy of the function in MediaReadOnlyStreamWrapper,
   * here in case that example is redefined or removed.
   */
  public function interpolateUrl() {

    $this->base_url = media_mediahaven_variable_get('url');

    if ($parameters = $this->get_parameters()) {
      return $this->base_url . '/index.php/embed/image?img=' . $parameters['f'];
    }
    else {
      return FALSE;
    }
  }

  /**
   * Determine mime type for certain uri.
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return 'application/octet-stream';
  }

  /**
   * Overrides getTarget().
   */
  public function getTarget($f) {
    return FALSE;
  }

  /**
   * Function getOriginalThumbnailPath().
   * @return string
   *   The path to the thumbnail.
   */
  public function getOriginalThumbnailPath() {
    $parts = $this->get_parameters();
    $thumb_width = media_mediahaven_variable_get('thumb_width');
    $thumb_height = media_mediahaven_variable_get('thumb_height');
    return media_mediahaven_variable_get('url') . '/index.php/embed/image?img=' . $parts['f'] . '&w=' . $thumb_width . '&h=' . $thumb_height;
  }

  /**
   * Determine thumbnail path.
   * @return string
   *   Path to thumbnail.
   */
  public function getLocalThumbnailPath() {
    $parts = $this->get_parameters();

    $local_path = file_default_scheme() . '://todo/' . check_plain($parts['f']) . '.jpg';
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
