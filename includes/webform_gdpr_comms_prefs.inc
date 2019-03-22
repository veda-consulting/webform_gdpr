<?php
/**
 * @file
 *  Class definition file.
 */
use CRM_Gdpr_CommunicationsPreferences_Utils as U;

/** 
 * Helper class for the Communications Preferences component.
 */
class WebformGDPRCommsPrefs {
 
  /**
   * Render webform component.
   * 
   * @param array $component
   * @param array $value
   * @param boolean $filter
   * @return array
   */
  public static function render($component, $value = NULL, $filter = TRUE) {
    
    $component_settings = $component['extra']['comms_prefs'];
    $use_channels = !empty($component_settings['use_channels']);
    $use_groups = !empty($component_settings['use_groups']);
    $settings = U::getSettings();
    $general_settings = $settings[U::SETTING_NAME];
    
    $group_settings = $settings[U::GROUP_SETTING_NAME];
    uasort($group_settings, function($a, $b) { 
      return $a['group_weight'] - $b['group_weight'];
    });
    $elem = array(
      '#weight' => $component['weight'],
      '#element_validate' => ['webform_gdpr_validate_comms_prefs'],
    );
    
    if ($use_channels) {
      $channel_elems = array(
        '#type' => 'fieldset',
        '#title' => $general_settings['page_title'],
        '#description' => $general_settings['channels_intro'],
        '#attributes' => ['class' => ['gdpr-comms-prefs-channels-wrapper']],
      );
      $contact = self::getLoggedInContact();
      
      foreach (U::getChannelOptions() as $channel => $label) {
        if (empty($general_settings['channels']['enable_' . $channel])) {
          continue;
        }
        $elem_type = !empty($component_settings['channel_element_type']) ? $component_settings['channel_element_type'] : 'radios';
        $options = array('YES' => t('Yes'), 'NO' => t('No'));
        $channel_elems[$channel] = array(
          '#type' => $elem_type,
          '#title' => $label, 
          // Option values compatible with the form elements provided by the extension.
          '#options' => $options, 
          '#required' => TRUE,
          '#prefix' => '<div class="webform-container-inline">',
          '#suffix' => '</div>',
          '#attributes' => ['data-channel' => $channel],
        ); 
        // Set default values based on contact properties.
        // We can only really determine the negatives.
        $is_no = !empty($contact['do_not_' . $channel]);
        $is_opt_out = $channel == 'email' && !empty($contact['is_opt_out']);
        $is_no_post = $channel == 'post' && !empty($contact['do_not_mail']);
        if ($is_no || $is_opt_out || $is_no_post) {
          $channel_elems[$channel]['#default_value'] = 'NO'; 
        }
      }
      $elem['channels'] = $channel_elems;
    }
  
    if ($use_groups) {
      $group_elems = array(
        '#type' => 'fieldset',
        '#title' => $general_settings['groups_heading'],
        '#description' => $general_settings['groups_intro'], 
        '#attributes' => ['class' => ['gdpr-comms-prefs-groups-wrapper']],
      );
      // Convenience function to extract channels.
      $group_channels = function($group) {
        $out = '';
        $enabled = [];
        foreach (U::getChannelOptions() as $channel => $label) {
          if (!empty($group[$channel])) {
            $enabled[] = $channel;
          }
        }
        return $enabled;
      };
      $current_groups = self::getContactGroups();
      foreach ($group_settings as $key => $group_setting) {
        if (empty($group_setting['group_enable'])) {
          continue;
        }
        $description = $group_setting['group_description']; 
        // @todo: provide user settings for group-channel display.
        // Channels are available as a property of the element, as a html data property and in the description.
        $grp_chans = $group_channels($group_setting);
        $gid = str_replace('group_', '', $key);
        $attr = [];
        $attr['data-channels'] = json_encode($grp_chans);
        $is_current = !empty($current_groups[$gid]);
        $description .= ' (<span class="group-channels">' . implode(', ', $grp_chans) . '</span>)';
        $group_chk = array(
          '#type' => 'checkbox',
          '#title' => $group_setting['group_title'],
          '#description' => $description,
          '#attributes' => $attr,
          '#channels' => $grp_chans,
          '#default_value' =>  $is_current,
        );
        $group_elems[$key] = $group_chk;
      }
      $elem['groups'] = $group_elems;
    }
    return $elem;
  }
 
