/*
Copyright (©) 2003-2016 Teus Benschop.

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


#include <filter/usfm.h>
#include <filter/string.h>
#include <filter/diff.h>
#include <database/books.h>
#include <database/styles.h>
#include <database/logs.h>
#include <database/config/bible.h>
#include <styles/logic.h>
#include <webserver/request.h>
#include <bible/logic.h>
#include <locale/translate.h>


BookChapterData::BookChapterData (int book_in, int chapter_in, string data_in)
{
  book = book_in;
  chapter = chapter_in;
  data = data_in;
}


UsfmNote::UsfmNote (int offset_in, string data_in)
{
  offset = offset_in;
  data = data_in;
}


// Returns the string $usfm as one long string.
// $usfm may contain new lines, but the resulting long string won't.
string usfm_one_string (string usfm)
{
  string long_string = "";
  vector <string> usfm_lines = filter_string_explode (usfm, '\n');
  for (string & line : usfm_lines) {
    line = filter_string_trim (line);
    // Skip empty line.
    if (line != "") {
      // The line will be appended to the output line.
      // If it does not start with a backslash (\), a space is inserted first.
      size_t pos = line.find ("\\");
      if (pos != 0) {
        if (long_string != "") long_string += " ";
      }
      long_string += line;
    }
  }
  return long_string;
}


// Returns the string $code as an array alternating between marker and text.
// Example, input is:   \id GEN
//                      \c 10
//             ...
// Output would be:     array ("\id ", "GEN", "\c ", "10", ...)
// If $code does not start with a marker, this becomes visible in the output too.
vector <string> usfm_get_markers_and_text (string code)
{
  vector <string> markers_and_text;
  code = filter_string_str_replace ("\n\\", "\\", code); // New line followed by backslash: leave new line out.
  code = filter_string_str_replace ("\n", " ", code); // New line only: change to space, according to the USFM specification.
  // No removal of double spaces, because it would remove an opening marker (which already has its own space), followed by a space.
  code = filter_string_trim (code);
  while (!code.empty ()) {
    size_t pos = code.find ("\\");
    if (pos == 0) {
      // Marker found.
      // The marker ends
      // - after the first space, or
      // - after the first asterisk (*), or
      // - at the first backslash (\), or
      // - at the end of the string,
      // whichever comes first.
      vector <size_t> positions;
      pos = code.find (" ");
      if (pos != string::npos) positions.push_back (pos + 1);
      pos = code.find ("*");
      if (pos != string::npos) positions.push_back (pos + 1);
      pos = code.find ("\\", 1);
      if (pos != string::npos) positions.push_back (pos);
      positions.push_back (code.length());
      sort (positions.begin (), positions.end());
      pos = positions[0];
      string marker = code.substr (0, pos);
      markers_and_text.push_back (marker);
      code = code.substr (pos);
    } else {
      // Text found. It ends at the next backslash or at the end of the string.
      pos = code.find ("\\");
      if (pos == string::npos) pos = code.length();
      string text = code.substr (0, pos);
      markers_and_text.push_back (text);
      code = code.substr (pos);
    }
  }
  return markers_and_text;
}


// Gets the marker from $usfm if it is there, else returns an empty string.
// Examples:
// "\id"    -> "id"
// "\id "   -> "id"
// "\add*"  -> "add"
// "\+add*" -> "add"
string usfm_get_marker (string usfm)
{
  if (usfm.empty ()) return usfm;
  size_t pos = usfm.find ("\\");
  if (pos == 0) {
    // Marker found.
    // Erase backslash.
    usfm = usfm.substr (1);
    // Optionally erase the + embedded marker.
    pos = usfm.find ("+");
    if (pos == 0) usfm = usfm.substr (1);
    // The marker ends
    // - at the first space, or
    // - at the first asterisk (*), or
    // - at the first backslash (\), or
    // - at the end of the string,
    // whichever comes first.
    vector <size_t> positions;
    pos = usfm.find (" ");
    if (pos != string::npos) positions.push_back (pos);
    pos = usfm.find ("*");
    if (pos != string::npos) positions.push_back (pos);
    pos = usfm.find ("\\");
    if (pos != string::npos) positions.push_back (pos);
    positions.push_back (usfm.length());
    sort (positions.begin(), positions.end());
    pos = positions[0];
    string marker = usfm.substr (0, pos);
    return marker;
  }
  // Text found. No marker.
  return "";
}


// This imports USFM $input.
// It takes raw $input,
// and returns a vector with objects with book_number, chapter_number, chapter_data.
vector <BookChapterData> usfm_import (string input, string stylesheet)
{
  vector <BookChapterData> result;

  int book_number = 0;
  int chapter_number = 0;
  string chapter_data = "";

  input = usfm_one_string (input);
  vector <string> markers_and_text = usfm_get_markers_and_text (input);
  bool retrieve_book_number_on_next_iteration = false;
  bool retrieve_chapter_number_on_next_iteration = false;

  for (string marker_or_text : markers_and_text) {
    if (retrieve_book_number_on_next_iteration) {
      book_number = Database_Books::getIdFromUsfm (marker_or_text.substr (0, 3));
      chapter_number = 0;
      retrieve_book_number_on_next_iteration = false;
    }
    if (retrieve_chapter_number_on_next_iteration) {
      retrieve_chapter_number_on_next_iteration = false;
      chapter_number = convert_to_int (marker_or_text);
    }
    string marker = usfm_get_marker (marker_or_text);
    if (marker != "") {
      // USFM marker found.
      bool store_chapter_data = false;
      if (marker == "id") {
        retrieve_book_number_on_next_iteration = true;
        store_chapter_data = true;
      }
      if (marker == "c") {
        retrieve_chapter_number_on_next_iteration = true;
        store_chapter_data = true;
      }
      if (store_chapter_data) {
        chapter_data = filter_string_trim (chapter_data);
        if (chapter_data != "") result.push_back ( { book_number, chapter_number, chapter_data } );
        chapter_number = 0;
        chapter_data = "";
        store_chapter_data = false;
      }
      Database_Styles database_styles;
      Database_Styles_Item marker_data = database_styles.getMarkerData (stylesheet, marker);
      int type = marker_data.type;
      int subtype = marker_data.subtype;
      if (styles_logic_starts_new_line_in_usfm (type, subtype)) {
        chapter_data += "\n";
      }
    }
    chapter_data += marker_or_text;
  }
  chapter_data = filter_string_trim (chapter_data);
  if (chapter_data != "") result.push_back (BookChapterData (book_number, chapter_number, chapter_data));
  return result;
}


// Returns an array with the verse numbers found in $usfm.
// It handles a single verse, a range of verses, or a sequence of verses.
// It locates separate whole verse numbers.
// Examples:
// 10
// 10-12b
// 10,11a
// 10,12
vector <int> usfm_get_verse_numbers (string usfm)
{
  vector <int> verse_numbers = { 0 };
  vector <string> markers_and_text = usfm_get_markers_and_text (usfm);
  bool extract_verse = false;
  for (string marker_or_text : markers_and_text) {
    if (extract_verse) {
      string verse = usfm_peek_verse_number (marker_or_text);
      
      // If there is a range, take the beginning and the end and fill up in between.
      if (verse.find("-") != string::npos) {
        size_t position;
        position = verse.find("-");
        string start_range, end_range;
        start_range = verse.substr (0, position);
        verse.erase (0, ++position);
        end_range = verse;
        unsigned int start_verse_i = convert_to_int(number_in_string(start_range));
        unsigned int end_verse_i = convert_to_int(number_in_string(end_range));
        for (unsigned int i = start_verse_i; i <= end_verse_i; i++) {
          if (i == start_verse_i)
            verse_numbers.push_back (convert_to_int (start_range));
          else if (i == end_verse_i)
            verse_numbers.push_back (convert_to_int (end_range));
          else
            verse_numbers.push_back (i);
        }
      }
      
      // Else if there is a sequence, take each verse in the sequence, and store it.
      else if (verse.find(",") != string::npos) {
        int iterations = 0;
        do {
          // In case of an unusual range formation, do not hang, but give message.
          iterations++;
          if (iterations > 50) {
            break;
          }
          size_t position = verse.find (",");
          string vs;
          if (position == string::npos) {
            vs = verse;
            verse.clear ();
          } else {
            vs = verse.substr (0, position);
            verse.erase(0, ++position);
          }
          verse_numbers.push_back (convert_to_int (vs));
        } while (!verse.empty());
      }
      
      // No range and no sequence: a single verse.
      else {
        verse_numbers.push_back (convert_to_int (verse));
      }
      
      extract_verse = false;
    }
    if (marker_or_text.substr (0, 2) == "\\v") {
      extract_verse = true;
    }
  }
  return verse_numbers;
}


// Returns the verse numbers in the string of $usfm code at line number $line_number.
vector <int> usfm_linenumber_to_versenumber (string usfm, unsigned int line_number)
{
  vector <int> verse_number = {0}; // Initial verse number.
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (unsigned int i = 0; i < lines.size(); i++) {
    if (i <= line_number) {
      vector <int> verse_numbers = usfm_get_verse_numbers (lines[i]);
      if (verse_numbers.size() >= 2) {
        verse_number = filter_string_array_diff (verse_numbers, {0});
      }
    }
  }
  return verse_number;
}


// Returns the verse numbers in the string of $usfm code at offset $offset.
// Offset is calculated with unicode_string_length to support UTF-8.
vector <int> usfm_offset_to_versenumber (string usfm, unsigned int offset)
{
  unsigned int totalOffset = 0;
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (unsigned int i = 0; i < lines.size(); i++) {
    int length = unicode_string_length (lines [i]);
    totalOffset += length;
    if (totalOffset >= offset) {
      return usfm_linenumber_to_versenumber (usfm, i);
    }
    // Add 1 for new line.
    totalOffset += 1;
  }
  return {0};
}



// Returns the offset within the $usfm code where $verse number starts.
int usfm_versenumber_to_offset (string usfm, int verse)
{
  // Verse number 0 starts at offset 0.
  if (verse == 0) return 0;
  int totalOffset = 0;
  vector <string> lines = filter_string_explode (usfm, '\n');
  for (string line : lines) {
    vector <int> verses = usfm_get_verse_numbers (line);
    for (auto & v : verses) {
      if (v == verse) return totalOffset;
    }
    totalOffset += unicode_string_length (line);
    // Add 1 for new line.
    totalOffset += 1;
  }
  return unicode_string_length (usfm);
}


// Returns the verse text given a $verse_number and $usfm code.
// Handles combined verses.
string usfm_get_verse_text (string usfm, int verse_number)
{
  vector <string> result;
  bool hit = (verse_number == 0);

  vector <string> lines = filter_string_explode (usfm, '\n');
  for (string line : lines) {
    vector <int> verses = usfm_get_verse_numbers (line);
    if (verse_number == 0) {
      if (verses.size () != 1) hit = false;
      if (hit) result.push_back (line);
    } else {
      if (in_array (verse_number, verses)) {
        // Desired verse found.
        hit = true;
      } else if (verses.size () == 1) {
        // No verse found: No change in situation.
      } else {
        // Outside desired verse.
        hit = false;
      }
      if (hit) result.push_back (line);
    }
  }
  
  // Return the verse text.
  string verseText = filter_string_implode (result, "\n");
  return verseText;
}


// Returns the USFM text for a range of verses for the input $usfm code.
// It handles combined verses.
// It ensures that the $exclude_usfm does not make it to the output of the function.
string usfm_get_verse_range_text (string usfm, int verse_from, int verse_to, const string& exclude_usfm)
{
  vector <string> bits;
  string previous_usfm;
  for (int vs = verse_from; vs <= verse_to; vs++) {
    string verse_usfm = usfm_get_verse_text (usfm, vs);
    // Do not include repeating USFM in the case of combined verse numbers in the input USFM code.
    if (verse_usfm == previous_usfm) continue;
    previous_usfm = verse_usfm;
    // In case of combined verses, the excluded USFM should not be included in the result.
    if (verse_usfm != exclude_usfm) {
      bits.push_back (verse_usfm);
    }
  }
  usfm = filter_string_implode (bits, "\n");
  return usfm;
}


// Returns true if the $code contains a USFM marker.
bool usfm_is_usfm_marker (string code)
{
  if (code.length () < 2) return false;
  if (code.substr (0, 1) == "\\") return true;
  return false;
}


// Returns true if the marker in $usfm is an opening marker.
// Else it returns false.
bool usfm_is_opening_marker (string usfm)
{
  return usfm.find ("*") == string::npos;
}


// Returns true if the marker in $usfm is an embedded marker.
// Else it returns false.
bool usfm_is_embedded_marker (string usfm)
{
  return usfm.find ( "+") != string::npos;
}


// Returns the USFM book identifier.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: if increased by one, it should point to the \id in $usfm.
string usfm_get_book_identifier (const vector <string>& usfm, unsigned int pointer)
{
  string identifier = "XXX"; // Fallback value.
  if (++pointer < usfm.size ()) {
    identifier = usfm[pointer].substr (0, 3);
  }
  return identifier;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. It gets increased by one.
string usfm_get_text_following_marker (const vector <string>& usfm, unsigned int & pointer)
{
  string text = ""; // Fallback value.
  ++pointer;
  if (pointer < usfm.size()) {
    text = usfm [pointer];
  }
  return text;
}


// Returns the text that follows a USFM marker.
// $usfm: array of strings alternating between USFM code and subsequent text.
// $pointer: should point to the marker in $usfm. Pointer is left as it is.
string usfm_peek_text_following_marker (const vector <string>& usfm, unsigned int pointer)
{
  return usfm_get_text_following_marker (usfm, pointer);
}


// Returns the verse number in the $usfm code.
string usfm_peek_verse_number (string usfm)
{
  // Make it robust, even handling cases like \v 1-2“Moi - No space after verse number.
  string verseNumber = "";
  size_t usfmStringLength = usfm.length ();
  unsigned int i = 0;
  for (i = 0; i < usfmStringLength; i++) {
    string character = usfm.substr (i, 1);
    if (character == "0") continue;
    if (character == "1") continue;
    if (character == "2") continue;
    if (character == "3") continue;
    if (character == "4") continue;
    if (character == "5") continue;
    if (character == "6") continue;
    if (character == "7") continue;
    if (character == "8") continue;
    if (character == "9") continue;
    if (character == ",") continue;
    if (character == "-") continue;
    if (character == "a") continue;
    if (character == "b") continue;
    break;
  }
  verseNumber = usfm.substr (0, i);
  verseNumber = filter_string_trim (verseNumber);
  return verseNumber;
}


// Takes a marker in the form of text only, like "id" or "add",
// and converts it into opening USFM, like "\id " or "\add ".
// Supports the embedded markup "+".
string usfm_get_opening_usfm (string text, bool embedded)
{
  string embed = embedded ? "+" : "";
  return "\\" + embed + text + " ";
}


// Takes a marker in the form of text only, like "add",
// and converts it into closing USFM, like "\add*".
// Supports the embedded markup "+".
string usfm_get_closing_usfm (string text, bool embedded)
{
  string embed = embedded ? "+" : "";
  return "\\" + embed + text + "*";
}


// This function extracts notes, e.g. cross references.
// $usfm: Where to extract from.
// $markers: Array of possible markers surrounding the notes.
// It returns an array of array ($offset, $note).
vector <UsfmNote> usfm_extract_notes (string usfm, const vector <string> & markers)
{
  set <string> openers;
  set <string> closers;
  for (string marker : markers) {
    openers.insert (usfm_get_opening_usfm (marker));
    closers.insert (usfm_get_closing_usfm (marker));
  }

  vector <string> markers_and_text = usfm_get_markers_and_text (usfm);

  vector <UsfmNote> notes;
  
  bool within_note = false;
  size_t running_offset = 0;
  string running_note = "";

  for (string item : markers_and_text) {
    
    if (openers.find (item) != openers.end ()) within_note = true;

    // Caclulate the offset in the main text. 
    // That means not to consider the length of the notes.
    if (!within_note) {
      running_offset += unicode_string_length (item);
    }
    
    if (within_note) running_note += item;

    if (closers.find (item) != closers.end()) {
      notes.push_back (UsfmNote (running_offset, running_note));
      running_note = "";
      within_note = false;
    }
  }
  
  return notes;
}


// This function removes the notes from the USFM.
// $usfm: Where to extract from.
// $markers: Array of markers surrounding the notes.
// It returns the USFM without the notes.
string usfm_remove_notes (string usfm, const vector <string> & markers)
{
  vector <UsfmNote> notes = usfm_extract_notes (usfm, markers);
  for (auto note : notes) {
    string search = note.data;
    usfm = filter_string_str_replace (search, "", usfm);
  }
  return usfm;
}


// This function inserts notes into USFM.
// It replaces $search with $replace in the $notes.
// $usfm: string where to insert the notes - it is assumed not to contain any notes yet.
// $notes: object of offsets => notes.
// $ratio: ratio to multiply the offsets with.
// It returns the updated USFM.
string usfm_insert_notes (string usfm, vector <UsfmNote> notes, float ratio)
{
  if (usfm.empty()) return usfm;
  if (notes.empty()) return usfm;
  
  vector <string> markers = {"x", "f", "fe"};
  
  // Temporally extract existing notes.
  vector <UsfmNote> existing = usfm_extract_notes (usfm, markers);

  // Work with USFM without notes so that a note will not be inserted in another note.
  usfm = usfm_remove_notes (usfm, markers);

  // Calculate insertion positions before actually inserting the notes to avoid nested notes placement.
  for (UsfmNote & note : notes) {
    int position = note.offset;
    position = convert_to_int (position * ratio);
    position = usfm_get_new_note_position (usfm, position, 0);
    note.offset = position;
  }

  // Add existing notes data.
  notes.insert (notes.end(), existing.begin(), existing.end());

  // Sort the notes such that the last one gets inserted first.
  // This way inserting happens from the end of the USFM towards the start.
  // Inserted text does not affect any insertion positions this way.
  int highest_position = 0;
  for (UsfmNote note : notes) {
    int position = note.offset;
    if (position > highest_position) {
      highest_position = position;
    }
  }
  vector <UsfmNote> notes2;
  for (int i = highest_position; i >= 0; i--) {
    for (UsfmNote note : notes) {
      if (note.offset == i) {
        notes2.push_back (note);
      }
    }
  }

  // Insert the notes into the USFM at the correct position.
  for (UsfmNote note : notes2) {
    int position = note.offset;
    string text = note.data;
    usfm.insert (position, text);
  }

  return usfm;
}


// This function moves a note one word back or forth in the USFM data.
// $usfm: The USFM data.
// $direction: The direction into which to move the note.
// $number: The note number. Starts at 1.
// Returns the updated USFM.
string usfm_move_note (string usfm, int direction, int number)
{
  vector <string> markers = {"x", "f", "fe"};
  vector <UsfmNote> notes = usfm_extract_notes (usfm, markers);
  int key = number - 1;
  if (key < 0) return usfm;
  if (key >= (int)notes.size()) return usfm;
  usfm = usfm_remove_notes (usfm, markers);
  int position = notes[key].offset;
  position += (direction * 3);
  position = usfm_get_new_note_position (usfm, position, direction);
  notes[key].offset = position;
  usfm = usfm_insert_notes (usfm, notes, 1);
  return usfm;
}


// This function gets a new position to insert a note.
// $usfm: The USFM code to work with.
// $position: Current position.
// $direction: Direction where to go to find the new position:
//   -1: Go back to the previous slot.
//    0: Attempt current position, else take next slot.
//    1: Go forward to the next slot.
// The positions take the string as UTF8.
size_t usfm_get_new_note_position (string usfm, size_t position, int direction)
{
  vector <string> words = filter_string_explode (usfm, ' ');

  size_t length = 0;

  vector <size_t> lengths;

  for (string word : words) {

    // Add length of item.
    length += unicode_string_length (word);
    
    // Check whether at opening marker.
    bool opening_marker = usfm_is_usfm_marker (word);
    if (opening_marker) {
      opening_marker = usfm_is_opening_marker (word);
    }

    // Don't create a slot for placing a note right after an opening marker.
    if (!opening_marker) lengths.push_back (length);

    // Add length of space.
    length++;
    
  }
  
  bool found = false;
  
  if (direction > 0) {
    // Take next position.
    for (size_t length : lengths) {
      if (found) continue;
      if (length > position) {
        position = length;
        found = true;
      }
    }
  } else if (direction < 0) {
    // Take previous position.
    vector <size_t> lengths_r (lengths.begin(), lengths.end());
    reverse (lengths_r.begin(), lengths_r.end());
    for (size_t length : lengths_r) {
      if (found) continue;
      if (length < position) {
        position = length;
        found = true;
      }
    }
  } else {
    // Take optimal position.
    for (size_t length : lengths) {
      if (found) continue;
      if (length >= position) {
        position = length;
        found = true;
      }
    }
  }
  
  if (!found) {
    position = unicode_string_length (usfm);
  }
  
  // Move a note to before punctuation.
  set <string> punctuation = {".", ",", ";", ":", "?", "!"};
  string character = unicode_string_substr (usfm, position - 1, 1);
  if (punctuation.find (character) != punctuation.end()) position--;

  return position;
}


// This function compares the $newtext with the $oldtext.
// It returns an empty string if the difference is below the limit set for the Bible.
// It returns a message specifying the difference if it exceeds that limit.
string usfm_save_is_safe (void * webserver_request, string oldtext, string newtext, bool chapter)
{
  // Two texts are equal: safe.
  if (newtext == oldtext) return "";

  Webserver_Request * request = (Webserver_Request *) webserver_request;

  // Allowed percentage difference.
  int allowed_percentage = request->database_config_user ()->getEditingAllowedDifferenceVerse ();
  if (chapter) allowed_percentage = request->database_config_user ()->getEditingAllowedDifferenceChapter ();

  // The length of the new text should not differ more than a set percentage from the old text.
  float existingLength = oldtext.length();
  float newLength = newtext.length ();
  int percentage = 100 * (newLength - existingLength) / existingLength;
  percentage = abs (percentage);
  if (percentage > 100) percentage = 100;
  if (percentage > allowed_percentage) {
    Database_Logs::log ("The text was not saved for safety reasons. The length differs " + convert_to_string (percentage) + "% from the existing text. Make smaller changes and save more often. Or relax the restriction in the Bible's editing settings.");
    Database_Logs::log (newtext);
    return translate ("Text length differs too much");
  }
  
  // The new text should be at least a set percentage similar to the old text.
  percentage = filter_diff_similarity (oldtext, newtext);
  if (percentage < (100 - allowed_percentage)) {
    Database_Logs::log ("The text was not saved for safety reasons. The new text is " + convert_to_string (percentage) + "% similar to the existing text. Make smaller changes and save more often. Or relax the restriction in the Bible's editing settings.");
    Database_Logs::log (newtext);
    return translate ("Text content differs too much");
  }
  
  // Safety checks have passed.
  return "";
}


// Function to safely store a chapter.
// It saves the chapter if the new USFM does not differ too much from the existing USFM.
// On success it returns an empty string.
// On failure it returns the reason of the failure.
// This function proves useful in cases that the text in the Bible editor gets corrupted
// due to human error.
// It also is useful in cases where the session is deleted from the server,
// where the text in the editors would get corrupted.
// It also is useful in view of an unstable connection between browser and server, to prevent data corruption.
string usfm_safely_store_chapter (void * webserver_request, string bible, int book, int chapter, string usfm)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  // Existing chapter contents.
  string existing = request->database_bibles()->getChapter (bible, book, chapter);
  
  // Bail out if the existing chapter equals the USFM to be saved.
  if (usfm == existing) return "";
  
  // Safety check.
  string message = usfm_save_is_safe (webserver_request, existing, usfm, true);
  if (!message.empty ()) return message;
  
  // Safety checks have passed: Save chapter.
  Bible_Logic::storeChapter (bible, book, chapter, usfm);
  return "";
}


// Function to safely store a verse.
// It saves the verse if the new USFM does not differ too much from the existing USFM.
// On success it returns an empty message.
// On failure it returns a message that specifies the reason why it could not be saved.
// This function proves useful in cases that the text in the Bible editor gets corrupted
// due to human error.
// It also is useful in cases where the session is deleted from the server,
// where the text in the editors would get corrupted.
// It also is useful in view of an unstable connection between browser and server, to prevent data corruption.
// It handles combined verses.
string usfm_safely_store_verse (void * webserver_request, string bible, int book, int chapter, int verse, string usfm)
{
  Webserver_Request * request = (Webserver_Request *) webserver_request;
  
  usfm = filter_string_trim (usfm);

  // Check that the USFM to be saved is for the correct verse.
  vector <int> save_verses = usfm_get_verse_numbers (usfm);
  if ((verse != 0) && !save_verses.empty ()) {
    save_verses.erase (save_verses.begin());
  }
  if (save_verses.empty ()) {
    Database_Logs::log ("The USFM contains no verse information: " + usfm);
    return translate ("Missing verse number");
  }
  if (!in_array (verse, save_verses)) {
    vector <string> vss;
    for (auto vs : save_verses) vss.push_back (convert_to_string (vs));
    Database_Logs::log ("The USFM contains verse(s) " + filter_string_implode (vss, " ") + " while it wants to save to verse " + convert_to_string (verse) + ": " + usfm);
    return translate ("Verse mismatch");
  }

  // Get the existing chapter USFM.
  string chapter_usfm = request->database_bibles()->getChapter (bible, book, chapter);
  
  // Get the existing USFM fragment for the verse to save.
  string existing_verse_usfm = usfm_get_verse_text (chapter_usfm, verse);
  existing_verse_usfm = filter_string_trim (existing_verse_usfm);
  
  // Check that there is a match between the existing verse numbers and the verse numbers to save.
  vector <int> existing_verses = usfm_get_verse_numbers (existing_verse_usfm);
  save_verses = usfm_get_verse_numbers (usfm);
  bool verses_match = true;
  if (save_verses.size () == existing_verses.size ()) {
    for (unsigned int i = 0; i < save_verses.size (); i++) {
      if (save_verses [i] != existing_verses [i]) verses_match = false;
    }
  } else {
    verses_match = false;
  }
  if (!verses_match) {
    vector <string> existing, save;
    for (auto vs : existing_verses) existing.push_back (convert_to_string (vs));
    for (auto vs : save_verses) save.push_back (convert_to_string (vs));
    Database_Logs::log ("The USFM contains verse(s) " + filter_string_implode (save, " ") + " which would overwrite a fragment that contains verse(s) " + filter_string_implode (existing, " ") + ": " + usfm);
    return translate ("Cannot overwrite another verse");
  }

  // Bail out if the new USFM is the same as the existing.
  if (usfm == existing_verse_usfm) {
    return "";
  }

  // Check maximum difference between new and existing USFM.
  string message = usfm_save_is_safe (webserver_request, existing_verse_usfm, usfm, false);
  if (!message.empty ()) return message;
  
  // Store the new verse USFM in the existing chapter USFM.
  size_t pos = chapter_usfm.find (existing_verse_usfm);
  size_t length = existing_verse_usfm.length ();
  chapter_usfm.erase (pos, length);
  chapter_usfm.insert (pos, usfm);
  
  // Safety checks have passed: Save chapter.
  Bible_Logic::storeChapter (bible, book, chapter, chapter_usfm);

  // Done: OK.
  return "";
}
