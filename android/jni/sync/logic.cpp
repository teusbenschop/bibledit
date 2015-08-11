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


#include <sync/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/roles.h>
#include <database/notes.h>
#include <database/noteactions.h>
#include <database/mail.h>
#include <database/logs.h>
#include <database/config/general.h>
#include <database/config/bible.h>
#include <database/usfmresources.h>
#include <database/offlineresources.h>
#include <database/modifications.h>
#include <config/logic.h>
#include <trash/handler.h>


Sync_Logic::Sync_Logic (void * webserver_request_in)
{
  webserver_request = webserver_request_in;
}


Sync_Logic::~Sync_Logic ()
{
}


bool Sync_Logic::credentials_okay ()
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Get the credentials the client POSTed to the us, the server.
  string username = hex2bin (request->post ["u"]);
  string password = request->post ["p"];
  int level = convert_to_int (request->post ["l"]);
  
  // Check all credentials.
  bool user_ok = request->database_users ()->usernameExists (username);
  if (!user_ok) Database_Logs::log ("Non existing user: " + username, Filter_Roles::manager ());
  bool pass_ok = (password == request->database_users ()->getmd5 (username));
  if (!pass_ok) Database_Logs::log ("Incorrect password: " + password, Filter_Roles::manager ());
  bool level_ok = (level == request->database_users ()->getUserLevel (username));
  if (!level_ok) Database_Logs::log ("Incorrect role: " + Filter_Roles::text (level), Filter_Roles::manager ());
  if (!user_ok || !pass_ok || !level_ok) {
    // Unauthorized.
    // Delay a while to obstruct a flood of requests with invalid credentials.
    this_thread::sleep_for (chrono::seconds (1));
    request->response_code = 401;
    return false;
  }
  
  // Set username in session.
  request->session_logic ()->setUsername (username);
  
  // OK.
  return true;
}


// Calculates the checksum of the array of note identifiers.
string Sync_Logic::checksum (const vector <int> & identifiers)
{
  Database_Notes database_notes = Database_Notes (webserver_request);
  vector <string> checksums;
  for (const auto & identifier : identifiers) {
    checksums.push_back (database_notes.getChecksum (identifier));
  }
  string checksum = filter_string_implode (checksums, "");
  checksum = md5 (checksum);
  return checksum;
}


// This function takes a start-ing note identifier,
// and an end-ing note identifier.
// It divides this range of identifier into ten 
// smaller ranges.
// It returns an array of array (start, end) with the new ranges 
// for the note identifiers.
vector <Sync_Logic_Range> Sync_Logic::create_range (int start, int end)
{
  int range = end - start;
  range = round (range / 10);
  vector <Sync_Logic_Range> ranges;
  for (unsigned int i = 0; i <= 9; i++) {
    int first = start + (i * range);
    int last = start + ((i + 1) * range) - 1;
    if (i == 9) {
      last = end;
    }
    Sync_Logic_Range item;
    item.low = first;
    item.high = last;
    ranges.push_back (item);
  }
  return ranges;
}


// Sends a post request to the url.
// It returns the server's response, or an empty string on failure.
// burst: Set the connection timing for a burst response after a relatively long silence.
string Sync_Logic::post (map <string, string> & post, const string& url, string & error, bool burst)
{
  error.clear ();
  string response = filter_url_http_post (url, post, error, burst);
  if (error.empty ()) {
    // Success: Return response.
    return response;
  }
  // Failure.
  return "";
}


// Calculates the checksum of all settings to be kept in sync between server and client.
string Sync_Logic::settings_checksum (const vector <string> & bibles)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  string checksum;
  checksum.append (request->database_config_user()->getWorkbenchURLs ());
  checksum.append (request->database_config_user()->getWorkbenchWidths ());
  checksum.append (request->database_config_user()->getWorkbenchHeights ());
  vector <string> resources = request->database_config_user()->getActiveResources ();
  checksum.append (filter_string_implode (resources, "\n"));
  for (auto & bible : bibles) {
    int id = request->database_bibles ()->getID (bible);
    checksum.append (convert_to_string (id));
    // Download Bible text font name: It is the default name for the clients.
    checksum.append (Database_Config_Bible::getTextFont (bible));
  }
  return md5 (checksum);
}


