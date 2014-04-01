drupal_gdrive
=============

A drupal module for authenticating and interfacing with a user's Google Drive account.

# Requirements

* Libraries (Drupal module), 
* Google API PHP Client (a PHP library)

# Installation

## Install The Google API PHP Client

Navigate to your sites /sites/all/libraries folder or create one if it is not there. Then, clone the latest code for the Google API Client by running:

```
git clone https://github.com/google/google-api-php-client.git
```
The code should now be found at {yoursite}/sites/all/libraries/google-api-php-client and within that folder there should be a folder /src which contains a folder called Google. If this is the case, then it has been properly installed.

Further instructions on installation can be found at https://developers.google.com/api-client-library/php/start/installation However, the GDrive module takes care of the php include_path for this library.

## Register Your Site With Google

In order to use Google's Authentication API, you will need to register your site with Google. This is done by going to the Google Developer's Console at https://console.developers.google.com/project and clicking "Create Project". Next, click on the "APIS & Auth\APIs" link in the side column and turn on the Google Drive API. (The Google Drive SDK is not necessary.) Once this is done go to the "APIS & Auth\Credentials" page from the link in the side column, and press "Create New Client ID". Choose "Web application", and in the first text box put the base url of your site without a trailing slash, and in the second box put "{url of your site}/gdrive/response". Finally click "Create Client ID".

You will need to refer to the resulting screen later in the module installation. So keep it open.

## Install the GDrive Module

To install this module in your Drupal site, navigation to the site/all/modules folder or a subfolder within it and run the following command to clone the GIT repository:

```
git clone https://github.com/shanti-uva/drupal_gdrive.git gdrive
```

This will clone the module into a gdrive folder in your Drupal site's module folder.

Next, go to your site as administrator and enable the Gdrive module



