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
   // Only allow one Comms Prefs on a webform.
   $node = node_load($component['nid']);
   $comms_prefs_components = webform_gdpr_get_components($node, 'gdpr_comms_prefs');
   $disable = FALSE;
   if ($comms_prefs_components) {
     foreach ($comms_prefs_components as $other_component) {
       if (empty($component['cid']) || $other_component['cid'] != $component['cid']) {
         $disable = TRUE;
       }
     }
   }
   // @todo: prevent multiple instances of this component on the same form.
   // Add a notice
   civicrm_initialize();
   if (!$disable) {
   $notice = t('<p>This component will add elements on the webform, using the Communications Preferences settings on the CiviCRM GDPR extension.</p>
       </p><p> You can <a href="!url" target="blank">review the settings here</a>.</p>', array(
           '!url' => CRM_Utils_System::url('civicrm/gdpr/comms-prefs/settings')
       )
   );
   }
   else {
     $notice = t('There can only be one Communications Preferences component per Webform and this form already has one. This component will not be active.');
   }
   $conf = $component['extra']['comms_prefs'];
   $element = array(
     '#type' => 'fieldset',
     '#title' => t('Communications Preferences'),
     '#description' => $notice,
   );

   if (!$disable) {
     $element['use_channels'] = array(
       '#type' => 'checkbox',
       '#title' => t('Enable Communication Preferences channel opt-in.'),
       '#value' => TRUE,
       '#attributes' => array('disabled' => 1),
     );

     $element['use_channels_options_fieldset'] = [
       '#type' => 'fieldset',
       '#title' => t('Channels opt-in'),
       '#id' => 'wf-crm-options-fieldset',
     ];
     $settings = CRM_Gdpr_CommunicationsPreferences_Utils::getSettings();
     $general_settings = $settings[CRM_Gdpr_CommunicationsPreferences_Utils::SETTING_NAME];

     foreach (CRM_Gdpr_CommunicationsPreferences_Utils::getChannelOptions() as $channel => $label) {
       if (empty($general_settings['channels']['enable_' . $channel])) {
         continue;
       }
       $defaultValue = 1;
       if (!empty($conf['use_channels_options_fieldset'])) {
         $defaultValue = $conf['use_channels_options_fieldset']['use_channels_options_' . $channel] ?? 0;
       }
       $element['use_channels_options_fieldset']['use_channels_options_' . $channel] = [
         '#type' => 'checkbox',
         '#title' => $label,
         '#default_value' => $defaultValue,
       ];
     }

     $element['use_channels_clear_defaultValue'] = array(
       '#type' => 'checkbox',
       '#title' => t('Do not pre-select channels opt-in.'),
       '#default_value' => $conf['use_channels_clear_defaultValue'] ?? 0,
     );

     $element['use_groups'] = array(
       '#type' => 'checkbox',
       '#title' => t('Enable groups subscription.'),
       '#default_value' => $conf['use_groups'],
     );
     $element['channel_element_type'] = array(
       '#type' => 'select',
       '#title' => t('Element type for channel opt-ins.'),
       '#options' => ['radios' => t('Use Yes/No Radios'), 'select' => t('Use a Select element')],
       '#default_value' => $conf['channel_element_type'],
     );
   }
   $form['extra']['comms_prefs'] = $element;
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
