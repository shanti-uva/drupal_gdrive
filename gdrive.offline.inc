<?php
/**
 * Get user permission to access Gdrive from desktop app, localhost install
 * 
 * See https://developers.google.com/identity/protocols/OAuth2InstalledApp for how this all works
 */
 
 /**
  * For using on localhost "offline"
  * 
  * Go to https://developers.google.com/oauthplayground
  * Authorize GDrive readonly and exchange for code
  * Paste code as gdauthcode below 
  */
 global $gdauthcode;
 $gdauthcode = 'ya29.swGXWS5Uh4rqvZFo1NHMUDuB9dmhNgjqY_JpTKXBCKMK-r56rseGilk8UU4USKNuvg-Z';
 
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
 * Checks to see if offline is authorized
 * 
 * Returns TRUE if authorized
 */
function gdrive_offline_authorized() {
	$token = variable_get('gdrive_local_token', FALSE);
	$auth = (gdrive_is_offline() && !empty($token)) ? TRUE : FALSE;
	return $auth;
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
 * 
 * Sets variable 'gdrive_local_token' as json text with following fields:
 * 			"access_token"
 * 			"token_type"
 * 			"expires_in"	
 * 			"refresh_token"
 * 			"id_token":
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

function gdrive_offline_filelist($gid=FALSE, $format='array') {
	global $gdauthcode;
	// Check out https://developers.google.com/oauthplayground/
	// Access token: ya29.agFiURVhqL4pf_Ld7e5G-k-3t0A-JiotPYOJbI8Zc0-hzwvOlUiw3YioYBiagBfHu_BJ820LGXrpEA

	if (!gdrive_offline_authorized()) {return array(); } // Return empty array if not authorized
	$token = json_decode(variable_get('gdrive_local_token'));
	$url = 'https://www.googleapis.com/drive/v2/files';
	if($gid) {
		$url .= "/$gid";
	}
	$token = json_decode(variable_get('gdrive_local_token'));
	$data = array(
	  'access_token' => $token->access_token,
	);
	$options = array(
		'protocol' => 'HTTP/1.1',
		'method' => 'GET',
		'headers' => array(
			'Authorization' => "Bearer {$gdauthcode}",
			'Content-length' => 0,
			'User-Agent' => '',
		 ),
	);

	$results = drupal_http_request($url, $options);
	$jdata = $results->data;
	$json = json_decode($jdata);
	$ss = array();
	if ($gid) {
		if($format == 'array') { $json = (array)$json; }
		return $json;
	} else if (empty($json->items)) {
		drupal_set_message('No GDocs found. Possibly your authorization to access GDrive has been revoked.');
		return $ss;
	}
	foreach($json->items as $n => $item) {
		if (strpos($item->mimeType, 'spreadsheet') > -1) {
			$item = (array)$item;
			$item['gid'] = $item['id'];
			$item['url'] = $item['alternateLink'];
			$ss[$n] = $item;
		}
	}
	return $ss;
	/*
	/// Doesn't Work
	$url = 'https://www.googleapis.com/drive/v2/files';
	$token = json_decode(variable_get('gdrive_local_token'));
	$data = array(
	  'access_token' => $token->access_token,
	);
	
	$options = array(
	  'method' => 'GET',
	  'data' => drupal_http_build_query($data),
	  'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
	);
	
	$result = drupal_http_request($url, $options);
	return $result;
	 * 
	 */
}

function gdrive_offline_get_file($gid) {
	// test id: 1JqQcnhDMz8gCMXy00bJe6mE0gpc4XbQSUl95cNM4Imo
	return gdrive_offline_filelist($gid);
}
