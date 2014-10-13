/**
 * @file
 * Handles the JS for the views file browser. Note that this does not currently
 * support multiple file selection
 */


//(function ($) {
//
//  Drupal.behaviors.MediaMediaHavenBrowser = {
//    attach: function (context, settings) {
//
//      // Container for the files that get passed back to the browser
//      var files = {};
//
//      // Disable the links on media items list
//      $('ul#media-browser-library-list a').click(function() {
//        return false;
//      });
//
//      // Catch the click on a media item
//      $('.result_list li a').bind('click', function (e) {
//		e.preventDefault();
//		var uuid = $(this).attr('rel');
//		var uri = 'mediahaven://f/' + uuid;
//		var file = Drupal.settings.media.files[uri];
//        var files = new Array();
//        files.push(file);
//        Drupal.media.browser.selectMedia(files);
//        $("input[name='submitted-video']").val(uri);
//      });
//
////      $('.')
//    }
//  }
//
//}(jQuery));

(function ($) {

  Drupal.behaviors.mediaYouTubeBrowser = {
    attach: function (context, settings) {

      // Container for the files that get passed back to the browser
      var files = {};

      // Disable the links on media items list
      $('ul#media-browser-library-list a').click(function() {
        return false;
      });

      // Catch the click on a media item
      $('#media-mediahaven-browser-add .media-item').bind('click', function () {
        // Remove all currently selected files
        $('.media-item').removeClass('selected');
        // Set the current item to active
        $(this).addClass('selected');
        // Add this FID to the array of selected files
        var uri = $(this).parent('a[data-uri]').attr('data-uri');

        // Get the file from the settings which was stored in
        // template_preprocess_media_views_view_media_browser()
        var file = Drupal.settings.media.files[uri];
        //alert(JSON.stringify(file, null, 4));
        var files = new Array();
        files.push(file);
        Drupal.media.browser.selectMedia(files);
        $("input[name='submitted-media']").val(uri);
      });

//      $('.')

        $('.facetlink').each(function(i){
            $(this).click(function(e){
                e.preventDefault();
                var data = $(this).attr('data');
                var href = $(this).attr('href');

                //Fix the data
                var current_data = $('#edit-facet-selected').val();

                if (current_data == '') {
                  $('#edit-facet-selected').val(data);
                } else {
                  $('#edit-facet-selected').val($('#edit-facet-selected').val() + "\n" + data);
                }

                //Fix the href
                href = href.replace('/?q=', '');
                var current_search = $('#edit-facet-search').val();

                if (current_search == '') {
                  $('#edit-facet-search').val(href);
                } else {
                  $('#edit-facet-search').val($('#edit-facet-search').val() + "\n" + href);
                }

                $('#media-mediahaven-browser-add').submit();
            })
        });

        $('.facetdeletelink').each(function(i){
          $(this).click(function(e){
            //Remove tag
            e.preventDefault();
            var data = $(this).attr('data');
            var selected = $('#edit-facet-selected').val();
            selected = selected.split("\n");
            selected.splice(data, 1);

            if (selected.length != 1) {
              selected = selected.join("\n");
            } else {
              selected = selected.join("");
            }
            $('#edit-facet-selected').val(selected);

            //Remove atoms
            var atoms = $('#edit-facet-search').val();
            atoms = atoms.split("\n");
            atoms.splice(data, 1);

            if (atoms.length != 1) {
              atoms = atoms.join("\n");
            } else {
              atoms = atoms.join("");
            }

            $('#edit-facet-search').val(atoms);

            $('#media-mediahaven-browser-add').submit();
          });
        });
    }
  }

}(jQuery));
