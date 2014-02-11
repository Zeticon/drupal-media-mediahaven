<?php

/**
 * @file media_mediahaven/themes/media-mediahaven-video.tpl.php
 *
 * Template file for theme('media_mediahaven_video').
 *
 * Variables available:
 *  $mediahaven_url - The embed base_url for the mediahaven video (e.g., http://10.10.3.4).
 *  $umid - The unique identifier of the mediahaven video (e.g., 018d6b47c0d04cb698c1a06305db9cd16af4d92cabe14dc7b86a3e6c8bcce4ee160c7b1f69d24c7584b9d93a479ebd34).
 *  $video_width - The width value set in Media: mediahaven file display options.
 *  $video_height - The height value set in Media: mediahaven file display options.
 *
 */

?>
<div class="<?php print $classes; ?>">
  <iframe title="" src="<?= media_mediahaven_variable_get('url'); ?>/index.php/embed/<?php print $umid; ?>" frameborder="0" width="<?php print $video_width; ?>" height="<?php print $video_height; ?>"  allowfullscreen></iframe>
</div>
