<?php
require_once 'media_mediahaven.admin.inc';
/**
 * @file media_mediahaven/includes/MediaMediaHavenBrowser.inc
 *
 * Definition of MediaMediaHavenBrowser.
 */

/**
 * Media browser plugin for displaying a specific view and display.
 */
class MediaMediaHavenBrowser extends MediaBrowserPlugin {
  /**
   * Implements MediaBrowserPluginInterface::access().
   */
  public function access($account = NULL) {
    // @TODO: media_access() is a wrapper for file_entity_access(). Switch to the
    // new function when Media 1.x is deprecated.
    return media_access('create', $account);
  }

  /**
   * Implements MediaBrowserPlugin::view().
   */
  public function view() {

    // Check if the user is able to add remote media.
   // if (user_access('add media from remote sources')) {
      $build['form'] = drupal_get_form('media_mediahaven_add', $this->params['types'], $this->params['multiselect']);
      return $build;
  //  }
  }
}
