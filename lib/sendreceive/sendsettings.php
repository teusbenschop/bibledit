<?php
/*
Copyright (©) 2003-2015 Teus Benschop.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


require_once ("../bootstrap/bootstrap"); // Todo port it.


// Security: The script runs from the cli SAPI only.
Filter_Cli::assert ();


$database_logs = Database_Logs::getInstance ();
$database_config_bible = Database_Config_Bible::getInstance ();
$database_config_general = Database_Config_General::getInstance ();
$database_config_user = Database_Config_User::getInstance ();
$database_users = Database_Users::getInstance ();
$session_logic = Session_Logic::getInstance ();


Database_Logs::log (gettext("Sending and receiving Settings"), Filter_Roles::translator ());


$response = client_logic_enable_clientup ();
if ($response === false || $response < Filter_Roles::guest () || $response > Filter_Roles::admin ()) {
  Database_Logs::log (gettext("Failure sending and receiving Settings"), Filter_Roles::translator ());
  die;
}


// Set the correct user in the session, which is sole user on the Client.
$users = $database_users->getUsers ();
$user = $users [0];
$session_logic->setUsername ($user);


$address = Database_Config_General::getServerAddress ();
$url = "$address/sync/setting";


// Go through all settings flagged as having been updated on this client.
$ids = request->database_config_user()->getUpdatedSettings ();
if (!empty ($ids)) {
  Database_Logs::log (gettext("Sending settings"), Filter_Roles::translator ());
}
for ($ids as $id) {

  // Get and serialize the setting.
  $setting = null;
  switch ($id) {
    case Sync_Logic::WORKBENCH_SETTING:
    {
      $urls = request->database_config_user()->getWorkbenchURLs ();
      $urls = unserialize ($urls);
      $widths = request->database_config_user()->getWorkbenchWidths ();
      $widths = unserialize ($widths);
      $heights = request->database_config_user()->getWorkbenchHeights ();
      $heights = unserialize ($heights);
      $setting = array ();
      $setting ['urls'] = $urls;
      $setting ['widths'] = $widths;
      $setting ['heights'] = $heights;
      break;
    }
  }

  // POST the setting to the server.
  $post = array (
    "u" => bin2hex ($user),
    "p" => $database_users->getmd5 ($user),
    "l" => $database_users->getUserLevel ($user),
    "i" => $id,
    "s" => serialize ($setting)
  );
  $response = Sync_Logic::post ($post, $url);

  // Handle server's response.
  if ($response === false) {
    Database_Logs::log ("Failure sending setting to server", Filter_Roles::translator ());
  } else {
    request->database_config_user()->removeUpdatedSetting ($id);
  }

}


// All changed settings have now been sent to the server.
// The client will now synchronize its settings with the server's settings.
tasks_logic_queue (Tasks_Logic::PHP, array (__DIR__ . "/syncsettings"));


?>
