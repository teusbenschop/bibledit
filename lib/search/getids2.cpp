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


#include <search/getids2.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>


string search_getids2_url ()
{
  return "search/getids2";
}


bool search_getids2_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string search_getids2 (void * webserver_request)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  
  // Get search variables from the query.
  string bible = request->query ["b"];
  string searchfor = request->query ["q"];
  bool casesensitive = (request->query ["c"] == "true");
  bool searchplain = (request->query ["p"] == "true");
  
  
  // Do the search.
  vector <int> hits;
  /* Todo
  if (casesensitive) {
    if (searchplain) {
      hits = request->database_search()->searchBibleTextCaseSensitive (bible, searchfor);
    } else {
      hits = request->database_search()->searchBibleUsfmCaseSensitive (bible, searchfor);
    }
  } else {
    if (searchplain) {
      hits = request->database_search()->searchBibleText (bible, searchfor);
    } else {
      hits = request->database_search()->searchBibleUsfm (bible, searchfor);
    }
  }
   */
  
  
  // Output identifiers of the search results.
  string output;
  for (auto & hit : hits) {
    if (!output.empty ()) output.append ("\n");
    output.append (convert_to_string (hit));
  }
  return output;
}
