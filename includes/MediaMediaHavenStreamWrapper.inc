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
 //include_once 'media_mediahaven.utils.inc';
 
class MediaMediaHavenStreamWrapper extends MediaReadOnlyStreamWrapper {

  /**
   * Returns a url in the format "TODO".
   *
   * Overrides interpolateUrl() defined in MediaReadOnlyStreamWrapper.
   * This is an exact copy of the function in MediaReadOnlyStreamWrapper,
   * here in case that example is redefined or removed.
   */
  public function interpolateUrl() {

    $this->base_url = variable_get('media_mediahaven_url','https://integration.mediahaven.com');

    if ($parameters = $this->get_parameters()) {
      return media_mediahaven_embed_url($parameters['f']);
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
    $rest = media_mediahaven_rest_client();
    $media_object = json_decode($rest->getMediaObject($parts['f']), TRUE);
    if ($media_object) {
      return $media_object['thumbnailImagePath'];
    }
    $thumb_width = variable_get('media_mediahaven_thumb_width',150);
    $thumb_height = variable_get('media_mediahaven_thumb_height',150);
    $url =  media_mediahaven_embed_url($parts['f']);
    $url.= '?w=' . $thumb_width . '&h=' . $thumb_height;
    return $url;
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
