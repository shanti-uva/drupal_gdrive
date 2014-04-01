drupal_gdrive
=============

A drupal module for authenticating and interfacing with a user's Google Drive account. It presently only gives access to a users spreadsheets on their Google Drive account.

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

Next, go to your site as administrator and enable the Gdrive module which will be listed under a project called SHANTI.

Once enabled you will see a warning saying:

> Make sure the Google Project authentication credentials are properly entered in the configuration page for this module.

The link will take you to the configuration page at {your site}/admin/config/services/gdrive

Go there and enter the Client ID and the Client Secret Code from the Google registration page mentioned above. The third text box is for an option url on your site where you want users redirected upon login.

## How It Works

The module uses Googles OAuth 2 authentication as described in https://developers.google.com/accounts/docs/OAuth2WebServer It does so through the PHP client api library. New users are shown a warning saying that they have not allowed this site to access their Google Drive account with a link to do so. The link goes to a page: {your site}/gdrive/auth which shows a page requesting permission to access their Google Drive account. If they click the Yes button, they are taken *one time* to Googles Authentication page, where they can then grant permission. The module then stores the resulting token and can read the user's Google Drive account. A message is shown on the user's profile page saying they have given permission to the site to access their Google Drive account and a link to revoke such access if they so choose.

## How To Use This Module
This module is primarily meant to be an intermediary or helper module for other modules that want to work with Google Drive spreadsheets. Thus, it provides only one "page" {your site}/gdrive/spreadsheets that shows a list of all the users spreadsheets.

The primary use for this module if for other modules to access a user's spreadsheets through the gdrive_get_filelist() function. This function returns an array of all the user's spreadsheets. Each spreadsheet is represented with a named array with the following fields:

* gid: The Google ID for the file
* title: The title of the file
* url: the URL of the file
* created: The date the file was created
* owners: A string of owners names separated by dollar signs, e.g. owner1$owner2$owner3
* shared: A boolean whether the file is shared or not

Developers can then use this information within their own modules as desired.