// Calculates the checksum of all USFM resources.
string Sync_Logic::usfm_resources_checksum ()
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <string> resources = database_usfmresources.getResources ();
  for (auto & resource : resources) {
    vchecksum.push_back (usfm_resource_checksum (resource));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name.
string Sync_Logic::usfm_resource_checksum (const string& name)
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <int> books = database_usfmresources.getBooks (name);
  for (auto & book : books) {
    vchecksum.push_back (convert_to_string (book));
    vchecksum.push_back (usfm_resource_book_checksum (name, book));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name book.
string Sync_Logic::usfm_resource_book_checksum (const string& name, int book)
{
  vector <string> vchecksum;
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  vector <int> chapters = database_usfmresources.getChapters (name, book);
  for (auto & chapter : chapters) {
    vchecksum.push_back (convert_to_string (chapter));
    vchecksum.push_back (usfm_resource_chapter_checksum (name, book, chapter));
  }
  string checksum = filter_string_implode (vchecksum, "");
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of USFM resource name book chapter.
string Sync_Logic::usfm_resource_chapter_checksum (const string& name, int book, int chapter)
{
  Database_UsfmResources database_usfmresources = Database_UsfmResources ();
  int checksum = database_usfmresources.getSize (name, book, chapter);
  return convert_to_string (checksum);
}


// Calculates the checksum of all offline resources.
string Sync_Logic::offline_resources_checksum ()
{
  string checksum;
  Database_OfflineResources database_offlineresources = Database_OfflineResources ();
  vector <string> names = database_offlineresources.names ();
  for (auto & name : names) {
    checksum += offline_resource_checksum (name);
  }
  checksum = md5 (checksum);
  return checksum;
}


// Calculates the checksum of offline resource name.
string Sync_Logic::offline_resource_checksum (const string& name)
{
  vector <string> checksum;
  Database_OfflineResources database_offlineresources = Database_OfflineResources ();
  vector <string> files = database_offlineresources.files (name);
  for (auto & file : files) {
    checksum.push_back (name);
    checksum.push_back (offline_resource_file_checksum (name, file));
  }
  string schecksum = filter_string_implode (checksum, "");
  schecksum = md5 (schecksum);
  return schecksum;
}


// Calculates the checksum of offline resource name the file.
string Sync_Logic::offline_resource_file_checksum (const string& name, const string& file)
{
  Database_OfflineResources database_offlineresources = Database_OfflineResources ();
  return convert_to_string (database_offlineresources.size (name, file));
}


// Calculates the total checksum for all the changes for $username.
string Sync_Logic::changes_checksum (const string & username)
{
  Database_Modifications database_modifications;
  vector <int> ids = database_modifications.getNotificationIdentifiers (username, false);
  string checksum;
  for (auto & id : ids) {
    checksum.append (convert_to_string (id));
  }
  checksum = md5 (checksum);
  return checksum;
}


// This function returns the root directories to go though on files sync.
// The $version influences which root directories to include.
// The $version is passed by the client to the server,
// so the server can adapt to the client's capabilities.
vector <string> Sync_Logic::files_get_directories (int version)
{
  vector <string> directories;
  switch (version) {
    case 1:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources"
      };
      break;
    case 2:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources",
        "databases/imageresources"
      };
      break;
    case 3:
      directories = {
        "fonts",
        "databases/usfmresources",
        "databases/offlineresources",
        "databases/imageresources",
        "databases/client"
      };
      break;
    default:
      break;
  }
  return directories;
}


// This returns the total checksum for all directories and files relevant to $version.
int Sync_Logic::files_get_total_checksum (int version)
{
  int checksum = 0;
  vector <string> directories = files_get_directories (version);
  for (auto directory : directories) {
    checksum += files_get_directory_checksum (directory);
  }
  return checksum;
}


// This returns the total checksum for all files in one root directory.
// It does a recursive scan for the files.
int Sync_Logic::files_get_directory_checksum (string directory)
{
  int checksum = 0;
  vector <string> files = files_get_files (directory);
  for (string file : files) {
    checksum += files_get_file_checksum (directory, file);
  }
  return checksum;
}


// This returns all the paths of the files within $directory.
// $directory is relative to the web root.
// It does a recursive scan for the files.
vector <string> Sync_Logic::files_get_files (string directory)
{
  directory = filter_url_create_root_path (directory);
  vector <string> result;
  vector <string> paths;
  filter_url_recursive_scandir (directory, paths);
  for (string path : paths) {
    if (filter_url_is_dir (path)) continue;
    string extension = filter_url_get_extension (path);
    if (extension == "o") continue;
    if (extension == "h") continue;
    if (extension == "cpp") continue;
    path.erase (0, directory.length () + 1);
    result.push_back (path);
  }
  return result;
}


// This returns the checksum of a $file in $directory.
int Sync_Logic::files_get_file_checksum (string directory, string file)
{
  string path = filter_url_create_root_path (directory, file);
  int checksum = filter_url_filesize (path);
  return checksum;
}
