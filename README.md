drupal-media-mediahaven
====================

This configuration guide is based on the standard drupal installation, the current version of drupal is 7.25. Any issues can be reported to [github] (https://github.com/Zeticon/drupal-media-mediahaven/issues).

For more information please visit [www.zeticon.com](http://www.zeticon.com) or contact us at [info@zeticon.com](mailto:info@zeticon.com)

Install the media_mediahaven module
-----------------------------------
Download the latest release and copy the mediahaven module to "sites/all/modules" and rename the folder to "media_mediahaven" if necessary or rename the zip file to "media_mediahaven" before uploading.

Required Modules
---------------------
Upload the following required modules into the folder "sites/all/modules", some modules require additional modules to operate!
*   [Media 2.x] (https://drupal.org/project/media)
*   [File_entity](https://drupal.org/project/file_entity)
*   [Views](https://drupal.org/project/views)
*   [Ctools](https://drupal.org/project/ctools)
*   [Styles](https://drupal.org/project/styles)
*   [Plupload](https://drupal.org/project/plupload)


Go to the "Modules" panel in your admin view and enable the listed modules.

Required Library
---------------------
Additionally to the module "plupload", the plupload library is required.
Download the plupload package from: [www.plupload.com/download](http://www.plupload.com/download/). It is important to note that the current php plupload module does not work with the 2+ version of the library. You will need to download version 1.5.x.

Create a new folder "libraries" under "sites/all/" and upload the plupload package to this folder and extract it there. Rename the extracted folder to 'plupload', so you have a folder structure: "sites/all/libraries/plupload". 

Mediahaven Configuration Settings
---------------------
<h3>MediaHaven API</h3>
*MediaHaven login:* The login to be used by the module to communicate with MediaHaven.  
*MediaHaven password:* The password to be used by the module to communicate with MediaHaven.   
*MediaHaven url:* The MediaHaven url.   
	
<h3>MediaHaven Upload settings</h3>
*Ingestspace id:* The MediaHaven ingestspace that will be used to upload files to.  
*Auto publish:* When enabled files will be automatically published after processing. If disabled a user will have to manually publish them using the mediahaven media management software.  
*Department id:* The default department where uploaded files will belong to. This will be used when autopublishing is enabled.  

<h3>MediaHaven Thumbnail settings</h3>
*Thumbnail width:* Width of the thumbnails (in pixels), standard is 150px  
*Thumbnail height:* Height of the thumbnails (in pixels), standard is 150px  

These settings require a Mediahaven account, for more information please visit [www.zeticon.com](http://www.zeticon.com)

---------------------

Configure a Media File Selector
---------------------
Linking media items to a node requires the configuration of a Media Selector Field for mediahaven.

Select "Structure" > "Content Types" from the main menu, followed by the operation "manage fields" for the preferred content type.

Create a new field by entering a field name of your choice (eg 'Mediahaven Image'), select the field type "Multimedia Asset" (preferred) or "File".
Set the widget to "Media File Selector" and confirm by subsequently clicking the "Save"-button and "Save Field"-button.

To complete the setup configure the following settings: 
*enable "Mediahaven" in the enabled browser plugins, 
*enable alle desired file types in Allowed remote media types 
*select "mediahaven:// (MediaHaven media)" in Allowed URI schemes

When editing a node of the content type, a Media Selector Field should be available.

---------------------

WYSIWYG
---------------------
<h3>ckeditor</h3>
https://drupal.org/comment/6814058#comment-6814058

###Media Browser


To enable MediaHaven file selection form within the media browser
* go to "configuration/media browser settings" and check MediaHaven.

Ajax callbacks in the media overlay cause the active tab to switch tab unexpectedly
---------------------
This is a known issue and can be fixed with this patch: https://drupal.org/node/1802026


---------------------

Usage of the module
---------------------
In order to include media from Mediahaven into Drupal, go to Content and click the link "Add Media from Mediahaven".
Enter a search term and/or click any of the provided facets.
Select an by clicking on the image.
A confirmation "Successfully added media" will be shown and the media file will be available for usage under the "Files" tab.

Testing the module
-------------------
The module and the rest-api webservices can be tested against an integration platform: https://integration.mediahaven.com

Following credentials can be used:
login: apikey
password: apikey

The rest webservices url: https://integration.mediahaven.com/mediahaven-rest-api/
