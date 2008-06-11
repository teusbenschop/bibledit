/*
 ** Copyright (©) 2003-2008 Teus Benschop.
 **  
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **  
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **  
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc.,  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **  
 */

#ifndef INCLUDED_MERGE_GUI_H
#define INCLUDED_MERGE_GUI_H

#include "libraries.h"
#include <glib.h>
#include <gtk/gtk.h>
#include "reference.h"
#include "editor.h"
#include "merge_utils.h"

class MergeGUI
{
public:
  MergeGUI(GtkWidget * notebook_vbox);
  ~MergeGUI();

  // Widgets.
public:
private:
  GtkWidget *my_notebook_vbox;
  GtkWidget *notebook1;
  // Merge widgets.
  GtkWidget *vbox1;
  GtkWidget *label6;
  GtkWidget *combobox_master;
  GtkWidget *label7;
  GtkWidget *combobox_edited;
  GtkWidget *label_info;
  GtkWidget *scrolledwindow1;
  GtkWidget *textview1;
  GtkWidget *hbox1;
  GtkWidget *button_previous;
  GtkWidget *alignment2;
  GtkWidget *hbox2;
  GtkWidget *image6;
  GtkWidget *label9;
  GtkWidget *button_merge;
  GtkWidget *alignment1;
  GtkWidget *hbox4;
  GtkWidget *image5;
  GtkWidget *label8;
  GtkWidget *button_next;
  GtkWidget *alignment3;
  GtkWidget *hbox3;
  GtkWidget *label10;
  GtkWidget *image8;
  // Approve widgets.
  GtkWidget *vbox2;
  GtkWidget *label_approve;
  GtkWidget *scrolledwindow2;
  GtkWidget *textview_approval;
  GtkWidget *button_ready;
  GtkWidget *alignment6;
  GtkWidget *hbox7;
  GtkWidget *image11;
  GtkWidget *label19;
  
  // Focus and activation.
public:
  bool has_focus();
  void give_focus();
  void set_active(bool value);
private:
  bool active;

  // Editor linking.
public:
  void set_focused_editor(Editor * editor);
  void set_visible_editors(const vector <Editor *>& editors);
  void editors_changed();
  GtkWidget * editors_get_text_button;
  ustring main_project_data;
  ustring edited_project_data;
private:
  Editor * focused_editor;
  vector <Editor *> visible_editors;
  guint editors_changed_event_id;
  static bool on_editors_changed_timeout(gpointer user_data);
  void on_editors_changed();

  // GUI loading.
public:
private:
  guint load_gui_event_id;
  void load_gui_delayer();
  static bool on_load_gui_timeout(gpointer user_data);
  void load_gui();

  // Master project.
public:
  ustring current_master_project;
private:
  void set_master_project();
  ustring previous_master_project;
  static void on_combobox_master_changed(GtkComboBox *combobox, gpointer user_data);
  void on_combobox_master();

  // Edited project.
public:
  ustring current_edited_project;
private:
  void set_edited_project();
  ustring previous_edited_project;
  static void on_combobox_edited_changed(GtkComboBox *combobox, gpointer user_data);
  void on_combobox_edited();

  // Next (previous) change.
public:
  unsigned int book;
  unsigned int chapter;
  GtkWidget * new_reference_button;
private:
  static void on_button_previous_clicked(GtkButton *button, gpointer user_data);
  static void on_button_next_clicked(GtkButton *button, gpointer user_data);
  void on_button_next_previous(bool next);
  bool cross_book_boundaries(bool next, unsigned int& cross_book, unsigned int& cross_chapter);

  // Merge.
public:
  GtkWidget * save_editors_button;
  GtkWidget * reload_editors_button;
private:
  static void on_button_merge_clicked(GtkButton *button, gpointer user_data);
  void on_button_merge();
  void merge_edited_into_master(bool approve);
  void copy_master_to_edited_chapter(unsigned int bk, unsigned int ch, bool gui);
  void copy_master_to_edited_all();

  // Split and join.
public:
private:
  ustring workingdirectory;
  ustring split_data(const ustring& data);
  ustring join_data(const ustring& data);
  gchar * new_line_indicator();
  gchar * verse_indicator();

  // Conflict resolution.
public:
private:
  ustring merge_conflicts_2_human_readable_text(const ustring& data);

  // Differences view.
public:
private:
  GtkTextBuffer * differencesbuffer;
  GtkTextTag *heavy_weight_tag;
  GtkTextTag *strike_through_tag;
  void show_comparison();

  // Changes approval.
public:
private:
  void approval_setup(const ustring& maindata, const ustring& mergedata);
  ustring approve_master_project;
  ustring approve_edited_project;
  unsigned int approve_book;
  unsigned int approve_chapter;
  ustring approve_master_file;
  ustring approve_merge_file;
  ustring approve_patch_file;
  GtkTextBuffer * approve_buffer;
  void approval_show_diff();
  static void on_button_approve_clicked(GtkButton *button, gpointer user_data);
  void approval_approve(GtkButton *button);
  vector <ApproveButton> approve_buttons;
  static void on_button_ready_clicked(GtkButton *button, gpointer user_data);
  void button_ready_clicked ();
};

#endif
