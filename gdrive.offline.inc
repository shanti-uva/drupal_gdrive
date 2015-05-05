<?php
/**
 * Get user permission to access Gdrive from desktop app, localhost install
 */
 
 /**
  * Checks to see if the site is a localhost/offline site
  * 
  * Returns TRUE if offline
  */
 function gdrive_is_offline() {
 	$offline = (variable_get('gdrive_settings_localhost', 0) == TRUE) ? TRUE : FALSE;
	return $offline;
 }
 
 /**
  * Called from gdrive_auth_page_validate()
  * For registering offline site with Gdrive
  */
function gdrive_offline_validate() {
	$gsettings = gdrive_get_google_auth_settings();
	$url = "https://accounts.google.com/o/oauth2/auth";
	$data = array(
	  'scope' => 'profile',
	  'redirect_uri' => 'http://localhost/shiva',
	  'response_type' => 'code',
	  'client_id' => $gsettings['client'],
	);
	
	$options = array(
	  'method' => 'POST',
	  'data' => drupal_http_build_query($data),
	  'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
	);
	
	$result = drupal_http_request($url, $options);
	drupal_goto($result->redirect_url);
}

/**
 * Called from gdrive_register() with info['code'] and info['state']
 */
function dgrive_offline_register($info) {
	$gsettings = gdrive_get_google_auth_settings();
	$url = "https://www.googleapis.com/oauth2/v3/token";
	$data = array(
	  'code' => $info['gcode'],
	  'client_id' => $gsettings['client'],
	  'client_secret' => $gsettings['secret'],
	  'redirect_uri' => 'http://localhost/shiva',
	  'grant_type' => 'authorization_code'
	);
	
	$options = array(
	  'method' => 'POST',
	  'data' => drupal_http_build_query($data),
	  'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
	);
	
	$result = drupal_http_request($url, $options);
	variable_set('gdrive_local_token', $result->data);
}

function gdrive_offline_authorized() {
	$token = variable_get('gdrive_local_token', FALSE);
	$auth = (gdrive_is_offline() && !empty($token)) ? TRUE : FALSE;
	return $auth;
}
