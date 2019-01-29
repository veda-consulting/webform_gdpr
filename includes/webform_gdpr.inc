<?php
/**
 * Helper class for Webform GDPR.
 */
class WebformGdpr {
  /**
   * @var array
   *
   * Contribution pages that have their own Terms & Conditions settings.
   */
  private static $tcContributionPages = array();


  /**
   * @var array
   *
   * Events that have their own Terms & Conditions settings.
   */
  private static $tcEvents = array();

  /**
   * Return a data policy link.
   *
   * @return string
   */
  public static function getDataPolicyLink() {
    $default_urls = self::getDefaultUrls();
    $url = $default_urls['data_policy'];
    $label = CRM_Gdpr_SLA_Utils::getLinkLabel();
    $options['attributes']['target'] = 'blank';
    $options['attributes']['class'][] = 'data-policy-link';
    return l($label, $url, $options);
  }

  /**
   * Return a Data Policy link for a CiviCRM entity.
   *
   * @param string $type
   *  CiviCRM entity type: either 'Event' or 'ContributionPage'.
   * @param int $id
   *  Entity Id.
   *
   * @return string
   *  Rendered html link.
   */
  public static function getEntityLink($type, $id) {
    if (!$id) {
      $default_urls = self::getDefaultUrls();
      $url = $default_urls['entity'];
      $label = t('Data Policy');
    }
    elseif ($type == 'Event') {
      $helper = new CRM_Gdpr_SLA_Event($id);
    }
    elseif ($type = 'ContributionPage') {
      $helper = new CRM_Gdpr_SLA_ContributionPage($id);
    }
    if (!empty($helper)) {
      $url = $helper->getUrl();
      $label = $helper->getValue('Link_Label');
      $label = $label ? $label : t('Terms & Conditions');
    }
    $options['attributes']['target'] = 'blank';
    $options['attributes']['class'][] = 'terms-conditions-link';
    return l($label, $url, $options);
  }

  public static function recordAcceptance($contact_id, $type = NULL, $entity_id = NULL) {
    if (!$contact_id) {
      return;
    }
    if ($type == 'Event') {
      $helper = new CRM_Gdpr_SLA_Event($entity_id);
      return $helper->recordAcceptance($contact_id);
    }
    if ($type == 'ContributionPage') {
      $helper = new CRM_Gdpr_SLA_ContributionPage($entity_id);
      return $helper->recordAcceptance($contact_id);
    }
    return CRM_Gdpr_SLA_Utils::recordSLAAcceptance($contact_id);
  }

  /**
   * Gets a list of Contribution pages that have their own Terms & Conditions settings.
   *
   * @return array
   *  Contribution Page titles, keyed by id.
   */
  public static function getTCContributionPages() {
    if (!self::$tcContributionPages) {
      self::$tcContributionPages = self::fetchTCEntities('ContributionPage');
    }
    return self::$tcContributionPages;
  }

  /**
   * Gets a list of Events that have their own Terms & Conditions settings.
   *
   * @return array
   *  Event titles, keyed by id.
   */
  public static function getTCEvents() {
    if (!self::$tcEvents) {
      self::$tcEvents = self::fetchTCEntities('Event');
    }
    return self::$tcEvents;
  }

  private static function fetchTCEntities($type, $is_active_only = TRUE) {
    $return = array();
    if ($type == 'Event') {
      $entity_table = 'civicrm_event';
    }
    elseif ( $type == 'ContributionPage') {
      $entity_table = 'civicrm_contribution_page';
    }
    else {
      return $return;
    }
    $field_data = self::getTCFieldData($type);
    $value_table = $field_data['table_name'];
    $filter_column = $field_data['fields']['Terms_and_Conditions_File']['column_name'];
    if (!$value_table || !$filter_column) {
      return $return;
    }
    if (!class_exists('CRM_Core_DAO')) {
      civicrm_initialize();
    }
    // We want a list of Events/Contribution pages that have certain custom values set.
    // We resort to a direct query, since the api is not geared for this.
    $sql = "
      SELECT e.id id, e.title title, e.is_active is_active
      FROM $value_table tc
        INNER JOIN $entity_table e
        ON tc.entity_id = e.id
        AND  tc.$filter_column != ''
    ";
    if ($is_active_only) {
      $sql .= "
        WHERE e.is_active = 1;
      ";
    }
    $dao = CRM_Core_DAO::executeQuery($sql);
    while ($dao->fetch()) {
      $return[$dao->id] = check_plain($dao->title) . ' (' . $dao->id. ')';
    }
    return $return;
  }

  /**
   * Gets the default URLS for Data Policy and Terms Conditions.
   */
  public static function getDefaultUrls() {
    static $return = array();
    if (!$return) {
      $return['data_policy'] = CRM_Gdpr_SLA_Utils::getTermsConditionsUrl();
      $settings = CRM_Gdpr_Utils::getGDPRSettings();
      // URL may be for an uploaded file, or a path to any page.
      $use_link = !empty($settings['sla_data_policy_option']) && !empty($settings['entity_tc_link']) && $settings['sla_data_policy_option'] == 2;
      if ($use_link) {
        $return['entity'] = $settings['entity_tc_link'];
      }
      else {
        $return['entity'] = $settings['entity_tc'];
      }
    }
    return $return;
  }

  private static function normalizeUrl($url) {
    if (!$url) {
      return;
    }
    if (0 == strpos($url, '/')) {
      $url = substr($url, 1);
    }
    return CRM_Utils_System::url($url);
  }


  /**
   * Gets custom field data for Entity-specific Terms & Conditions settings.
   *
   * @param string $type
   *  'Event'|'ContributionPage'
   *
   *  @return string
   *   Name of table with custom values.
   */
  private static function getTCFieldData($type) {
    static $fieldData = array();
    if (empty($fieldData[$type])) {
      $group_name = '';
      if ($type == 'Event') {
        $group_name = 'Event_terms_and_conditions';
      }
      elseif ($type == 'ContributionPage') {
        $group_name = 'Contribution_Page_terms_and_conditions';
      }
      if ($group_name) {
        $result = civicrm_api3('CustomGroup', 'get', array(
          'sequential' => 1,
          'name' => $group_name,
          'api.customField.get' => array(),
        ));
        if (!empty($result['values'])) {
          $fieldData[$type]['table_name'] = $result['values'][0]['table_name'];
          foreach ($result['values'][0]['api.customField.get']['values'] as $field) {
            $fields[$field['name']] = $field;
          }
          $fieldData[$type]['fields'] = $fields;
        }
      }
    }
    return $fieldData[$type];
  }
}