  /**
   * Display a webform submission value for this component.
   * 
   * @param array $component
   * @param array $values
   * @param string $format
   * @return array
   */
  public static function displaySubmission($component, $values, $format = 'html') {
    $settings = U::getSettings();
    $general_settings = $settings[U::SETTING_NAME];
    $group_settings = $settings[U::GROUP_SETTING_NAME];
    $channel_label = $general_settings['page_title'];
    $nl = $format = 'html' ? '<br />' : "\n";
    foreach ($values['channels'] as $channel => $value) {
      $channel_str .= $channel . ' : ' . $value . $nl;
    }
    $elem['channels'] = array(
      '#type' => 'fieldset',
      '#title' => $channel_label,
      'values' => array(
        '#markup' => $channel_str,
      ),
    );
    $grp_str = '';
    foreach ($values['groups'] as $key => $value) {
      $grp_str .= $group_settings[$key]['group_title'] . ': ' . $value . $nl; 
    }
    $elem['groups'] = array(
      '#type' => 'fieldset',
      '#title' => $general_settings['groups_heading'],
      'values' => array(
        '#markup' => $grp_str,
      ),
    );
    return $elem;
  }
  
  /**
   * Gets a contact's group memberships.
   * 
   * @param int $contact_id
   * @return boolean[]
   *   Array keyed by group id.
   *   False elements denote removed memberships, true denote active memberships.
   */
  public static function getContactGroups($contact_id =  NULL) {
    $return = [];
    if (!$contact_id) {
      $contact_id = CRM_Core_Session::getLoggedInContactID();
    }
    if (!$contact_id) {
      return $return;
    }
    $results = self::crm('GroupContact', 'get', [
      'contact_id' => $contact_id,
      'sequential' => FALSE,      
    ]);
    foreach ($results['values'] as $contact_group) {
      $return[$contact_group['group_id']] = empty($contact_group['status']) || $contact_group['status'] != 'Removed';
    }
    return $return;
  }
  
  /**
   * Get contact of the current user.
   * 
   * @return []
   */
  public static function getLoggedInContact() {
    $cid = CRM_Core_Session::getLoggedInContactID();
    $return = [];
    if ($cid) {
      $result = self::crm('Contact', 'get', ['id' => $cid, 'sequential' => 0]);
      if (!empty($result['values'][$cid])) {
        $return = $result['values'][$cid];
      }
    }
    return $return;
  }
  
  /**
   * Process submission of a Comms Prefs component.
   * 
   * This is called after values have been saved to webform, so we
   * are able to get entities from webform_civicrm.
   * 
   * @param array $component
   * @param stdClass $submission
   */
  public static function processCommsPrefs($component, $values, $contact_id) {
    if (!$contact_id) {
      // Unable to process.
      return false;
    }
    $component_settings = $component['extra']['comms_prefs'];
    $use_channels = !empty($component_settings['use_channels']);
    $use_groups = !empty($component_settings['use_groups']);
    $settings = U::getSettings();
    $channel_settings = $settings[U::SETTING_NAME];
    
    $contact_params = array('id' => $contact_id);
    if ($use_channels) {
      //Update contact communication preference based on channels selected
      $mapper = U::getCommunicationPreferenceMapper();
      foreach ($values['channels'] as $channel => $mapper_key) {
        $param = $mapper[$channel][$mapper_key];
        if (is_array($param)) {
          $contact_params = array_merge($contact_params, $param);
        }
      }
      // Update contact.
      self::crm('Contact', 'create', $contact_params);
      // Create activity.
      U::createCommsPrefActivity($contact_id);
    }

    // Process groups.
    if ($use_groups) {
      // Values from form.
      $group_selections = $values['groups'];
      // User current groups.
      $current_groups = self::getContactGroups($contact_id); 
      // Groups enabled in Comms Prefs settings.
      $group_settings  = $settings[U::GROUP_SETTING_NAME];
      foreach ($group_settings as $group_key => $group_setting) {
        $gid = str_replace('group_', '', $group_key);
        // If this group is not enabled in settings then do nothing.
        if (empty($group_setting['group_enable']) || !$gid) {
          continue;
        }
        $params = [
          'contact_id' => $contact_id,
          'group_id' => $gid,
        ];
        // Has user opted in to this group?
        $is_selected = !empty($group_selections[$group_key]);
        // Does user already belong in the group?
        $is_current = !empty($current_groups[$gid]);
        
        // Do we need to do anything?
        if ($is_current && !$is_selected) {
          $params['status'] = 'Removed';
        }
        elseif (!$is_current && $is_selected) {
          $params['status'] = 'Added';
        }
        if (!empty($params['status'])) {
          self::crm('GroupContact', 'create', $params);
        }
      }
    }
  }
 
  /**
   * Wrapper for the CiviCRM API.
   * 
   * @param string $entity
   * @param string $method
   * @param array $params
   * @return array
   */
  protected static function crm($entity, $method, $params) {
    try {
      $result = civicrm_api3($entity, $method, $params);
      return $result;
    }
    catch (Exception $e) {
      watchdog('webform_gdpr', 
        t('Error calling CiviCRM API: @message', [
          '@message' => $e->getMessage()
        ]
      ), WATCHDOG_ERROR);
    }
  }
}
