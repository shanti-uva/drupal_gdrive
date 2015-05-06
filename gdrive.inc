<?php
/**
 * @file
 *  Include file for Shanti Google Drive Module contains Exception classes and functions for Google Drive authentication
 *    Taken from: https://developers.google.com/drive/web/auth/web-server
 */

/**
 * Exception thrown when an error occurred while retrieving credentials.
 */
class GetCredentialsException extends Exception {
  protected $authorizationUrl;

  /**
   * Construct a GetCredentialsException.
   *
   * @param authorizationUrl The authorization URL to redirect the user to.
   */
  public function __construct($authorizationUrl) {
    $this->authorizationUrl = $authorizationUrl;
  }

  /**
   * @return the authorizationUrl.
   */
  public function getAuthorizationUrl() {
    return $this->authorizationUrl;
  }

  /**
   * Set the authorization URL.
   */
  public function setAuthorizationUrl($authorizationUrl) {
    $this->authorizationUrl = $authorizationUrl;
  }
}

/**
 * Exception thrown when no refresh token has been found.
 */
class NoRefreshTokenException extends GetCredentialsException {}

/**
 * Exception thrown when a code exchange has failed.
 */
class CodeExchangeException extends GetCredentialsException {}

/**
 * Exception thrown when no user ID could be retrieved.
 */
class NoUserIdException extends Exception {}

/**
 * Returns a new php Client object based on sites Google console settings. 
 */
function gdrive_get_client($state='') {
  global $base_url;
  if($state == '') { $state = gdrive_get_state(); }
  $appinfo =  gdrive_get_google_auth_settings(); // Retrieve Google Console settings for this site. Set in administration page for module.
  $scopes = gdrive_get_scopes();
  $client = new Google_Client(); // Create new client and give it site specific info.
  $client->setClientId($appinfo['client']);
  $client->setClientSecret($appinfo['secret']);
  $client->setRedirectUri($base_url . url($appinfo['redirect']));
  $client->setAccessType('offline');
  $client->setApprovalPrompt('auto');
  $client->setState($state);
  $client->setScopes($scopes);
  return $client;
}

/**
 * Retrieved stored credentials for the provided user ID.
 *
 * @param String $userId User's ID.
 * @return String Json representation of the OAuth 2.0 credentials.
 */
function gdrive_get_stored_credentials($uid) {
  $query = db_select('gdrive_auth', 'sga');
  $query->condition('sga.uid', $uid, '=');
  $query->fields('sga');
  $response = $query->execute();
  if($response->rowCount() == 0 ) {
    return FALSE;
  } else {
    $record = $response->fetchAssoc();
    return $record['credentials'];
  }
}

/**
 * When currently stored token is out of date, use the refresh token to get a new one and store that.
 */
function gdrive_refresh_token($creds) {
  $client = gdrive_get_client();
  $client->setAccessToken(json_encode($creds));
  $client->refreshToken($creds->refresh_token);
  $newtoken = $client->getAccessToken();
  return $newtoken;
}

/**
 * Revoke previously granted authorization
 *   TODO: Check if this should delete record from database as well
 */
function gdrive_revoke_auth($uid) {
  $client = gdrive_get_client();
  $creds = json_decode(gdrive_get_stored_credentials($uid));
  $token = $creds->access_token;
  $result = $client->getAuth()->revokeToken($token); 
  return $result;
}

/**
 * Store OAuth 2.0 credentials in the application's database.
 *
 * @param String $userId User's ID.
 * @param String $credentials Json representation of the OAuth 2.0 credentials to
                              store.
 */
function gdrive_store_credentials($uid, $credentials) {
  $query = db_select('gdrive_auth', 'gda');
  $query->condition('uid',$uid)->fields('gda');
  $results = $query->execute();
  if($results->rowCount() == 0) {
    $credinsert = db_insert('gdrive_auth')
      ->fields(array(
        'uid' => $uid,
        'credentials' => $credentials,
      ))
      ->execute();
   } else {
     db_update('gdrive_auth')
      ->fields(array(
        'credentials' => $credentials
       ))
      ->condition('uid', $uid, '=')
      ->execute();
   }
}

/**
 * Exchange an authorization code for OAuth 2.0 credentials.
 *
 * @param String $authorizationCode Authorization code to exchange for OAuth 2.0
 *                                  credentials.
 * @return String Json representation of the OAuth 2.0 credentials.
 * @throws CodeExchangeException An error occurred.
 */
function gdrive_exchange_code($authorizationCode) {
  try {
    $client = gdrive_get_client();
    return $client->authenticate($authorizationCode);
  } catch (Google_AuthException $e) {
    print 'An error occurred: ' . $e->getMessage();
    throw new CodeExchangeException(null);
  }
}

/**
 * Retrieve the authorization URL.
 *
 * @param String $emailAddress User's e-mail address.
 * @param String $state State for the authorization URL.
 * @return String Authorization URL to redirect the user to.
 */
function gdrive_get_auth_url($state) {
  global $base_url;
  $client = gdrive_get_client($state);
  return $client->createAuthUrl();
}

/**
 * Retrieve credentials using the provided authorization code.
 *
 * This function exchanges the authorization code for an access token and
 * queries the UserInfo API to retrieve the user's e-mail address. If a
 * refresh token has been retrieved along with an access token, it is stored
 * in the application database using the user's e-mail address as key. If no
 * refresh token has been retrieved, the function checks in the application
 * database for one and returns it if found or throws a NoRefreshTokenException
 * with the authorization URL to redirect the user to.
 *
 * @param String authorizationCode Authorization code to use to retrieve an access
 *                                 token.
 * @param String state State to set to the authorization URL in case of error.
 * @return String Json representation of the OAuth 2.0 credentials.
 * @throws NoRefreshTokenException No refresh token could be retrieved from
 *         the available sources.
 */
