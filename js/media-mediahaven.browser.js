/**
 * @file
 * Handles the JS for the views file browser. Note that this does not currently
 * support multiple file selection
 */


(function ($) {

  Drupal.behaviors.MediaMediaHavenBrowser = {
    attach: function (context, settings) {

      // Container for the files that get passed back to the browser
      var files = {};

      // Disable the links on media items list
      $('ul#media-browser-library-list a').click(function() {
        return false;
      });

      // Catch the click on a media item
      $('.result_list li a').bind('click', function (e) {
		e.preventDefault();
		
		var uuid = $(this).attr('rel');
		var uri = 'mediahaven://f/' + uuid;
		var file = Drupal.settings.media.files[uri];
		
        var files = new Array();
        files.push(file);
        Drupal.media.browser.selectMedia(files);
        $("input[name='submitted-video']").val(uri);
      });

//      $('.')
    }
  }

}(jQuery));