<?php
/**
 * @file wf_component_gdpr.inc
 *
 * Defines a Terms and Conditions / Data Policy component for webform.
 */

/**
 * Implements _webform_edit_component
 **/
function _webform_edit_gdpr($component) {
  $defaults = !empty($component['extra']) ? $component['extra'] : array();
  $node = node_load($component['nid']);
  // Get contact numbers
  if (!empty($node->webform_civicrm['data']['contact'])) {
    foreach($node->webform_civicrm['data']['contact'] as $key => $contact_setting) {
      $contact_options[$key] = $contact_setting['contact'][1]['webform_label'];
    }
  }
  else {
    drupal_set_message(t('You do not yet have any CiviCRM contacts configured for this webform.'));
  }
  $tc_defaults = WebformGdpr::getDefaultUrls();
  $data_policy = $tc_defaults['data_policy'];
  $entity_tc = $tc_defaults['entity'];
  $default_label = t('I agree to the [data-policy-link] and the [event-terms-conditions-link]');

  $event_options = WebformGdpr::getTCEvents();
  $contribution_options = WebformGdpr::getTCContributionPages();
  // Add option for default terms and conditions for Event or Contribution.
  if ($entity_tc) {
    $label = t('Use default Terms & Conditions');
    $event_options[0] = $label;
    $contribution_options[0] = $label;
  }
  // Unset values if there are no options available.
  if (!$data_policy) {
    $defaults['enable_data_policy'] = '';
  }
  $tc_event_description = $tc_contribution_description = '';
  if (!$event_options) {
    $defaults['enable_tc_event'] = '';
    $tc_event_description = t('No Terms and Conditions available for Events.');
  }
  if (!$contribution_options) {
    $defaults['enable_tc_contribution'] = '';
    $tc_contribution_description = t('No Terms and Conditions available for Contributions.');
  }
  $form = array();
  $form['extra'] = array(
    'contact' => array(
      '#title' => t('Contact'),
      '#type' => 'select',
      '#options' => $contact_options,
      '#description' => t('The contact accepting the Policy or Terms. In most cases this will be the first contact.'),
      '#default_value' => isset($defaults['contact']) ? $defaults['contact'] : 1,
    ),
    'enable_data_policy' => array(
      '#title'  => t('Enable Data Policy'),
      '#type' => 'checkbox',
      '#description' => !empty($tc_defaults['data_policy']) ? t('Use the Data Policy from the CiviCRM GDPR settings.') : t('You do not have a Data Policy in the CiviCRM GDPR settings'),
      '#disabled' => empty($data_policy),

      '#default_value' => isset($defaults['enable_data_policy']) ? $defaults['enable_data_policy'] : '',
    ),
    'enable_tc_event' => array(
      '#title'  => t('Enable Event Terms & Conditions'),
      '#type' => 'checkbox',
      '#description' => $tc_event_description,
      '#default_value' => isset($defaults['enable_tc_event']) ? $defaults['enable_tc_event'] : '',
      '#disabled' => empty($event_options),
    ),
    'tc_event' => array(
      '#title' => t('Event'),
      '#type' => 'select',
      '#options' => $event_options,
      '#description' => t('The Event should have a Terms &amp; Conditions file or url set for it.'),
      '#default_value' => isset($defaults['tc_event']) ? $defaults['tc_event'] : 0,
      '#states' => array(
        'visible' => array(
          '#edit-extra-enable-tc-event' => array(
            'checked' => TRUE,
          ),
        ),
      ),
    ),
    'enable_tc_contribution' => array(
      '#title'  => t('Enable Contribution Terms & Conditions'),
      '#type' => 'checkbox',
      '#description' => $tc_contribution_description,
      '#default_value' => isset($defaults['enable_tc_contribution']) ? $defaults['enable_tc_contribution'] : '',
      '#disabled' => empty($contribution_options),
    ),
    'tc_contribution' => array(
      '#title' => t('Contribution Page'),
      '#type' => 'select',
      '#options' => $contribution_options,
      '#description' => t('The Contribution Page should have a Terms &amp; Conditions file or url set for it.'),
      '#default_value' => isset($defaults['tc_contribution']) ? $defaults['tc_contribution'] : 0,
      '#states' => array(
        'visible' => array(
          '#edit-extra-enable-tc-contribution' => array(
            'checked' => TRUE,
          ),
        ),
      ),
    ),
    'label' => array(
      '#title' => t('Checkbox Label'),
      '#type' => 'textarea',
      '#description' => t('The label for the acceptance checkbox. <br /> You can use: [data-policy-link], [event-terms-conditions-link], [contribution-terms-conditions-link] to insert the respective links into the label.'),
      '#default_value' => isset($defaults['label']) ? $defaults['label'] : $default_label,
    ),
  );
  return $form;
}

/**
 * Implements _webform_display_component().
 */
function _webform_display_gdpr($component, $values, $format = 'html') {
  return array(
    '#title' => $component['name'],
    '#weight' => $component['weight'],
    '#theme' => 'webform_display_textfield',
    '#theme_wrappers' => $format == 'html' ? array('webform_element') : array('webform_element_text'),
    '#post_render' => array('webform_element_wrapper'),
    '#component' => $component,
    '#format' => $format,
    '#value' => isset($value[0]) ? $value[0] : '',
  );
}

/**
 * Implements _webform_table_component().
 */
function _webform_table_gdpr($component, $values) {
  $output = _webform_display_gdpr($component, $values);
  return $output['#value'];
}

/**
 * Implements _webform_submit_component().
 */
function _webform_submit_gdpr($component, $values) {
  return $values;
}

/**
 * Implements csv_headers_component().
 */
function _webform_csv_headers_gdpr($component, $export_options) {
}

/**
 * Implements csv_data_component().
 */
function _webform_csv_data_gdpr($component, $export_options, $data) {
}

/**
 * Implements _webform_render_component().
 */
function _webform_render_gdpr($component, $value = NULL, $filter = TRUE) {
  $settings = $component['extra'];
  $search = array(
    '[data-policy-link]',
    '[event-terms-conditions-link]',
    '[contribution-terms-conditions-link]',
  );
  $replace = array(
    $settings['enable_data_policy'] ? WebformGdpr::getDataPolicyLink() : '',
    $settings['enable_tc_event'] ? WebformGdpr::getEntityLink('Event', $settings['tc_event']) : '',
    $settings['enable_tc_contribution'] ? WebformGdpr::getEntityLink('ContributionPage', $settings['tc_contribution']) : '',
  );
  $label = str_replace($search, $replace, $settings['label']);
  $label = filter_xss($label);
  $node = isset($component['nid']) ? node_load($component['nid']) : NULL;
  $element = array(
    '#type' => 'checkbox',
    '#title' => $label,
    '#weight' => $component['weight'],
    '#description' => $filter ? webform_filter_descriptions($component['extra']['description'], $node) : $component['extra']['description'],
    '#default_value' => $value ? 'checked' : NULL,
    '#return_value' => 1,
    '#required' => $component['required'],
    '#theme_wrappers' => array('webform_element'),
  );
  if (!empty($component['#required'])) {
    $element['#attributes']['required'] = 'required';
  }
  return $element;
}