function gdrive_get_credentials($authorizationCode, $state) {
  global $user;
  try {
    $credentials = gdrive_exchange_code($authorizationCode);
    $credsArray = json_decode($credentials, true);
    if (isset($credsArray['refresh_token'])) {  // both this and the line below had 'refresh_token' (ndg, 2014-03-27) Changed 
      return $credentials;
    } else {
      $oldcreds = json_decode(gdrive_get_stored_credentials($user->uid), true);
      $oldcreds['access_token'] == $credsArray['access_token'];
      $credentials = json_encode($oldcreds);
      return $credentials;
    }
  } catch (CodeExchangeException $e) {
    print 'An error occurred during code exchange.';
    // Drive apps should try to retrieve the user and credentials for the current
    // session.
    // If none is available, redirect the user to the authorization URL.
    $e->setAuthorizationUrl(gdrive_get_auth_url($state));
    throw $e;
  } catch (NoUserIdException $e) {
    print 'No e-mail address could be retrieved.';
  }
  
  // No refresh token has been retrieved.
  $authorizationUrl = gdrive_get_auth_url($state); 
  throw new NoRefreshTokenException($authorizationUrl);
}

/**
 * Returns a list of file API objects for spreadsheets in current user's account
 * 
 * @param String $sorted filed field to sort the list by. Defaults to title.
 * 
 * @return Array Named array of the following information for each file: gid, title, url, created, owners, shared.
 * 
 */
function gdrive_get_filelist($sorted='title') {
  global $user;
	if(gdrive_is_offline()) {return gdrive_offline_filelist(); }
  $client = gdrive_get_client();
  $creds = gdrive_get_stored_credentials($user->uid);
  $credobj = ($creds) ? json_decode($creds) : FALSE;
  if($credobj && (!isset($credobj->revoked) || $credobj->revoked != TRUE)) {
    $client->setAccessToken($creds);
    $driveService = new Google_Service_Drive($client);
    $files = $driveService->files->listFiles(array(
         'q' => 'mimeType=\'application/vnd.google-apps.spreadsheet\'',  // filter out everything but spreadsheets
         'maxResults' => 10000
       ))->getItems();
    $newfiles = array();
    foreach($files as $file) {
      $farray = gdrive_file_attributes($file);
      array_push($newfiles, $farray);
    }
    if($sorted == 'title') {
      $titles = array();
      foreach($newfiles as $key => $f) {
        $titles[$key] = $f['title'];
      }
      array_multisort($titles, SORT_ASC, $newfiles);
    }
    return $newfiles;
  } else {
    return FALSE;
  }
}

/**
 * Returns a file object for a given Google ID
 * 
 * @params String $gid An alpha-numeric ID for the spreadsheet assigned by Google
 * 
 * @return Object File The php file object for that spreadsheet.
 */
function gdrive_get_file($gid, $format='array') {
  // Returns a files resource: https://developers.google.com/drive/v2/reference/files#resource
  global $user;
	if (gdrive_is_offline()) {return gdrive_offline_get_file($gid); }
  $client = gdrive_get_client();
  $creds = gdrive_get_stored_credentials($user->uid);
  $credobj = ($creds) ? json_decode($creds) : FALSE;
  if ($credobj && (!isset($credobj->revoked) || $credobj->revoked != TRUE)) {
    $client->setAccessToken($creds);
    $driveService = new Google_Service_Drive($client);
    if (strpos($gid, '?key=') > -1) {
      $pts = explode('?key=', $gid);
      if (isset($pts[1])) {
        $gid = $pts[1];
        if (preg_match('/([^\&\#]+)[\&\#]/', $gid, $matches)) {
          $gid = $matches[1];
        }
      }
    }
    try { 
      $file = $driveService->files->get($gid);
			if ($format == 'object') { return $file; }
      return (array)$file;
    } catch (Google_Service_Exception $gse) {
      return FALSE;
    }	
  }
}

/**
 * Function that returns a set array of file attributes from a file object
 */
function gdrive_file_attributes($file) {
  return array(
    'gid' => $file->getId(),
    'title' => $file->getTitle(),
    'url' => $file->getAlternateLink(),
    /** Unnecessary right now
    'embedurl' => $file->getEmbedLink(),
    'exportlinks' => $file->getExportLinks(),
    'self' => $file->getSelfLink(),
     **/
    /* These all return empty strings:
    'defaulturl' => $file->getDefaultOpenWithLink(),
    'wcurl' => $file->getWebContentLink(),
    'wvulr' => $file->getWebViewLink(),
     */
    'created' => $file->getCreatedDate(),
    'owners' => implode('$', $file->getOwnerNames()),
    'shared' => $file->getShared(),
  );
}

/**
 * Function to determine if a given user has or has not authorized reading of their Google Drive account
 */
function gdrive_is_authorized($uid = FALSE) {
  global $user;
	if (gdrive_offline_authorized()) { return TRUE; }
  if (!$uid) { $uid = $user->uid; }
  $client = gdrive_get_client();
  $creds = gdrive_get_stored_credentials($uid);
  $credobj = ($creds) ? json_decode($creds) : FALSE;
  if ($credobj && (!isset($credobj->revoked) || $credobj->revoked != TRUE)) {
    return TRUE;
  } else {
    return FALSE;
  }
}
