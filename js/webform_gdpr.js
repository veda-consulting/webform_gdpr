(function ($) {
	function processCommsPrefs(context) {
		var cpWrapper = $('.gdpr-comms-prefs-channels-wrapper', context),
		  groupsWrapper = $('.gdpr-comms-prefs-groups-wrapper'),
		  channelElemPrefix = 'edit-submitted-communications-preferences-channels-',
		  groupsChk = groupsWrapper.find('input[type=checkbox]');
		if (cpWrapper.length !== 1 || groupsWrapper.length !== 1) {
			return;
		}
		function getOptOutChannels() {
		  var channelSelects = cpWrapper.find("input:checked,select");
		  var optOuts = [];
		  channelSelects.each(function() {
			  var channel = $(this).data('channel');
			  if ($(this).val() == 'NO') {
				optOuts.push($(this).data('channel'));  
			  }
		  });
		  return optOuts;
		}
		function processOptOutGroups() {
		   var optOuts = getOptOutChannels();
		   groupsChk.each(function() {
			  // $('.cp-group-status').remove();
			  var channels = $(this).data('channels');
			  var remaining = $(channels).not(optOuts).get();

			  if (channels && remaining.length == 0) {
				  if ($(this).val()) {
			      //  $(this).after('<span class="cp-group-status description">Removed from this group because there are no communication channels selected.</span>');
		          }
				  $(this).attr('checked', false);
				  $(this).attr('disabled', 1);
			  }
			  else {
				  $(this).attr('disabled', false);
			  }
		   });	
		}
		cpWrapper.find("input,select").change(function(e) {
			processOptOutGroups();
		}).trigger('change');
	}
    Drupal.behaviors.webform_gdpr = Drupal.behaviors.webform_gdpr || {};
	Drupal.behaviors.webform_gdpr.attach = function(context) {
		processCommsPrefs(context);
	};  	
})(jQuery);
