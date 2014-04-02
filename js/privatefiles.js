/**
 * Created by alexboyce on 4/1/14.
 */
(function($) {
  Drupal.behaviors.privatefiles = {
    attach: function(context, settings) {
      $(".pf-hash-clipboard").click(function(event) {
        event.preventDefault();
        var text = $("a:not('.pf-hash-clipboard')", $(this).parent().parent()).first().text();
        var url = location.protocol + '//' + location.hostname + '/private/file/' + text;
        if (!!window.clipboardData) {
          window.clipboardData.setData("Text", url);
        }
        else {
          window.prompt("Press Ctrl + C to copy the path to the clipboard", url);
        }
      });
    }
  };
})(jQuery);