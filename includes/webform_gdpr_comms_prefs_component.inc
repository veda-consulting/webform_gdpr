<?php
/**
 * @file wf_component_gdpr_comms_prefs.inc
 *
 * Defines a component for webform.
 */

/**
 * Implements _webform_edit_component
 **/
function _webform_edit_gdpr_comms_prefs($component) {
   // @todo: prevent multiple instances of this component on the same form.
   // Add a notice 
   civicrm_initialize();
   $notice = t('<p>This component will add elements on the webform, using the Communications Preferences settings on the CiviCRM GDPR extension.</p>
       </p><p> You can <a href="!url" target="blank">review the settings here</a>.</p>', array(
           '!url' => CRM_Utils_System::url('civicrm/gdpr/comms-prefs/settings')
       )
   );
   $form = array();
   
   $form['extra']['description'] = array(
       '#markup' => $notice, 
   );
   $form['extra']['use_channels'] = array(
     '#type' => 'checkbox',
     '#title' => t('Enable channels section in this form.'),
   );
   $form['extra']['use_groups'] = array(
     '#type' => 'checkbox',
     '#title' => t('Enable groups section in this form.'),
   );
   return $form;
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_gdpr_comms_prefs($component, $values, $format = 'html') {
  if (is_array($values)) {
    $values = $values[0];
  }
  $values = unserialize($values);
  return WebformGDPRCommsPrefs::displaySubmission($component, $values, $format);
}

/**
 * Implements _webform_table_component().
 */
function _webform_table_gdpr_comms_prefs($component, $values) {
  $output = _webform_display_gdpr_comms_prefs($component, $values);
  return $output['#value'];
}

/**
 * Implements _webform_submit_component().
 */
function _webform_submit_gdpr_comms_prefs($component, $values) {
  return serialize($values);
}

/**
 * Implements csv_headers_component().
 */
function _webform_csv_headers_gdpr_comms_prefs($component, $export_options) {
}

/**
 * Implements csv_data_component().
 */
function _webform_csv_data_gdpr_comms_prefs($component, $export_options, $data) {
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_gdpr_comms_prefs($component, $value = NULL, $filter = TRUE) {
  return WebformGDPRCommsPrefs::render($component, $value, $filter);
}
