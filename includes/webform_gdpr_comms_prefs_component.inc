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
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_gdpr_comms_prefs($component, $values, $format = 'html') {
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
  return $values;
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
  $element = [];
  return $element;
}
