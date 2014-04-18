<?php
/**
 * @file
 * media_mediahaven/includes/MediaMediaHavenBrowser.inc
 *
 * Definition of MediaMediaHavenBrowser.
 */

require_once 'media_mediahaven.admin.inc';
/**
 * Media browser plugin for displaying a specific view and display.
 */
class MediaMediaHavenBrowser extends MediaBrowserPlugin {

  /**
   * Implements MediaBrowserPluginInterface::access().
   */
  public function access($account = NULL) {
    return media_access('create', $account);
  }

  /**
   * Implements MediaBrowserPlugin::view().
   */
  public function view() {
    $build['form'] = drupal_get_form('media_mediahaven_browser_add',
        $this->params['types'], $this->params['multiselect']);
    return $build;
  }
}
