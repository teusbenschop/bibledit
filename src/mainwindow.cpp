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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/


#include <config.h>
#include "mainwindow.h"
#include "libraries.h"
#include "dialoglistview.h"
#include "dialogshowscript.h"
#include "constants.h"
#include "utilities.h"
#include "htmlbrowser.h"
#include "usfmtools.h"
#include "dialogreplace.h"
#include "dialoggotoreference.h"
#include <signal.h>
#include "bible.h"
#include "projectutils.h"
#include "dialogproject.h"
#include "usfm.h"
#include "dialogimporttext.h"
#include "dialogreplacing.h"
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include "dialogsearchspecial.h"
#include "referenceutils.h"
#include "references.h"
#include "pdfviewer.h"
#include "notes_utils.h"
#include "dialogfindnote.h"
#include "dialogimportnotes.h"
#include "date_time_utils.h"
#include "dialognotes.h"
#include "dialogentry.h"
#include "projectutils.h"
#include "directories.h"
#include "dialogcompare.h"
#include "export_utils.h"
#include "dialogprintprefs.h"
#include "dialogprintproject.h"
#include "printproject.h"
#include "xep.h"
#include "dialogformatter.h"
#include "compareutils.h"
#include "dialogexportnotes.h"
#include "dialogshownotes.h"
#include "dialogentry3.h"
#include "gwrappers.h"
#include "gtkwrappers.h"
#include "search_utils.h"
#include "combobox.h"
#include "scripturechecks.h"
#include "dialogrefexchange.h"
#include "dialogeditlist.h"
#include <sqlite3.h>
#include "sqlite_reader.h"
#include "highlight.h"
#include "fonts.h"
#include "dialogfont.h"
#include "stylesheetutils.h"
#include "keyboard.h"
#include "dialoginsertnote.h"
#include "dialogparallelbible.h"
#include "print_parallel_bible.h"
#include "editor.h"
#include "dialogscreenlayout.h"
#include "layout.h"
#include "dialogbook.h"
#include "books.h"
#include "screen.h"
#include "dialoglinecutter.h"
#include "dialogoutpost.h"
#include "dialognotestransfer.h"
#include "dialogchapternumber.h"
#include "versification.h"
#include "dialogmychecks.h"
#include "help.h"
#include "dialogoriginreferences.h"
#include "dialogtidy.h"
#include "dialognotesupdate.h"
#include "dialogbackup.h"
#include "backup.h"
#include "dialogviewsubversion.h"
#include "dialogsubversionsetup.h"
#include "dialogchanges.h"
#include "dialogrevert.h"
#include "resource_utils.h"
#include "dialogeditnote.h"
#include "dialogwordlist.h"
#include "dialogfontcolor.h"
#include "color.h"
#include "dialognewstylesheet.h"
#include "settings.h"
#include "ipc.h"
#include "dialogfeatures.h"
#include "password.h"
#include "gui_features.h"
#include "dialogprintreferences.h"
#include "print_parallel_references.h"
#include "dialogfixmarkers.h"
#include "dialogtextreplacement.h"
#include "temporal.h"
#include "parallel_passages.h"
#include "dialogpdfviewer.h"
#include "dialogviewusfm.h"
#include "dialoginserttable.h"
#include "tiny_utilities.h"
#include "xep.h"
#include "hyphenate.h"
#include "dialogreportingsetup.h"
#include "dialogeditstatus.h"
#include "dialogviewstatus.h"
#include "planning.h"
#include "dialogplanningsetup.h"
#include "dialoginterfaceprefs.h"
#include "dialognewresource.h"
#include "shutdown.h"
#include "dialogfilters.h"
#include "dialogradiobutton.h"


/*
|
|
|
|
|
Construction and destruction
|
|
|
|
|
 */


MainWindow::MainWindow (unsigned long xembed):
navigation (0),
selected_reference (0, 0, ""),
bibletime (true),
httpd (0)
{
  // Set some pointers to NULL.  
  // To save memory, we only create the object when actually needed.
  keytermsgui = NULL;
  outline = NULL;
  note_editor = NULL;
  resourcesgui = NULL;
  editorsgui = NULL;

  // Initialize some variables.
  notes_redisplay_source_id = 0;
  displayprojectnotes = NULL;
  git_reopen_project = false;
  
  // Gui Features object.
  GuiFeatures guifeatures (0);
  
  accel_group = gtk_accel_group_new ();

  if (xembed) {
    mainwindow = gtk_plug_new (GdkNativeWindow (xembed));
  } else {
    mainwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  }
  set_titlebar ("");
  gtk_window_set_default_size (GTK_WINDOW (mainwindow), 800, 600);
  gtk_window_set_default_icon_from_file (gw_build_filename (directories_get_package_data (), "bibledit.xpm").c_str(), NULL);
  gtk_window_set_gravity (GTK_WINDOW (mainwindow), GDK_GRAVITY_STATIC);
  
  g_set_application_name ("Bibledit");

  // Pointer to the settings.
  extern Settings * settings;
  
  // Start Outpost.
  windowsoutpost = new WindowsOutpost (true);
  if (settings->genconfig.use_outpost_get ()) windowsoutpost->Start ();

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (mainwindow), vbox1);

  menubar1 = gtk_menu_bar_new ();
  gtk_widget_show (menubar1);
  gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);

  menuitem_file = gtk_menu_item_new_with_mnemonic ("_File");
  gtk_widget_show (menuitem_file);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_file);

  menuitem_file_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_file), menuitem_file_menu);

  file_project = gtk_image_menu_item_new_with_mnemonic ("_Project");
  gtk_widget_show (file_project);
  gtk_container_add (GTK_CONTAINER (menuitem_file_menu), file_project);

  image463 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image463);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_project), image463);

  file_project_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_project), file_project_menu);
  
  new1 = NULL;
  open1 = NULL;
  close1 = NULL;
  delete1 = NULL;
  if (guifeatures.project_management ()) {

    new1 = gtk_image_menu_item_new_with_mnemonic ("_New");
    gtk_widget_show (new1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), new1);

    image903 = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image903);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (new1), image903);

    open1 = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_widget_show (open1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), open1);

    close1 = gtk_image_menu_item_new_from_stock ("gtk-close", accel_group);
    gtk_widget_show (close1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), close1);

    delete1 = gtk_image_menu_item_new_from_stock ("gtk-delete", accel_group);
    gtk_widget_show (delete1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), delete1);
    
  }

  print_project = NULL;
  if (guifeatures.printing ()) {
    
    print_project = gtk_image_menu_item_new_with_mnemonic ("_Print");
    gtk_widget_show (print_project);
    gtk_container_add (GTK_CONTAINER (file_project_menu), print_project);

    image805 = gtk_image_new_from_stock ("gtk-print", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image805);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (print_project), image805);
    
  }

  properties1 = NULL;
  import1 = NULL;
  export_project = NULL;
  export_project_menu = NULL;
  export_usfm_files = NULL;
  export_zipped_unified_standard_format_markers1 = NULL;
  to_bibleworks_version_database_compiler = NULL;
  export_to_sword_module = NULL;
  export_opendocument = NULL;
  copy_project_to = NULL;
  compare_with1 = NULL;
  parallel_bible = NULL;
  if (guifeatures.project_management ()) {
    
    properties1 = gtk_image_menu_item_new_with_mnemonic ("P_roperties");
    gtk_widget_show (properties1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), properties1);

    image4995 = gtk_image_new_from_stock ("gtk-properties", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image4995);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (properties1), image4995);

    import1 = gtk_image_menu_item_new_with_mnemonic ("_Import");
    gtk_widget_show (import1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), import1);

    image464 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image464);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (import1), image464);

    export_project = gtk_image_menu_item_new_with_mnemonic ("_Export");
    gtk_widget_show (export_project);
    gtk_container_add (GTK_CONTAINER (file_project_menu), export_project);

    image3298 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image3298);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_project), image3298);

    export_project_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (export_project), export_project_menu);

    export_usfm_files = gtk_image_menu_item_new_with_mnemonic ("_Unified Standard Format Markers files");
    gtk_widget_show (export_usfm_files);
    gtk_container_add (GTK_CONTAINER (export_project_menu), export_usfm_files);

    image12814 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image12814);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_usfm_files), image12814);

    export_zipped_unified_standard_format_markers1 = gtk_image_menu_item_new_with_mnemonic ("_Zipped Unified Standard Format Markers");
    gtk_widget_show (export_zipped_unified_standard_format_markers1);
    gtk_container_add (GTK_CONTAINER (export_project_menu), export_zipped_unified_standard_format_markers1);

    image17639 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image17639);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_zipped_unified_standard_format_markers1), image17639);

    to_bibleworks_version_database_compiler = gtk_image_menu_item_new_with_mnemonic ("_BibleWorks Version Database Compiler");
    gtk_widget_show (to_bibleworks_version_database_compiler);
    gtk_container_add (GTK_CONTAINER (export_project_menu), to_bibleworks_version_database_compiler);

    image3299 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image3299);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (to_bibleworks_version_database_compiler), image3299);

    export_to_sword_module = gtk_image_menu_item_new_with_mnemonic ("_SWORD module");
    gtk_widget_show (export_to_sword_module);
    gtk_container_add (GTK_CONTAINER (export_project_menu), export_to_sword_module);

    image11392 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image11392);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_to_sword_module), image11392);

    export_opendocument = gtk_image_menu_item_new_with_mnemonic ("_OpenDocument");
    gtk_widget_show (export_opendocument);
    gtk_container_add (GTK_CONTAINER (export_project_menu), export_opendocument);

    image15162 = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image15162);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_opendocument), image15162);

    copy_project_to = gtk_image_menu_item_new_with_mnemonic ("Cop_y to");
    gtk_widget_show (copy_project_to);
    gtk_container_add (GTK_CONTAINER (file_project_menu), copy_project_to);

    image2688 = gtk_image_new_from_stock ("gtk-copy", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image2688);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (copy_project_to), image2688);

    compare_with1 = gtk_image_menu_item_new_with_mnemonic ("Co_mpare with");
    gtk_widget_show (compare_with1);
    gtk_container_add (GTK_CONTAINER (file_project_menu), compare_with1);

    image2764 = gtk_image_new_from_stock ("gtk-zoom-fit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image2764);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (compare_with1), image2764);

    parallel_bible = gtk_image_menu_item_new_with_mnemonic ("P_arallel Bible");
    gtk_widget_show (parallel_bible);
    gtk_container_add (GTK_CONTAINER (file_project_menu), parallel_bible);

    image11566 = gtk_image_new_from_stock ("gtk-copy", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image11566);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (parallel_bible), image11566);

  }

  project_backup = gtk_image_menu_item_new_with_mnemonic ("_Backup");
  gtk_widget_show (project_backup);
  gtk_container_add (GTK_CONTAINER (file_project_menu), project_backup);

  image18535 = gtk_image_new_from_stock ("gtk-floppy", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image18535);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (project_backup), image18535);

  project_backup_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (project_backup), project_backup_menu);

  project_backup_incremental = gtk_image_menu_item_new_with_mnemonic ("_Incremental");
  gtk_widget_show (project_backup_incremental);
  gtk_container_add (GTK_CONTAINER (project_backup_menu), project_backup_incremental);

  image18536 = gtk_image_new_from_stock ("gtk-floppy", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image18536);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (project_backup_incremental), image18536);

  project_backup_flexible = gtk_image_menu_item_new_with_mnemonic ("_Flexible");
  gtk_widget_show (project_backup_flexible);
  gtk_container_add (GTK_CONTAINER (project_backup_menu), project_backup_flexible);

  image18537 = gtk_image_new_from_stock ("gtk-cdrom", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image18537);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (project_backup_flexible), image18537);

  project_changes = NULL;
  if (guifeatures.project_management ()) {

    project_changes = gtk_image_menu_item_new_with_mnemonic ("C_hanges");
    gtk_widget_show (project_changes);
    gtk_container_add (GTK_CONTAINER (file_project_menu), project_changes);

    image19115 = gtk_image_new_from_stock ("gtk-zoom-fit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image19115);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (project_changes), image19115);
    
  }

  file_projects_merge = gtk_image_menu_item_new_with_mnemonic ("Mer_ge");
  gtk_widget_show (file_projects_merge);
  gtk_container_add (GTK_CONTAINER (file_project_menu), file_projects_merge);

  image28085 = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image28085);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_projects_merge), image28085);

  file_references = NULL;
  if (guifeatures.references_management () || guifeatures.printing ()) {
    
    file_references = gtk_image_menu_item_new_with_mnemonic ("_References");
    gtk_widget_show (file_references);
    gtk_container_add (GTK_CONTAINER (menuitem_file_menu), file_references);

    image465 = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image465);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_references), image465);

    file_references_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_references), file_references_menu);
    
  }

  if (guifeatures.references_management ()) {
    
    open_references1 = gtk_image_menu_item_new_with_mnemonic ("_Open");
    gtk_widget_show (open_references1);
    gtk_container_add (GTK_CONTAINER (file_references_menu), open_references1);

    image466 = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image466);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (open_references1), image466);

    references_save_as = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel_group);
    gtk_widget_show (references_save_as);
    gtk_container_add (GTK_CONTAINER (file_references_menu), references_save_as);
    
  }

  print_references = NULL;
  if (guifeatures.printing ()) {
    
    print_references = gtk_image_menu_item_new_with_mnemonic ("_Print");
    gtk_widget_show (print_references);
    gtk_container_add (GTK_CONTAINER (file_references_menu), print_references);
    gtk_widget_add_accelerator (print_references, "activate", accel_group,
                                GDK_P, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image759 = gtk_image_new_from_stock ("gtk-print", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image759);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (print_references), image759);
    
  }

  if (guifeatures.references_management ()) {
    
    close_references = gtk_image_menu_item_new_with_mnemonic ("D_elete all");
    gtk_widget_show (close_references);
    gtk_container_add (GTK_CONTAINER (file_references_menu), close_references);

    image468 = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image468);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (close_references), image468);

    delete_references = gtk_image_menu_item_new_with_mnemonic ("_Delete");
    gtk_widget_show (delete_references);
    gtk_container_add (GTK_CONTAINER (file_references_menu), delete_references);

    image469 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image469);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (delete_references), image469);

    reference_hide = gtk_image_menu_item_new_with_mnemonic ("_Hide from now on");
    gtk_widget_show (reference_hide);
    gtk_container_add (GTK_CONTAINER (file_references_menu), reference_hide);

    image6483 = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6483);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (reference_hide), image6483);
    
  }

  style = NULL;
  if (guifeatures.styles ()) {
    
    style = gtk_image_menu_item_new_with_mnemonic ("_Styles");
    gtk_widget_show (style);
    gtk_container_add (GTK_CONTAINER (menuitem_file_menu), style);

    image10735 = gtk_image_new_from_stock ("gtk-print-preview", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10735);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (style), image10735);
    
  }

  GtkWidget *style_menu = NULL;
  if (guifeatures.styles ()) {
    
  style_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (style), style_menu);
    
  }

  GtkWidget *stylesheets_expand_all = NULL;
  if (guifeatures.styles ()) {
    
    stylesheets_expand_all = gtk_image_menu_item_new_with_mnemonic ("_Expand all");
    gtk_widget_show (stylesheets_expand_all);
    gtk_container_add (GTK_CONTAINER (style_menu), stylesheets_expand_all);

    GtkWidget *image10736;
    image10736 = gtk_image_new_from_stock ("gtk-add", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10736);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_expand_all), image10736);
    
  }

  GtkWidget *stylesheets_collapse_all = NULL;
  if (guifeatures.styles ()) {
  
    stylesheets_collapse_all = gtk_image_menu_item_new_with_mnemonic ("_Collapse all");
    gtk_widget_show (stylesheets_collapse_all);
    gtk_container_add (GTK_CONTAINER (style_menu), stylesheets_collapse_all);

    GtkWidget *image10737;
    image10737 = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10737);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_collapse_all), image10737);
    
  }

  GtkWidget *style_insert = NULL;
  if (guifeatures.styles ()) {
    
    style_insert = gtk_image_menu_item_new_with_mnemonic ("_Insert");
    gtk_widget_show (style_insert);
    gtk_container_add (GTK_CONTAINER (style_menu), style_insert);

    GtkWidget *image10738;
    image10738 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10738);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (style_insert), image10738);
    
  }

  GtkWidget *stylesheet_edit_mode = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheet_edit_mode = gtk_check_menu_item_new_with_mnemonic ("Edit _mode");
    gtk_widget_show (stylesheet_edit_mode);
    gtk_container_add (GTK_CONTAINER (style_menu), stylesheet_edit_mode);
    
  }

  GtkWidget *style_new = NULL;
  if (guifeatures.styles_management ()) {
    
    style_new = gtk_image_menu_item_new_with_mnemonic ("_New");
    gtk_widget_show (style_new);
    gtk_container_add (GTK_CONTAINER (style_menu), style_new);
    
    GtkWidget *image10739;
    image10739 = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10739);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (style_new), image10739);
    
  }

  GtkWidget *style_properties = NULL;
  if (guifeatures.styles_management ()) {
    
    style_properties = gtk_image_menu_item_new_from_stock ("gtk-properties", accel_group);
    gtk_widget_show (style_properties);
    gtk_container_add (GTK_CONTAINER (style_menu), style_properties);
    
  }

  GtkWidget *style_delete = NULL;
  if (guifeatures.styles_management ()) {
    
    style_delete = gtk_image_menu_item_new_from_stock ("gtk-delete", accel_group);
    gtk_widget_show (style_delete);
    gtk_container_add (GTK_CONTAINER (style_menu), style_delete);
    
  }

  GtkWidget *menu_stylesheet = NULL;
  if (guifeatures.styles_management ()) {
    
    menu_stylesheet = gtk_image_menu_item_new_with_mnemonic ("_Stylesheet");
    gtk_widget_show (menu_stylesheet);
    gtk_container_add (GTK_CONTAINER (style_menu), menu_stylesheet);

    GtkWidget *image10740;
    image10740 = gtk_image_new_from_stock ("gtk-print-preview", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10740);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_stylesheet), image10740);
    
  }

  GtkWidget *menu_stylesheet_menu = NULL;
  if (guifeatures.styles_management ()) {
    
    menu_stylesheet_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_stylesheet), menu_stylesheet_menu);
    
  }

  GtkWidget *stylesheet_switch = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheet_switch = gtk_image_menu_item_new_with_mnemonic ("_Switch");
    gtk_widget_show (stylesheet_switch);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheet_switch);

    GtkWidget *image10741;
    image10741 = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10741);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheet_switch), image10741);
    
  }

  GtkWidget *stylesheets_new = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheets_new = gtk_image_menu_item_new_with_mnemonic ("_New");
    gtk_widget_show (stylesheets_new);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheets_new);

    GtkWidget *image10742;
    image10742 = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10742);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_new), image10742);
    
  }

  GtkWidget *stylesheets_delete = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheets_delete = gtk_image_menu_item_new_with_mnemonic ("_Delete");
    gtk_widget_show (stylesheets_delete);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheets_delete);

    GtkWidget *image10743;
    image10743 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10743);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_delete), image10743);
    
  }

  GtkWidget *stylesheets_rename = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheets_rename = gtk_image_menu_item_new_with_mnemonic ("_Rename");
    gtk_widget_show (stylesheets_rename);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheets_rename);

    GtkWidget *image10744;
    image10744 = gtk_image_new_from_stock ("gtk-redo", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10744);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_rename), image10744);
    
  }

  GtkWidget *stylesheets_import = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheets_import = gtk_image_menu_item_new_with_mnemonic ("_Import");
    gtk_widget_show (stylesheets_import);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheets_import);

    GtkWidget *image10745;
    image10745 = gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10745);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_import), image10745);
    
  }

  GtkWidget *stylesheets_export = NULL;
  if (guifeatures.styles_management ()) {
    
    stylesheets_export = gtk_image_menu_item_new_with_mnemonic ("_Export");
    gtk_widget_show (stylesheets_export);
    gtk_container_add (GTK_CONTAINER (menu_stylesheet_menu), stylesheets_export);

    GtkWidget *image10746;
    image10746 = gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image10746);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (stylesheets_export), image10746);

  }
  
  notes2 = NULL;
  if (guifeatures.project_notes_management ()) {
    
    notes2 = gtk_image_menu_item_new_with_mnemonic ("Project _notes");
    gtk_widget_show (notes2);
    gtk_container_add (GTK_CONTAINER (menuitem_file_menu), notes2);

    image936 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image936);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (notes2), image936);

    notes2_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (notes2), notes2_menu);

    new_note = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
    gtk_widget_show (new_note);
    gtk_container_add (GTK_CONTAINER (notes2_menu), new_note);

    delete_note = gtk_image_menu_item_new_with_mnemonic ("_Delete");
    gtk_widget_show (delete_note);
    gtk_container_add (GTK_CONTAINER (notes2_menu), delete_note);

    image963 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image963);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (delete_note), image963);

    import_notes = gtk_image_menu_item_new_with_mnemonic ("_Import");
    gtk_widget_show (import_notes);
    gtk_container_add (GTK_CONTAINER (notes2_menu), import_notes);

    image1455 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image1455);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (import_notes), image1455);

    export_notes = gtk_image_menu_item_new_with_mnemonic ("_Export");
    gtk_widget_show (export_notes);
    gtk_container_add (GTK_CONTAINER (notes2_menu), export_notes);

    image4068 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image4068);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (export_notes), image4068);
    
  }

  file_resources = gtk_image_menu_item_new_with_mnemonic ("R_esources");
  gtk_widget_show (file_resources);
  gtk_container_add (GTK_CONTAINER (menuitem_file_menu), file_resources);

  image27365 = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27365);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources), image27365);

  file_resources_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file_resources), file_resources_menu);

  file_resources_open = gtk_image_menu_item_new_with_mnemonic ("_Open");
  gtk_widget_show (file_resources_open);
  gtk_container_add (GTK_CONTAINER (file_resources_menu), file_resources_open);

  image27366 = gtk_image_new_from_stock ("gtk-open", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27366);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources_open), image27366);

  file_resources_close = gtk_image_menu_item_new_with_mnemonic ("_Close");
  gtk_widget_show (file_resources_close);
  gtk_container_add (GTK_CONTAINER (file_resources_menu), file_resources_close);

  image27367 = gtk_image_new_from_stock ("gtk-close", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27367);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources_close), image27367);

  file_resources_new = gtk_image_menu_item_new_with_mnemonic ("_New");
  gtk_widget_show (file_resources_new);
  gtk_container_add (GTK_CONTAINER (file_resources_menu), file_resources_new);

  image27514 = gtk_image_new_from_stock ("gtk-new", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27514);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources_new), image27514);

  file_resources_edit = gtk_image_menu_item_new_with_mnemonic ("_Edit");
  gtk_widget_show (file_resources_edit);
  gtk_container_add (GTK_CONTAINER (file_resources_menu), file_resources_edit);

  image27515 = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27515);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources_edit), image27515);

  file_resources_delete = gtk_image_menu_item_new_with_mnemonic ("_Delete");
  gtk_widget_show (file_resources_delete);
  gtk_container_add (GTK_CONTAINER (file_resources_menu), file_resources_delete);

  image27664 = gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image27664);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (file_resources_delete), image27664);

  quit1 = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
  gtk_widget_show (quit1);
  gtk_container_add (GTK_CONTAINER (menuitem_file_menu), quit1);

  menuitem_edit = gtk_menu_item_new_with_mnemonic ("_Edit");
  gtk_widget_show (menuitem_edit);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_edit);

  menuitem_edit_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_edit), menuitem_edit_menu);

  cut1 = gtk_image_menu_item_new_from_stock ("gtk-cut", accel_group);
  gtk_widget_show (cut1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), cut1);

  copy1 = gtk_image_menu_item_new_from_stock ("gtk-copy", accel_group);
  gtk_widget_show (copy1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), copy1);

  copy_without_formatting = gtk_image_menu_item_new_with_mnemonic ("Copy _without formatting");
  gtk_widget_show (copy_without_formatting);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), copy_without_formatting);
  gtk_widget_add_accelerator (copy_without_formatting, "activate", accel_group,
                              GDK_C, (GdkModifierType) (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                              GTK_ACCEL_VISIBLE);

  image18220 = gtk_image_new_from_stock ("gtk-copy", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image18220);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (copy_without_formatting), image18220);

  paste1 = gtk_image_menu_item_new_from_stock ("gtk-paste", accel_group);
  gtk_widget_show (paste1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), paste1);

  separator2 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator2);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator2);
  gtk_widget_set_sensitive (separator2, FALSE);

  undo1 = gtk_image_menu_item_new_with_mnemonic ("_Undo");
  gtk_widget_show (undo1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), undo1);
  gtk_widget_add_accelerator (undo1, "activate", accel_group, GDK_Z, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

  image295 = gtk_image_new_from_stock ("gtk-undo", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image295);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (undo1), image295);

  redo1 = gtk_image_menu_item_new_with_mnemonic ("_Redo");
  gtk_widget_show (redo1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), redo1);
  gtk_widget_add_accelerator (redo1, "activate", accel_group, GDK_Z,
                              GdkModifierType (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                              GTK_ACCEL_VISIBLE);

  image296 = gtk_image_new_from_stock ("gtk-redo", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image296);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (redo1), image296);

  separator4 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator4);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator4);
  gtk_widget_set_sensitive (separator4, FALSE);

  if (guifeatures.references_and_find ()) {
    
    find1 = gtk_image_menu_item_new_from_stock ("gtk-find", accel_group);
    gtk_widget_show (find1);
    gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), find1);
    
  }

  if (guifeatures.replace ()) {
    
    find_and_replace1 = gtk_image_menu_item_new_from_stock ("gtk-find-and-replace", accel_group);
    gtk_widget_show (find_and_replace1);
    gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), find_and_replace1);
    
  }

  find_in_notes1 = gtk_image_menu_item_new_with_mnemonic ("Find in Project _notes");
  gtk_widget_show (find_in_notes1);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), find_in_notes1);

  image1430 = gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image1430);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (find_in_notes1), image1430);

  separator7 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator7);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator7);
  gtk_widget_set_sensitive (separator7, FALSE);

  get_references_from_note = gtk_image_menu_item_new_with_mnemonic ("_Get references from project note");
  gtk_widget_show (get_references_from_note);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), get_references_from_note);

  image3158 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image3158);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (get_references_from_note), image3158);

  separator15 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator15);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator15);
  gtk_widget_set_sensitive (separator15, FALSE);

  edit_revert = gtk_image_menu_item_new_with_mnemonic ("Re_vert");
  gtk_widget_show (edit_revert);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), edit_revert);

  image19262 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image19262);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (edit_revert), image19262);

  separator17 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator17);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator17);
  gtk_widget_set_sensitive (separator17, FALSE);

  edit_bible_note = gtk_image_menu_item_new_with_mnemonic ("_Bible note");
  gtk_widget_show (edit_bible_note);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), edit_bible_note);

  image20483 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20483);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (edit_bible_note), image20483);

  separator21 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator21);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), separator21);
  gtk_widget_set_sensitive (separator21, FALSE);

  editstatus = gtk_image_menu_item_new_with_mnemonic ("St_atus");
  gtk_widget_show (editstatus);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), editstatus);

  image25815 = gtk_image_new_from_stock ("gtk-about", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image25815);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (editstatus), image25815);

  edit_planning = gtk_image_menu_item_new_with_mnemonic ("P_lanning");
  gtk_widget_show (edit_planning);
  gtk_container_add (GTK_CONTAINER (menuitem_edit_menu), edit_planning);

  image26801 = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image26801);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (edit_planning), image26801);

  menuitem_view = gtk_menu_item_new_with_mnemonic ("_View");
  gtk_widget_show (menuitem_view);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_view);

  menuitem_view_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_view), menuitem_view_menu);

  view_font = gtk_image_menu_item_new_with_mnemonic ("_Font & Color");
  gtk_widget_show (view_font);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), view_font);

  image20234 = gtk_image_new_from_stock ("gtk-select-font", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20234);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_font), image20234);

  view_font_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (view_font), view_font_menu);

  view_text_font = gtk_image_menu_item_new_with_mnemonic ("_Text");
  gtk_widget_show (view_text_font);
  gtk_container_add (GTK_CONTAINER (view_font_menu), view_text_font);

  image20235 = gtk_image_new_from_stock ("gtk-select-font", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20235);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_text_font), image20235);

  view_notes_font = gtk_image_menu_item_new_with_mnemonic ("_Notes");
  gtk_widget_show (view_notes_font);
  gtk_container_add (GTK_CONTAINER (view_font_menu), view_notes_font);

  image20236 = gtk_image_new_from_stock ("gtk-select-font", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20236);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_notes_font), image20236);

  printer_font = gtk_image_menu_item_new_with_mnemonic ("_Printer");
  gtk_widget_show (printer_font);
  gtk_container_add (GTK_CONTAINER (view_font_menu), printer_font);

  image20237 = gtk_image_new_from_stock ("gtk-select-font", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20237);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (printer_font), image20237);

  if (guifeatures.project_notes ()) {
    
    viewnotes = gtk_image_menu_item_new_with_mnemonic ("Project _notes");
    gtk_widget_show (viewnotes);
    gtk_container_add (GTK_CONTAINER (menuitem_view_menu), viewnotes);

    image2627 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image2627);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (viewnotes), image2627);
    
  }

  screen_layout = gtk_image_menu_item_new_with_mnemonic ("Screen _layout");
  gtk_widget_show (screen_layout);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), screen_layout);

  image11944 = gtk_image_new_from_stock ("gtk-justify-fill", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image11944);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (screen_layout), image11944);

  view_git_tasks = gtk_image_menu_item_new_with_mnemonic ("_Git tasks");
  gtk_widget_show (view_git_tasks);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), view_git_tasks);

  image18685 = gtk_image_new_from_stock ("gtk-connect", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image18685);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_git_tasks), image18685);

  parallel_passages1 = gtk_check_menu_item_new_with_mnemonic ("_Parallel passages");
  gtk_widget_show (parallel_passages1);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), parallel_passages1);

  view_usfm_code = gtk_image_menu_item_new_with_mnemonic ("USFM _code");
  gtk_widget_show (view_usfm_code);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), view_usfm_code);

  image25006 = gtk_image_new_from_stock ("gtk-properties", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image25006);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_usfm_code), image25006);

  view_status = gtk_image_menu_item_new_with_mnemonic ("S_tatus");
  gtk_widget_show (view_status);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), view_status);

  image25963 = gtk_image_new_from_stock ("gtk-apply", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image25963);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_status), image25963);

  view_planning = gtk_image_menu_item_new_with_mnemonic ("Pl_anning");
  gtk_widget_show (view_planning);
  gtk_container_add (GTK_CONTAINER (menuitem_view_menu), view_planning);

  image26812 = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image26812);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (view_planning), image26812);

  insert1 = gtk_menu_item_new_with_mnemonic ("_Insert");
  gtk_widget_show (insert1);
  gtk_container_add (GTK_CONTAINER (menubar1), insert1);

  insert1_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (insert1), insert1_menu);

  standard_text_1 = NULL;
  standard_text_2 = NULL;
  standard_text_3 = NULL;
  standard_text_4 = NULL;
  current_reference1 = NULL;
  
  if (guifeatures.project_notes ()) {
    
    standard_text_1 = gtk_image_menu_item_new_with_mnemonic ("Standard text _1");
    gtk_widget_show (standard_text_1);
    gtk_container_add (GTK_CONTAINER (insert1_menu), standard_text_1);
    gtk_widget_add_accelerator (standard_text_1, "activate", accel_group,
                                GDK_1, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image1963 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image1963);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (standard_text_1), image1963);

    standard_text_2 = gtk_image_menu_item_new_with_mnemonic ("Standard text _2");
    gtk_widget_show (standard_text_2);
    gtk_container_add (GTK_CONTAINER (insert1_menu), standard_text_2);
    gtk_widget_add_accelerator (standard_text_2, "activate", accel_group,
                                GDK_2, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image1964 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image1964);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (standard_text_2), image1964);

    standard_text_3 = gtk_image_menu_item_new_with_mnemonic ("Standard text _3");
    gtk_widget_show (standard_text_3);
    gtk_container_add (GTK_CONTAINER (insert1_menu), standard_text_3);
    gtk_widget_add_accelerator (standard_text_3, "activate", accel_group,
                                GDK_3, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image1965 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image1965);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (standard_text_3), image1965);

    standard_text_4 = gtk_image_menu_item_new_with_mnemonic ("Standard text _4"); 
    gtk_widget_show (standard_text_4);
    gtk_container_add (GTK_CONTAINER (insert1_menu), standard_text_4);
    gtk_widget_add_accelerator (standard_text_4, "activate", accel_group,
                                GDK_4, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image1966 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image1966);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (standard_text_4), image1966);

    separator9 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator9);
    gtk_container_add (GTK_CONTAINER (insert1_menu), separator9);
    gtk_widget_set_sensitive (separator9, FALSE);

    current_reference1 = gtk_image_menu_item_new_with_mnemonic ("_Current reference");
    gtk_widget_show (current_reference1);
    gtk_container_add (GTK_CONTAINER (insert1_menu), current_reference1);

    image3797 = gtk_image_new_from_stock ("gtk-paste", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image3797);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (current_reference1), image3797);

    separator20 = gtk_separator_menu_item_new ();
    gtk_widget_show (separator20);
    gtk_container_add (GTK_CONTAINER (insert1_menu), separator20);
    gtk_widget_set_sensitive (separator20, FALSE);

  }
  
  insert_special_character = gtk_image_menu_item_new_with_mnemonic ("_Special character");
  gtk_widget_show (insert_special_character);
  gtk_container_add (GTK_CONTAINER (insert1_menu), insert_special_character);

  image25281 = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image25281);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (insert_special_character), image25281);

  menuitem_goto = gtk_menu_item_new_with_mnemonic ("_Goto");
  gtk_widget_show (menuitem_goto);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_goto);

  menuitem_goto_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_goto), menuitem_goto_menu);

  next_verse1 = gtk_image_menu_item_new_with_mnemonic ("Next verse");
  gtk_widget_show (next_verse1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), next_verse1);
  gtk_widget_add_accelerator (next_verse1, "activate", accel_group, 
                              GDK_Down, (GdkModifierType) GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);
  // The GdkModifierType has to be cast to a GdkModifierType.

  image95 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image95);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (next_verse1), image95);

  previous_verse1 = gtk_image_menu_item_new_with_mnemonic ("Previous verse");
  gtk_widget_show (previous_verse1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), previous_verse1);
  gtk_widget_add_accelerator (previous_verse1, "activate", accel_group, 
                              GDK_Up, (GdkModifierType) GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  image96 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image96);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (previous_verse1), image96);

  next_chapter1 = gtk_image_menu_item_new_with_mnemonic ("Next chapter");
  gtk_widget_show (next_chapter1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), next_chapter1);
  gtk_widget_add_accelerator (next_chapter1, "activate", accel_group, 
                              GDK_Page_Down, (GdkModifierType) GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  image97 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image97);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (next_chapter1), image97);

  previous_chapter1 = gtk_image_menu_item_new_with_mnemonic ("Previous chapter");
  gtk_widget_show (previous_chapter1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), previous_chapter1);
  gtk_widget_add_accelerator (previous_chapter1, "activate", accel_group, 
                              GDK_Page_Up, (GdkModifierType) GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  image98 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image98);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (previous_chapter1), image98);

  next_book1 = gtk_image_menu_item_new_with_mnemonic ("Next book");
  gtk_widget_show (next_book1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), next_book1);
  gtk_widget_add_accelerator (next_book1, "activate", accel_group, 
                              GDK_Page_Down, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK),
                              GTK_ACCEL_VISIBLE);

  image99 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image99);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (next_book1), image99);

  previous_book1 = gtk_image_menu_item_new_with_mnemonic ("Previous book");
  gtk_widget_show (previous_book1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), previous_book1);
  gtk_widget_add_accelerator (previous_book1, "activate", accel_group, 
                              GDK_Page_Up, (GdkModifierType) (GDK_CONTROL_MASK | GDK_MOD1_MASK),
                              GTK_ACCEL_VISIBLE);

  image100 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image100);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (previous_book1), image100);
  
  next_reference_in_history1 = gtk_image_menu_item_new_with_mnemonic ("Next reference in history");
  gtk_widget_show (next_reference_in_history1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), next_reference_in_history1);
  gtk_widget_add_accelerator (next_reference_in_history1, "activate", accel_group,
                              GDK_Right, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  image5687 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image5687);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (next_reference_in_history1), image5687);

  previous_reference_in_history1 = gtk_image_menu_item_new_with_mnemonic ("Previous reference in history");
  gtk_widget_show (previous_reference_in_history1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), previous_reference_in_history1);
  gtk_widget_add_accelerator (previous_reference_in_history1, "activate", accel_group,
                              GDK_Left, GDK_MOD1_MASK,
                              GTK_ACCEL_VISIBLE);

  image5688 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image5688);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (previous_reference_in_history1), image5688);

  separator18 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator18);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), separator18);

  if (guifeatures.references_and_find ()) {

    next_reference1 = gtk_image_menu_item_new_with_mnemonic ("Next reference in Reference Area");
    gtk_widget_show (next_reference1);
    gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), next_reference1);
    gtk_widget_add_accelerator (next_reference1, "activate", accel_group, GDK_F6, GdkModifierType (0),
                              GTK_ACCEL_VISIBLE);

    image608 = gtk_image_new_from_stock ("gtk-go-down", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image608);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (next_reference1), image608);

    previous_reference1 = gtk_image_menu_item_new_with_mnemonic ("Previous reference in Reference Area");
    gtk_widget_show (previous_reference1);
    gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), previous_reference1);
    gtk_widget_add_accelerator (previous_reference1, "activate", accel_group, GDK_F6, GDK_SHIFT_MASK,
                                GTK_ACCEL_VISIBLE);

    image609 = gtk_image_new_from_stock ("gtk-go-up", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image609);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (previous_reference1), image609);
    
  }

  goto_next_project = gtk_image_menu_item_new_with_mnemonic ("Next project");
  gtk_widget_show (goto_next_project);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), goto_next_project);
  gtk_widget_add_accelerator (goto_next_project, "activate", accel_group,
                              GDK_Right, (GdkModifierType) (GDK_SHIFT_MASK | GDK_MOD1_MASK),
                              GTK_ACCEL_VISIBLE);

  image19528 = gtk_image_new_from_stock ("gtk-go-forward", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image19528);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (goto_next_project), image19528);

  goto_previous_project = gtk_image_menu_item_new_with_mnemonic ("Previous project");
  gtk_widget_show (goto_previous_project);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), goto_previous_project);
  gtk_widget_add_accelerator (goto_previous_project, "activate", accel_group,
                              GDK_Left, (GdkModifierType) (GDK_SHIFT_MASK | GDK_MOD1_MASK),
                              GTK_ACCEL_VISIBLE);

  image19529 = gtk_image_new_from_stock ("gtk-go-back", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image19529);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (goto_previous_project), image19529);

  reference1 = gtk_image_menu_item_new_with_mnemonic ("Any reference");
  gtk_widget_show (reference1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), reference1);
  gtk_widget_add_accelerator (reference1, "activate", accel_group, GDK_g, GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

  image101 = gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image101);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (reference1), image101);

  separator19 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator19);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), separator19);

  text_area1 = gtk_image_menu_item_new_with_mnemonic ("Text area");
  gtk_widget_show (text_area1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), text_area1);
  gtk_widget_add_accelerator (text_area1, "activate", accel_group,
                              GDK_F5, GdkModifierType (0),
                              GTK_ACCEL_VISIBLE);

  image4721 = gtk_image_new_from_stock ("gtk-home", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image4721);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (text_area1), image4721);

  goto_bible_notes_area1 = gtk_image_menu_item_new_with_mnemonic ("Bible notes area");
  gtk_widget_show (goto_bible_notes_area1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), goto_bible_notes_area1);
  gtk_widget_add_accelerator (goto_bible_notes_area1, "activate", accel_group,
                              GDK_F5, GdkModifierType (GDK_CONTROL_MASK | GDK_SHIFT_MASK),
                              GTK_ACCEL_VISIBLE);

  image11878 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image11878);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (goto_bible_notes_area1), image11878);

  references_area1 = gtk_image_menu_item_new_with_mnemonic ("Tools area");
  gtk_widget_show (references_area1);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), references_area1);
  gtk_widget_add_accelerator (references_area1, "activate", accel_group,
                              GDK_F5, GDK_SHIFT_MASK,
                              GTK_ACCEL_VISIBLE);

  image4722 = gtk_image_new_from_stock ("gtk-jump-to", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image4722);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (references_area1), image4722);

  if (guifeatures.styles ()) {
    
    insert_style = gtk_image_menu_item_new_with_mnemonic ("Styles area");
    gtk_widget_show (insert_style);
    gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), insert_style);
    gtk_widget_add_accelerator (insert_style, "activate", accel_group,
                                GDK_S, (GdkModifierType) GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image11111 = gtk_image_new_from_stock ("gtk-print-preview", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image11111);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (insert_style), image11111);
    
  }

  if (guifeatures.project_notes ()) {
    
    notes_area1 = gtk_image_menu_item_new_with_mnemonic ("Project notes area");
    gtk_widget_show (notes_area1);
    gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), notes_area1);
    gtk_widget_add_accelerator (notes_area1, "activate", accel_group,
                                GDK_F5, GDK_CONTROL_MASK,
                                GTK_ACCEL_VISIBLE);

    image4723 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image4723);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (notes_area1), image4723);
    
  }

  separator11 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator11);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), separator11);
  gtk_widget_set_sensitive (separator11, FALSE);

  synchronize_other_programs2 = gtk_image_menu_item_new_with_mnemonic ("_Synchronize other programs");
  gtk_widget_show (synchronize_other_programs2);
  gtk_container_add (GTK_CONTAINER (menuitem_goto_menu), synchronize_other_programs2);

  image4931 = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image4931);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (synchronize_other_programs2), image4931);
  
  check1 = NULL;
  if (guifeatures.checks ()) {

    check1 = gtk_menu_item_new_with_mnemonic ("Chec_k");
    gtk_widget_show (check1);
    gtk_container_add (GTK_CONTAINER (menubar1), check1);

    check1_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (check1), check1_menu);

    chapters_and_verses1 = gtk_image_menu_item_new_with_mnemonic ("_Chapters and verses");
    gtk_widget_show (chapters_and_verses1);
    gtk_container_add (GTK_CONTAINER (check1_menu), chapters_and_verses1);

    image5580 = gtk_image_new_from_stock ("gtk-zoom-100", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image5580);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (chapters_and_verses1), image5580);

    markers1 = gtk_image_menu_item_new_with_mnemonic ("_Markers");
    gtk_widget_show (markers1);
    gtk_container_add (GTK_CONTAINER (check1_menu), markers1);

    image5578 = gtk_image_new_from_stock ("gtk-zoom-100", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image5578);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (markers1), image5578);

    markers1_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (markers1), markers1_menu);

    validate_usfms1 = gtk_image_menu_item_new_with_mnemonic ("_Validate");
    gtk_widget_show (validate_usfms1);
    gtk_container_add (GTK_CONTAINER (markers1_menu), validate_usfms1);

    image5579 = gtk_image_new_from_stock ("gtk-ok", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image5579);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (validate_usfms1), image5579);

    count_usfms1 = gtk_image_menu_item_new_with_mnemonic ("_Count");
    gtk_widget_show (count_usfms1);
    gtk_container_add (GTK_CONTAINER (markers1_menu), count_usfms1);

    image6239 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6239);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (count_usfms1), image6239);

    compare_usfm1 = gtk_image_menu_item_new_with_mnemonic ("C_ompare");
    gtk_widget_show (compare_usfm1);
    gtk_container_add (GTK_CONTAINER (markers1_menu), compare_usfm1);

    image6748 = gtk_image_new_from_stock ("gtk-zoom-fit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6748);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (compare_usfm1), image6748);

    check_markers_spacing = gtk_image_menu_item_new_with_mnemonic ("_Spacing");
    gtk_widget_show (check_markers_spacing);
    gtk_container_add (GTK_CONTAINER (markers1_menu), check_markers_spacing);

    image17930 = gtk_image_new_from_stock ("gtk-media-pause", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image17930);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_markers_spacing), image17930);

    characters1 = gtk_image_menu_item_new_with_mnemonic ("C_haracters");
    gtk_widget_show (characters1);
    gtk_container_add (GTK_CONTAINER (check1_menu), characters1);

    image6867 = gtk_image_new_from_stock ("gtk-zoom-100", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6867);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (characters1), image6867);

    characters1_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (characters1), characters1_menu);

    count_characters = gtk_image_menu_item_new_with_mnemonic ("_Inventory");
    gtk_widget_show (count_characters);
    gtk_container_add (GTK_CONTAINER (characters1_menu), count_characters);

    image6868 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6868);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (count_characters), image6868);

    unwanted_patterns = gtk_image_menu_item_new_with_mnemonic ("_Unwanted patterns");
    gtk_widget_show (unwanted_patterns);
    gtk_container_add (GTK_CONTAINER (characters1_menu), unwanted_patterns);

    image7494 = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7494);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (unwanted_patterns), image7494);

    check_words = gtk_image_menu_item_new_with_mnemonic ("_Words");
    gtk_widget_show (check_words);
    gtk_container_add (GTK_CONTAINER (check1_menu), check_words);

    image7111 = gtk_image_new_from_stock ("gtk-zoom-100", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7111);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_words), image7111);

    check_words_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (check_words), check_words_menu);

    check_capitalization = gtk_image_menu_item_new_with_mnemonic ("_Capitalization");
    gtk_widget_show (check_capitalization);
    gtk_container_add (GTK_CONTAINER (check_words_menu), check_capitalization);

    image7112 = gtk_image_new_from_stock ("gtk-ok", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7112);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_capitalization), image7112);

    check_repetition = gtk_image_menu_item_new_with_mnemonic ("_Repetition");
    gtk_widget_show (check_repetition);
    gtk_container_add (GTK_CONTAINER (check_words_menu), check_repetition);

    image7238 = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7238);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_repetition), image7238);

    unwanted_words = gtk_image_menu_item_new_with_mnemonic ("_Unwanted");
    gtk_widget_show (unwanted_words);
    gtk_container_add (GTK_CONTAINER (check_words_menu), unwanted_words);

    image7631 = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7631);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (unwanted_words), image7631);

    word_count_inventory = gtk_image_menu_item_new_with_mnemonic ("_Inventory");
    gtk_widget_show (word_count_inventory);
    gtk_container_add (GTK_CONTAINER (check_words_menu), word_count_inventory);

    image13715 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image13715);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (word_count_inventory), image13715);

    check_punctuation = gtk_image_menu_item_new_with_mnemonic ("_Punctuation");
    gtk_widget_show (check_punctuation);
    gtk_container_add (GTK_CONTAINER (check1_menu), check_punctuation);

    image7366 = gtk_image_new_from_stock ("gtk-zoom-100", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7366);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_punctuation), image7366);

    check_punctuation_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (check_punctuation), check_punctuation_menu);

    check_matching_pairs = gtk_image_menu_item_new_with_mnemonic ("_Matching pairs");
    gtk_widget_show (check_matching_pairs);
    gtk_container_add (GTK_CONTAINER (check_punctuation_menu), check_matching_pairs);

    image7367 = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image7367);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_matching_pairs), image7367);

    check_references = gtk_image_menu_item_new_with_mnemonic ("_References");
    gtk_widget_show (check_references);
    gtk_container_add (GTK_CONTAINER (check1_menu), check_references);

    image21826 = gtk_image_new_from_stock ("gtk-zoom-fit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image21826);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_references), image21826);

    check_references_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (check_references), check_references_menu);

    check_references_inventory = gtk_image_menu_item_new_with_mnemonic ("_Inventory");
    gtk_widget_show (check_references_inventory);
    gtk_container_add (GTK_CONTAINER (check_references_menu), check_references_inventory);

    image21827 = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image21827);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_references_inventory), image21827);

    check_references_validate = gtk_image_menu_item_new_with_mnemonic ("_Validate");
    gtk_widget_show (check_references_validate);
    gtk_container_add (GTK_CONTAINER (check_references_menu), check_references_validate);

    image21828 = gtk_image_new_from_stock ("gtk-spell-check", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image21828);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_references_validate), image21828);

    checks_passages = gtk_image_menu_item_new_with_mnemonic ("P_assages");
    gtk_widget_show (checks_passages);
    gtk_container_add (GTK_CONTAINER (check1_menu), checks_passages);

    image24103 = gtk_image_new_from_stock ("gtk-justify-fill", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image24103);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (checks_passages), image24103);

    checks_passages_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (checks_passages), checks_passages_menu);

    check_nt_quotations_from_the_ot = gtk_image_menu_item_new_with_mnemonic ("_NT quotations from the OT");
    gtk_widget_show (check_nt_quotations_from_the_ot);
    gtk_container_add (GTK_CONTAINER (checks_passages_menu), check_nt_quotations_from_the_ot);

    image24104 = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image24104);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_nt_quotations_from_the_ot), image24104);

    synoptic_parallel_passages_from_the_nt = gtk_image_menu_item_new_with_mnemonic ("_Synoptic parallels from the NT");
    gtk_widget_show (synoptic_parallel_passages_from_the_nt);
    gtk_container_add (GTK_CONTAINER (checks_passages_menu), synoptic_parallel_passages_from_the_nt);

    image24105 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image24105);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (synoptic_parallel_passages_from_the_nt), image24105);

    parallels_from_the_ot = gtk_image_menu_item_new_with_mnemonic ("_Parallels from the OT");
    gtk_widget_show (parallels_from_the_ot);
    gtk_container_add (GTK_CONTAINER (checks_passages_menu), parallels_from_the_ot);

    image24106 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image24106);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (parallels_from_the_ot), image24106);

    check_terms = gtk_image_menu_item_new_with_mnemonic ("_Terms");
    gtk_widget_show (check_terms);
    gtk_container_add (GTK_CONTAINER (check1_menu), check_terms);

    image17074 = gtk_image_new_from_stock ("gtk-spell-check", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image17074);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (check_terms), image17074);

    my_checks = gtk_image_menu_item_new_with_mnemonic ("M_y checks");
    gtk_widget_show (my_checks);
    gtk_container_add (GTK_CONTAINER (check1_menu), my_checks);

    image15438 = gtk_image_new_from_stock ("gtk-home", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image15438);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (my_checks), image15438);
    
  }

  menutools = NULL;
  if (guifeatures.tools ()) {

    menutools = gtk_menu_item_new_with_mnemonic ("_Tools");
    gtk_widget_show (menutools);
    gtk_container_add (GTK_CONTAINER (menubar1), menutools);

    menutools_menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menutools), menutools_menu);

    line_cutter_for_hebrew_text1 = gtk_image_menu_item_new_with_mnemonic ("_Line cutter for Hebrew text");
    gtk_widget_show (line_cutter_for_hebrew_text1);
    gtk_container_add (GTK_CONTAINER (menutools_menu), line_cutter_for_hebrew_text1);

    image13532 = gtk_image_new_from_stock ("gtk-cut", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image13532);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (line_cutter_for_hebrew_text1), image13532);

    notes_transfer = gtk_image_menu_item_new_with_mnemonic ("_Transfer text to project notes");
    gtk_widget_show (notes_transfer);
    gtk_container_add (GTK_CONTAINER (menutools_menu), notes_transfer);

    image14659 = gtk_image_new_from_stock ("gtk-refresh", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image14659);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (notes_transfer), image14659);

    tool_origin_references_in_bible_notes = gtk_image_menu_item_new_with_mnemonic ("_Bible notes mass update");
    gtk_widget_show (tool_origin_references_in_bible_notes);
    gtk_container_add (GTK_CONTAINER (menutools_menu), tool_origin_references_in_bible_notes);

    image16248 = gtk_image_new_from_stock ("gtk-find-and-replace", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image16248);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tool_origin_references_in_bible_notes), image16248);

    tool_project_notes_mass_update1 = gtk_image_menu_item_new_with_mnemonic ("_Project notes mass update");
    gtk_widget_show (tool_project_notes_mass_update1);
    gtk_container_add (GTK_CONTAINER (menutools_menu), tool_project_notes_mass_update1);

    image17187 = gtk_image_new_from_stock ("gtk-execute", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image17187);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tool_project_notes_mass_update1), image17187);

    tool_generate_word_lists = gtk_image_menu_item_new_with_mnemonic ("_Generate word lists");
    gtk_widget_show (tool_generate_word_lists);
    gtk_container_add (GTK_CONTAINER (menutools_menu), tool_generate_word_lists);

    image20671 = gtk_image_new_from_stock ("gtk-justify-fill", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image20671);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tool_generate_word_lists), image20671);
    
    tool_simple_text_corrections = gtk_image_menu_item_new_with_mnemonic ("_Simple text corrections");
    gtk_widget_show (tool_simple_text_corrections);
    gtk_container_add (GTK_CONTAINER (menutools_menu), tool_simple_text_corrections);

    image21054 = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image21054);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (tool_simple_text_corrections), image21054);

  }

  menuitem_preferences = gtk_menu_item_new_with_mnemonic ("P_references");
  // At first the Alt-P was the shortcut key. On the XO machine, this key is 
  // in use already: 
  // Alt-N goes to the Next Activity and
  // Alt-P goes to the Previous one. 
  // For that reason the Alt-R is now the shortcut key.
  gtk_widget_show (menuitem_preferences);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_preferences);

  menuitem_preferences_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_preferences), menuitem_preferences_menu);

  preferences_remote_git_repository = NULL;
  if (guifeatures.preferences ()) {

    notes_preferences = gtk_image_menu_item_new_with_mnemonic ("Project _notes");
    gtk_widget_show (notes_preferences);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), notes_preferences);

    image2116 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image2116);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (notes_preferences), image2116);

    printingprefs = gtk_image_menu_item_new_with_mnemonic ("_Printing");
    gtk_widget_show (printingprefs);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), printingprefs);

    image3493 = gtk_image_new_from_stock ("gtk-print", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image3493);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (printingprefs), image3493);

    formatting_objects_processor = gtk_image_menu_item_new_with_mnemonic ("_Formatter");
    gtk_widget_show (formatting_objects_processor);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), formatting_objects_processor);

    image3718 = gtk_image_new_from_stock ("gtk-print-preview", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image3718);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (formatting_objects_processor), image3718);

    reference_exchange1 = gtk_image_menu_item_new_with_mnemonic ("_Reference exchange");
    gtk_widget_show (reference_exchange1);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), reference_exchange1);

    image5972 = gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image5972);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (reference_exchange1), image5972);

    ignored_references1 = gtk_image_menu_item_new_with_mnemonic ("_Ignored references");
    gtk_widget_show (ignored_references1);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), ignored_references1);

    image6467 = gtk_image_new_from_stock ("gtk-remove", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image6467);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (ignored_references1), image6467);

    prefs_books = gtk_image_menu_item_new_with_mnemonic ("_Books");
    gtk_widget_show (prefs_books);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), prefs_books);

    image12167 = gtk_image_new_from_stock ("gtk-index", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image12167);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (prefs_books), image12167);

    preferences_windows_outpost = gtk_image_menu_item_new_with_mnemonic ("_Windows Outpost");
    gtk_widget_show (preferences_windows_outpost);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_windows_outpost);

    image14287 = gtk_image_new_from_stock ("gtk-network", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image14287);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_windows_outpost), image14287);

    preferences_tidy_text = gtk_image_menu_item_new_with_mnemonic ("_Tidy text");
    gtk_widget_show (preferences_tidy_text);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_tidy_text);

    image16359 = gtk_image_new_from_stock ("gtk-clear", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image16359);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_tidy_text), image16359);

    preferences_remote_git_repository = gtk_image_menu_item_new_with_mnemonic ("_Git repository");
    gtk_widget_show (preferences_remote_git_repository);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_remote_git_repository);

    image18977 = gtk_image_new_from_stock ("gtk-connect", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image18977);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_remote_git_repository), image18977);
    
  }

  preferences_features = gtk_image_menu_item_new_with_mnemonic ("F_eatures");
  gtk_widget_show (preferences_features);
  gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_features);

  image20936 = gtk_image_new_from_stock ("gtk-properties", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image20936);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_features), image20936);

  preferences_planning = NULL;
  if (guifeatures.preferences ()) {
    
    preferences_password = gtk_image_menu_item_new_with_mnemonic ("P_assword");
    gtk_widget_show (preferences_password);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_password);

    image20937 = gtk_image_new_from_stock ("gtk-dialog-authentication", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image20937);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_password), image20937);
    
    preferences_text_replacement = gtk_image_menu_item_new_with_mnemonic ("Te_xt replacement");
    gtk_widget_show (preferences_text_replacement);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_text_replacement);

    image23181 = gtk_image_new_from_stock ("gtk-find-and-replace", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image23181);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_text_replacement), image23181);

    pdf_viewer1 = gtk_image_menu_item_new_with_mnemonic ("P_DF Viewer");
    gtk_widget_show (pdf_viewer1);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), pdf_viewer1);

    image24540 = gtk_image_new_from_stock ("gtk-zoom-fit", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image24540);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pdf_viewer1), image24540);

    preferences_reporting = gtk_image_menu_item_new_with_mnemonic ("Rep_orting");
    gtk_widget_show (preferences_reporting);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_reporting);

    image25623 = gtk_image_new_from_stock ("gtk-sort-ascending", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image25623);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_reporting), image25623);

    preferences_planning = gtk_image_menu_item_new_with_mnemonic ("P_lanning");
    gtk_widget_show (preferences_planning);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_planning);

    image26888 = gtk_image_new_from_stock ("gtk-preferences", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image26888);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_planning), image26888);

    preferences_graphical_interface = gtk_image_menu_item_new_with_mnemonic ("Grap_hical Interface");
    gtk_widget_show (preferences_graphical_interface);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_graphical_interface);

    image27031 = gtk_image_new_from_stock ("gtk-fullscreen", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image27031);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_graphical_interface), image27031);

    preferences_filters = gtk_image_menu_item_new_with_mnemonic ("Filter_s");
    gtk_widget_show (preferences_filters);
    gtk_container_add (GTK_CONTAINER (menuitem_preferences_menu), preferences_filters);

    image28360 = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_MENU);
    gtk_widget_show (image28360);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (preferences_filters), image28360);

  }

  menuitem_help = gtk_menu_item_new_with_mnemonic ("_Help");
  gtk_widget_show (menuitem_help);
  gtk_container_add (GTK_CONTAINER (menubar1), menuitem_help);

  menuitem_help_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem_help), menuitem_help_menu);

  help_main = gtk_image_menu_item_new_with_mnemonic ("_Contents");
  gtk_widget_show (help_main);
  gtk_container_add (GTK_CONTAINER (menuitem_help_menu), help_main);
  gtk_widget_add_accelerator (help_main, "activate", accel_group,
                              GDK_F1, (GdkModifierType) 0,
                              GTK_ACCEL_VISIBLE);

  image17520 = gtk_image_new_from_stock ("gtk-help", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image17520);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (help_main), image17520);

  help_context = gtk_image_menu_item_new_with_mnemonic ("C_ontext");
  gtk_widget_show (help_context);
  gtk_container_add (GTK_CONTAINER (menuitem_help_menu), help_context);
  gtk_widget_add_accelerator (help_context, "activate", accel_group,
                              GDK_H, (GdkModifierType) GDK_CONTROL_MASK,
                              GTK_ACCEL_VISIBLE);

  image16053 = gtk_image_new_from_stock ("gtk-help", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image16053);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (help_context), image16053);

  system_log1 = gtk_image_menu_item_new_with_mnemonic ("_System log");
  gtk_widget_show (system_log1);
  gtk_container_add (GTK_CONTAINER (menuitem_help_menu), system_log1);

  image4388 = gtk_image_new_from_stock ("gtk-dialog-info", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image4388);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (system_log1), image4388);

  about1 = gtk_image_menu_item_new_from_stock ("gtk-about", accel_group);
  gtk_widget_show (about1);
  gtk_container_add (GTK_CONTAINER (menuitem_help_menu), about1);

  toolbar1 = gtk_toolbar_new ();
  gtk_widget_show (toolbar1);
  gtk_box_pack_start (GTK_BOX (vbox1), toolbar1, FALSE, FALSE, 0);
  gtk_toolbar_set_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_BOTH_HORIZ);

  navigation.build (toolbar1, next_reference_in_history1, previous_reference_in_history1);
  g_signal_connect ((gpointer) navigation.reference_signal_delayed, "clicked", G_CALLBACK (on_navigation_new_reference_clicked), gpointer(this));
  
  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  hpaned1 = gtk_hpaned_new ();
  gtk_widget_show (hpaned1);
  gtk_box_pack_start (GTK_BOX (hbox1), hpaned1, TRUE, TRUE, 0);
  // Position of horizontal pane will be set later, but is set here to a decent value.
  gtk_paned_set_position (GTK_PANED (hpaned1), 600);

  vbox_left = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_left);
  gtk_paned_pack1 (GTK_PANED (hpaned1), vbox_left, FALSE, TRUE);

  vpaned1 = gtk_vpaned_new ();
  gtk_widget_show (vpaned1);
  gtk_box_pack_start (GTK_BOX (vbox_left), vpaned1, TRUE, TRUE, 0);
  // Position of main vertical pane will be set later, but is set here to a decent value.
  gtk_paned_set_position (GTK_PANED (vpaned1), 400);

  vboxeditors = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxeditors);
  gtk_paned_pack1 (GTK_PANED (vpaned1), vboxeditors, FALSE, TRUE);

  // Store project of last session because it gets affected when the editors build.
  ustring project_last_session = settings->genconfig.project_get ();
  // Building the tabbed and split editor.
  editorsgui = new EditorsGUI (vboxeditors);
  g_signal_connect ((gpointer) editorsgui->focus_button, "clicked", G_CALLBACK (on_editorsgui_focus_button_clicked), gpointer(this));
  
  // Another topic: project notes.
  notebook_notes_area = gtk_notebook_new ();
  gtk_widget_show (notebook_notes_area);
  if (guifeatures.project_notes ()) {
    gtk_paned_pack2 (GTK_PANED (vpaned1), notebook_notes_area, TRUE, TRUE);
  }
  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook_notes_area), FALSE);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook_notes_area), FALSE);

  notebook1 = gtk_notebook_new ();
  gtk_widget_show (notebook1);
  gtk_container_add (GTK_CONTAINER (notebook_notes_area), notebook1);
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook1), FALSE);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow3);
  gtk_container_add (GTK_CONTAINER (notebook1), scrolledwindow3);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview_notes = gtk_text_view_new ();
  gtk_widget_show (textview_notes);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), textview_notes);
  // The editor does not accept tabs, so that <Tab> goes to the next widget.
  // USFM has no tabs defined anyway. This applies to all textviews.
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview_notes), FALSE);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_notes), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview_notes), GTK_WRAP_WORD);

  label1 = gtk_label_new ("");
  gtk_widget_show (label1);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);

  hbox3 = gtk_hbox_new (FALSE, 4);
  gtk_widget_show (hbox3);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox3);
  gtk_container_set_border_width (GTK_CONTAINER (hbox3), 1);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow4);
  gtk_box_pack_start (GTK_BOX (hbox3), scrolledwindow4, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview_note = gtk_text_view_new ();
  gtk_widget_show (textview_note);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), textview_note);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview_note), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview_note), GTK_WRAP_WORD);
  if (!guifeatures.project_notes_management ()) gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_note), FALSE);

  vbox4 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox4);
  gtk_box_pack_start (GTK_BOX (hbox3), vbox4, FALSE, FALSE, 0);

  button_note_cancel = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (button_note_cancel);
  gtk_box_pack_end (GTK_BOX (vbox4), button_note_cancel, FALSE, FALSE, 0);

  button_note_ok = gtk_button_new_from_stock ("gtk-ok");
  if (guifeatures.project_notes_management ()) gtk_widget_show (button_note_ok);
  gtk_box_pack_end (GTK_BOX (vbox4), button_note_ok, FALSE, FALSE, 0);

  label2 = gtk_label_new ("");
  gtk_widget_show (label2);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), label2);

  label30 = gtk_label_new ("");
  gtk_widget_show (label30);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_notes_area), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_notes_area), 0), label30);

  scrolledwindow_keyterm_text = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_keyterm_text);
  gtk_container_add (GTK_CONTAINER (notebook_notes_area), scrolledwindow_keyterm_text);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_keyterm_text), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_keyterm_text), GTK_SHADOW_IN);

  textview_keyterm_text = gtk_text_view_new ();
  gtk_widget_show (textview_keyterm_text);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_keyterm_text), textview_keyterm_text);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_keyterm_text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview_keyterm_text), GTK_WRAP_WORD);
  
  label31 = gtk_label_new ("");
  gtk_widget_show (label31);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_notes_area), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_notes_area), 1), label31);

  vbox_right = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_right);
  gtk_paned_pack2 (GTK_PANED (hpaned1), vbox_right, TRUE, TRUE);

  notebook_tools = gtk_notebook_new ();
  gtk_widget_show (notebook_tools);
  gtk_box_pack_start (GTK_BOX (vbox_right), notebook_tools, TRUE, TRUE, 0);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook_tools), TRUE);

  // Deal with screen layout.
  if (settings->genconfig.tools_area_left_get ()) {
    screen_layout_tools_area_set (false, vbox_left, vbox_right, vpaned1, notebook_tools);  
  }
  
  vpaned_references = gtk_vpaned_new ();
  gtk_widget_show (vpaned_references);
  gtk_container_add (GTK_CONTAINER (notebook_tools), vpaned_references);
  // Position of pane will be set later, but is here initialized to a decent value.
  gtk_paned_set_position (GTK_PANED (vpaned_references), 400);  

  scrolledwindow_references = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_references);
  gtk_paned_pack1 (GTK_PANED (vpaned_references), scrolledwindow_references, FALSE, TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_references), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  // Manually added and changed.
  // 1. localized human readable reference
  // 2. comment
  // 3. book id
  // 4. chapter
  // 5. verse
  liststore_references = gtk_list_store_new (5, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING);
  // Text cell renderer.
  GtkCellRenderer *renderer;
  renderer = gtk_cell_renderer_text_new ();
  
  treeview_references = gtk_tree_view_new_with_model (GTK_TREE_MODEL (liststore_references));
  gtk_widget_show (treeview_references);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_references), treeview_references);
  
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview_references), TRUE);
  // Unreference the store once, so it gets destroyed with the treeview.
  g_object_unref (liststore_references);
  // Add reference column.
  treecolumn_references = gtk_tree_view_column_new_with_attributes ("", renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_references), treecolumn_references);
  // Add comments column
  GtkTreeViewColumn * treecolumn2;
  treecolumn2 = gtk_tree_view_column_new_with_attributes ("Comment", renderer, "text", 1, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (treeview_references), treecolumn2);
  treeselect_references = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_references));
  gtk_tree_selection_set_mode (treeselect_references, GTK_SELECTION_MULTIPLE);

  scrolledwindow_quick_refs = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_quick_refs);
  gtk_paned_pack2 (GTK_PANED (vpaned_references), scrolledwindow_quick_refs, TRUE, TRUE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_quick_refs), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview_quick_refs = gtk_text_view_new ();
  gtk_widget_show (textview_quick_refs);
  gtk_container_add (GTK_CONTAINER (scrolledwindow_quick_refs), textview_quick_refs);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_quick_refs), FALSE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview_quick_refs), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview_quick_refs), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview_quick_refs), FALSE);
  
  label13 = gtk_label_new ("References");
  gtk_widget_show (label13);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 0), label13);
  
  if (!guifeatures.references_and_find ()) {
    gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 0));
  }

  vbox_styles = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_styles);
  gtk_container_add (GTK_CONTAINER (notebook_tools), vbox_styles);

  // Styles gui object
  styles = new GuiStyles (vbox_styles, style, style_menu, 
                          stylesheets_expand_all, stylesheets_collapse_all, 
                          style_insert, stylesheet_edit_mode, style_new, 
                          style_properties, style_delete, 
                          stylesheet_switch, stylesheets_new, stylesheets_delete,
                          stylesheets_rename, stylesheets_import, stylesheets_export);

  label14 = gtk_label_new ("Styles");
  gtk_widget_show (label14);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 1), label14);
  
  if (!guifeatures.styles ()) {
    gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 1));
  }

  vbox_project_note = gtk_vbox_new (FALSE, 1);
  gtk_widget_show (vbox_project_note);
  gtk_container_add (GTK_CONTAINER (notebook_tools), vbox_project_note);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_project_note), 2);

  label_note_category = gtk_label_new_with_mnemonic ("C_ategory");
  gtk_widget_show (label_note_category);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_category, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_category), 0, 0.5);

  combobox_note_category = gtk_combo_box_new_text ();
  gtk_widget_show (combobox_note_category);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), combobox_note_category, FALSE, TRUE, 0);
  
  label_note_references = gtk_label_new_with_mnemonic ("_References");
  gtk_widget_show (label_note_references);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_references, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_references), 0, 0.5);

  scrolledwindow8 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow8);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), scrolledwindow8, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow8), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview_note_references = gtk_text_view_new ();
  gtk_widget_show (textview_note_references);
  gtk_container_add (GTK_CONTAINER (scrolledwindow8), textview_note_references);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview_note_references), FALSE);

  label_note_project = gtk_label_new_with_mnemonic ("Pro_ject");
  gtk_widget_show (label_note_project);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_project, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_project), 0, 0.5);

  combobox_note_project = gtk_combo_box_new_text ();
  gtk_widget_show (combobox_note_project);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), combobox_note_project, FALSE, TRUE, 0);

  label_note_created_on = gtk_label_new ("");
  gtk_widget_show (label_note_created_on);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_created_on, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_created_on), 0, 0.5);

  label_note_created_by = gtk_label_new ("");
  gtk_widget_show (label_note_created_by);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_created_by, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_created_by), 0, 0.5);

  label_note_edited_on = gtk_label_new ("");
  gtk_widget_show (label_note_edited_on);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_edited_on, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_edited_on), 0, 0.5);

  label_note_logbook = gtk_label_new ("Logbook");
  gtk_widget_show (label_note_logbook);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), label_note_logbook, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label_note_logbook), 0, 0.5);

  scrolledwindow9 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow9);
  gtk_box_pack_start (GTK_BOX (vbox_project_note), scrolledwindow9, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow9), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  textview_note_logbook = gtk_text_view_new ();
  gtk_widget_show (textview_note_logbook);
  gtk_container_add (GTK_CONTAINER (scrolledwindow9), textview_note_logbook);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview_note_logbook), FALSE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview_note_logbook), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview_note_logbook), GTK_WRAP_WORD);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview_note_logbook), FALSE);

  label_notetools = gtk_label_new ("Project note");
  gtk_widget_show (label_notetools);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 2), label_notetools);

  scrolledwindow_keyterms = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_keyterms);
  gtk_container_add (GTK_CONTAINER (notebook_tools), scrolledwindow_keyterms);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_keyterms), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_keyterms), GTK_SHADOW_IN);

  vbox_keyterms = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_keyterms);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow_keyterms), vbox_keyterms);

  // Code that creates widgets for the KeytermsGUI has been moved there.

  label22 = gtk_label_new ("Keyterms");
  gtk_widget_show (label22);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 3), label22);

  vbox_outline = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_outline);
  gtk_container_add (GTK_CONTAINER (notebook_tools), vbox_outline);
  
  if (!guifeatures.checks ()) {
    gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), tapntKeyterms));
  }
  
  // Code for widgets creation for the Outline object has been moved there.
  
  label36 = gtk_label_new ("Outline");
  gtk_widget_show (label36);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 4), label36);
  
  scrolledwindow_resources = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_resources);
  gtk_container_add (GTK_CONTAINER (notebook_tools), scrolledwindow_resources);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_resources), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_resources), GTK_SHADOW_IN);

  vbox_resources = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox_resources);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow_resources), vbox_resources);
  
  label_tool_resources = gtk_label_new ("Resources");
  gtk_widget_show (label_tool_resources);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 5), label_tool_resources);

  scrolledwindow_merge = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow_merge);
  gtk_container_add (GTK_CONTAINER (notebook_tools), scrolledwindow_merge);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow_merge), GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow_merge), GTK_SHADOW_IN);

  vboxmerge = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxmerge);
  gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolledwindow_merge), vboxmerge);

  labelmerge = gtk_label_new ("Merge");
  gtk_widget_show (labelmerge);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_tools), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), 6), labelmerge);

  mergegui = new MergeGUI (vboxmerge);
  g_signal_connect ((gpointer) mergegui->editors_get_text_button, "clicked", G_CALLBACK (on_mergegui_get_text_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) mergegui->new_reference_button, "clicked", G_CALLBACK (on_mergegui_new_reference_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) mergegui->save_editors_button, "clicked", G_CALLBACK (on_mergegui_save_editors_button_clicked), gpointer(this));
  g_signal_connect ((gpointer) mergegui->reload_editors_button, "clicked", G_CALLBACK (on_editor_reload_clicked), gpointer(this));

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox5, FALSE, FALSE, 0);

  hbox7 = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox7);
  gtk_box_pack_start (GTK_BOX (hbox5), hbox7, TRUE, TRUE, 0);

  statuslabel_stylesheet = gtk_label_new ("");
  gtk_widget_show (statuslabel_stylesheet);
  gtk_box_pack_start (GTK_BOX (hbox7), statuslabel_stylesheet, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (statuslabel_stylesheet), 0, 0.5);

  statuslabel_style = gtk_label_new ("");
  gtk_widget_show (statuslabel_style);
  gtk_box_pack_start (GTK_BOX (hbox7), statuslabel_style, FALSE, FALSE, 0);

  // This label could display a lot of styles so running out of space, and 
  // therefore needs to show an ellipsis if there is too much text.
  gtk_label_set_ellipsize (GTK_LABEL (statuslabel_style), PANGO_ELLIPSIZE_MIDDLE);
  gtk_label_set_max_width_chars (GTK_LABEL (statuslabel_style), 50);

  label_git = gtk_label_new ("");
  gtk_widget_show (label_git);
  gtk_box_pack_start (GTK_BOX (hbox7), label_git, FALSE, FALSE, 0);

  statusbar1 = gtk_statusbar_new ();
  gtk_widget_show (statusbar1);
  gtk_box_pack_start (GTK_BOX (hbox5), statusbar1, FALSE, TRUE, 0);
  gtk_widget_set_size_request (statusbar1, 25, -1);

  // Appearance of text in editor.
  set_fonts ();

  // Size and position of window and screen layout.
  {
    ScreenLayoutDimensions dimensions (mainwindow, hpaned1, vpaned1, vpaned_references);
    dimensions.verify ();
    dimensions.load ();
  }

  g_signal_connect ((gpointer) mainwindow, "destroy", G_CALLBACK (gtk_main_quit), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "delete_event", G_CALLBACK (on_mainwindow_delete_event), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "focus_in_event", G_CALLBACK (on_mainwindow_focus_in_event), gpointer(this));
  g_signal_connect ((gpointer) mainwindow, "window_state_event", G_CALLBACK (on_mainwindow_window_state_event), gpointer(this));
  if (guifeatures.project_management ()) {
    g_signal_connect ((gpointer) new1, "activate", G_CALLBACK (on_new1_activate), gpointer(this));
    g_signal_connect ((gpointer) open1, "activate", G_CALLBACK (on_open1_activate), gpointer(this));
    g_signal_connect ((gpointer) close1, "activate", G_CALLBACK (on_close1_activate), gpointer(this));
    g_signal_connect ((gpointer) delete1, "activate", G_CALLBACK (on_delete1_activate), gpointer(this));
  }
  if (guifeatures.printing ()) {
    g_signal_connect ((gpointer) print_project, "activate", G_CALLBACK (on_print_project_activate), gpointer(this));
  }
  if (guifeatures.project_management ()) {
    g_signal_connect ((gpointer) properties1, "activate", G_CALLBACK (on_properties1_activate), gpointer(this));
    g_signal_connect ((gpointer) import1, "activate", G_CALLBACK (on_import1_activate), gpointer(this));
    g_signal_connect ((gpointer) export_usfm_files, "activate", G_CALLBACK (on_export_usfm_files_activate), gpointer(this));
    g_signal_connect ((gpointer) export_zipped_unified_standard_format_markers1, "activate", G_CALLBACK (on_export_zipped_unified_standard_format_markers1_activate), gpointer(this));
    g_signal_connect ((gpointer) to_bibleworks_version_database_compiler, "activate", G_CALLBACK (on_to_bibleworks_version_compiler_activate), gpointer(this));
    g_signal_connect ((gpointer) export_to_sword_module, "activate", G_CALLBACK (on_export_to_sword_module_activate), gpointer(this));
    g_signal_connect ((gpointer) export_opendocument, "activate", G_CALLBACK (on_export_opendocument_activate), gpointer(this));  
    g_signal_connect ((gpointer) copy_project_to, "activate", G_CALLBACK (on_copy_project_to_activate), gpointer(this));
    g_signal_connect ((gpointer) compare_with1, "activate", G_CALLBACK (on_compare_with1_activate), gpointer(this));
    g_signal_connect ((gpointer) parallel_bible, "activate", G_CALLBACK (on_parallel_bible_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) project_backup_incremental, "activate", G_CALLBACK (on_project_backup_incremental_activate), gpointer(this));
  g_signal_connect ((gpointer) project_backup_flexible, "activate", G_CALLBACK (on_project_backup_flexible_activate), gpointer(this));
  if (guifeatures.project_management ()) {
    g_signal_connect ((gpointer) project_changes, "activate", G_CALLBACK (on_project_changes_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) file_projects_merge, "activate", G_CALLBACK (on_file_projects_merge_activate), gpointer(this));
  if (guifeatures.references_management ()) {
    g_signal_connect ((gpointer) file_references, "activate", G_CALLBACK (on_file_references_activate), gpointer(this));
    g_signal_connect ((gpointer) open_references1, "activate", G_CALLBACK (on_open_references1_activate), gpointer(this));
    g_signal_connect ((gpointer) references_save_as, "activate", G_CALLBACK (on_references_save_as_activate), gpointer(this));
  }
  if (guifeatures.printing ()) {
    g_signal_connect ((gpointer) print_references, "activate", G_CALLBACK (on_print_references_activate), gpointer(this));
  }
  if (guifeatures.references_management ()) {
    g_signal_connect ((gpointer) close_references, "activate", G_CALLBACK (on_close_references_activate), gpointer (this));
    g_signal_connect ((gpointer) delete_references, "activate", G_CALLBACK (on_delete_references_activate), gpointer (this));
    g_signal_connect ((gpointer) reference_hide, "activate", G_CALLBACK (on_reference_hide_activate), gpointer (this));
  }
  if (guifeatures.project_notes_management ()) {
    g_signal_connect ((gpointer) new_note, "activate", G_CALLBACK (on_new_note_activate), gpointer(this));
    g_signal_connect ((gpointer) delete_note, "activate", G_CALLBACK (on_delete_note_activate), gpointer(this));
    g_signal_connect ((gpointer) import_notes, "activate", G_CALLBACK (on_import_notes_activate), gpointer(this));
    g_signal_connect ((gpointer) export_notes, "activate", G_CALLBACK (on_export_notes_activate), gpointer(this));
  }
  if (style) g_signal_connect ((gpointer) style, "activate", G_CALLBACK (on_file_styles_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources, "activate", G_CALLBACK (on_file_resources_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources_open, "activate", G_CALLBACK (on_file_resources_open_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources_close, "activate", G_CALLBACK (on_file_resources_close_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources_new, "activate", G_CALLBACK (on_file_resources_new_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources_edit, "activate", G_CALLBACK (on_file_resources_edit_activate), gpointer(this));
  g_signal_connect ((gpointer) file_resources_delete, "activate", G_CALLBACK (on_file_resources_delete_activate), gpointer(this));
  g_signal_connect ((gpointer) quit1, "activate", G_CALLBACK (on_quit1_activate), gpointer(this));
  g_signal_connect ((gpointer) menuitem_edit, "activate", G_CALLBACK (on_edit1_activate), gpointer(this));
  g_signal_connect ((gpointer) cut1, "activate", G_CALLBACK (on_cut1_activate), gpointer(this));
  g_signal_connect ((gpointer) copy1, "activate", G_CALLBACK (on_copy1_activate), gpointer(this));
  g_signal_connect ((gpointer) copy_without_formatting, "activate", G_CALLBACK (on_copy_without_formatting_activate), gpointer(this));
  g_signal_connect ((gpointer) paste1, "activate", G_CALLBACK (on_paste1_activate), gpointer(this));
  g_signal_connect ((gpointer) undo1, "activate", G_CALLBACK (on_undo1_activate), gpointer(this));
  g_signal_connect ((gpointer) redo1, "activate", G_CALLBACK (on_redo1_activate), gpointer(this));
  if (guifeatures.references_and_find ()) {
    g_signal_connect ((gpointer) find1, "activate", G_CALLBACK (on_findspecial1_activate), gpointer(this));
  }
  if (guifeatures.replace ()) {
    g_signal_connect ((gpointer) find_and_replace1, "activate", G_CALLBACK (on_find_and_replace1_activate), gpointer (this));
  }
  g_signal_connect ((gpointer) find_in_notes1, "activate", G_CALLBACK (on_find_in_notes1_activate), gpointer(this));
  g_signal_connect ((gpointer) get_references_from_note, "activate", G_CALLBACK (on_get_references_from_note_activate), gpointer(this));
  g_signal_connect ((gpointer) edit_revert, "activate", G_CALLBACK (on_edit_revert_activate), gpointer(this));
  g_signal_connect ((gpointer) edit_bible_note, "activate", G_CALLBACK (on_edit_bible_note_activate), gpointer(this));
  g_signal_connect ((gpointer) editstatus, "activate", G_CALLBACK (on_editstatus_activate), gpointer(this));
  g_signal_connect ((gpointer) edit_planning, "activate",  G_CALLBACK (on_edit_planning_activate), gpointer(this));
  g_signal_connect ((gpointer) menuitem_view, "activate", G_CALLBACK (on_menuitem_view_activate), gpointer(this));
  g_signal_connect ((gpointer) view_text_font, "activate", G_CALLBACK (on_view_text_font_activate), gpointer(this));
  g_signal_connect ((gpointer) view_notes_font, "activate", G_CALLBACK (on_view_notes_font_activate), gpointer(this));
  g_signal_connect ((gpointer) printer_font, "activate", G_CALLBACK (on_printer_font_activate), gpointer(this));
  if (guifeatures.project_notes ()) g_signal_connect ((gpointer) viewnotes, "activate", G_CALLBACK (on_viewnotes_activate), gpointer(this));
  g_signal_connect ((gpointer) screen_layout, "activate", G_CALLBACK (on_screen_layout_activate), gpointer(this));
  g_signal_connect ((gpointer) view_git_tasks, "activate", G_CALLBACK (on_view_git_tasks_activate), gpointer(this));
  g_signal_connect ((gpointer) parallel_passages1, "activate", G_CALLBACK (on_parallel_passages1_activate), gpointer(this));
  g_signal_connect ((gpointer) view_usfm_code, "activate", G_CALLBACK (on_view_usfm_code_activate), gpointer(this));
  g_signal_connect ((gpointer) view_status, "activate", G_CALLBACK (on_view_status_activate), gpointer(this));
  g_signal_connect ((gpointer) view_planning, "activate", G_CALLBACK (on_view_planning_activate), gpointer(this));
  g_signal_connect ((gpointer) insert1, "activate", G_CALLBACK (on_insert1_activate), gpointer(this));
  if (guifeatures.project_notes ()) {
    g_signal_connect ((gpointer) standard_text_1, "activate", G_CALLBACK (on_standard_text_1_activate), gpointer (this));
    g_signal_connect ((gpointer) standard_text_2, "activate", G_CALLBACK (on_standard_text_2_activate), gpointer (this));
    g_signal_connect ((gpointer) standard_text_3, "activate", G_CALLBACK (on_standard_text_3_activate), gpointer (this));
    g_signal_connect ((gpointer) standard_text_4, "activate", G_CALLBACK (on_standard_text_4_activate), gpointer (this));
    g_signal_connect ((gpointer) current_reference1, "activate", G_CALLBACK (on_current_reference1_activate), gpointer (this));
  }
  g_signal_connect ((gpointer) insert_special_character, "activate", G_CALLBACK (on_insert_special_character_activate), gpointer (this));
  if (guifeatures.styles ()) {
    g_signal_connect ((gpointer) insert_style, "activate", G_CALLBACK (on_goto_styles_area_activate), gpointer (this));
  }
  g_signal_connect ((gpointer) next_verse1, "activate", G_CALLBACK (on_next_verse_activate), gpointer (this));
  g_signal_connect ((gpointer) previous_verse1, "activate", G_CALLBACK (on_previous_verse_activate), gpointer (this));
  g_signal_connect ((gpointer) next_chapter1, "activate", G_CALLBACK (on_next_chapter_activate), gpointer (this));
  g_signal_connect ((gpointer) previous_chapter1, "activate", G_CALLBACK (on_previous_chapter_activate), gpointer (this));
  g_signal_connect ((gpointer) next_book1, "activate", G_CALLBACK (on_next_book_activate), gpointer (this));
  g_signal_connect ((gpointer) previous_book1, "activate", G_CALLBACK (on_previous_book_activate), gpointer (this));
  if (guifeatures.references_and_find ()) {
    g_signal_connect ((gpointer) next_reference1, "activate", G_CALLBACK (on_next_reference1_activate), gpointer(this));
    g_signal_connect ((gpointer) previous_reference1, "activate", G_CALLBACK (on_previous_reference1_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) goto_next_project, "activate", G_CALLBACK (on_goto_next_project_activate), gpointer(this));
  g_signal_connect ((gpointer) goto_previous_project, "activate", G_CALLBACK (on_goto_previous_project_activate), gpointer(this));
  g_signal_connect ((gpointer) reference1, "activate", G_CALLBACK (on_reference_activate), gpointer(this));
  g_signal_connect ((gpointer) text_area1, "activate", G_CALLBACK (on_text_area1_activate), gpointer(this));
  g_signal_connect ((gpointer) goto_bible_notes_area1, "activate", G_CALLBACK (on_goto_bible_notes_area1_activate), gpointer(this));
  g_signal_connect ((gpointer) references_area1, "activate", G_CALLBACK (on_tools_area1_activate), gpointer(this));
  if (guifeatures.project_notes ()) {
    g_signal_connect ((gpointer) notes_area1, "activate", G_CALLBACK (on_notes_area1_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) synchronize_other_programs2, "activate", G_CALLBACK (on_synchronize_other_programs2_activate), gpointer(this));
  if (guifeatures.checks ()) {
    g_signal_connect ((gpointer) check1, "activate", G_CALLBACK (on_check1_activate), gpointer(this));
    g_signal_connect ((gpointer) markers1, "activate", G_CALLBACK (on_markers1_activate), gpointer(this));
    g_signal_connect ((gpointer) validate_usfms1, "activate", G_CALLBACK (on_validate_usfms1_activate), gpointer(this));
    g_signal_connect ((gpointer) count_usfms1, "activate", G_CALLBACK (on_count_usfms1_activate), gpointer(this));
    g_signal_connect ((gpointer) compare_usfm1, "activate", G_CALLBACK (on_compare_usfm1_activate), gpointer(this));
    g_signal_connect ((gpointer) check_markers_spacing, "activate", G_CALLBACK (on_check_markers_spacing_activate), gpointer(this));
    g_signal_connect ((gpointer) chapters_and_verses1, "activate", G_CALLBACK (on_chapters_and_verses1_activate), gpointer(this));
    g_signal_connect ((gpointer) count_characters, "activate", G_CALLBACK (on_count_characters_activate), gpointer(this));
    g_signal_connect ((gpointer) unwanted_patterns, "activate", G_CALLBACK (on_unwanted_patterns_activate), gpointer(this));
    g_signal_connect ((gpointer) check_capitalization, "activate", G_CALLBACK (on_check_capitalization_activate), gpointer(this));
    g_signal_connect ((gpointer) check_repetition, "activate", G_CALLBACK (on_check_repetition_activate), gpointer(this));
    g_signal_connect ((gpointer) unwanted_words, "activate", G_CALLBACK (on_unwanted_words_activate), gpointer(this));
    g_signal_connect ((gpointer) check_matching_pairs, "activate", G_CALLBACK (on_check_matching_pairs_activate), gpointer(this));
    g_signal_connect ((gpointer) word_count_inventory, "activate", G_CALLBACK (on_word_count_inventory_activate), gpointer(this));
    g_signal_connect ((gpointer) check_references_inventory, "activate", G_CALLBACK (on_check_references_inventory_activate), gpointer(this));
    g_signal_connect ((gpointer) check_references_validate, "activate", G_CALLBACK (on_check_references_validate_activate), gpointer(this));
    g_signal_connect ((gpointer) check_nt_quotations_from_the_ot, "activate", G_CALLBACK (on_check_nt_quotations_from_the_ot_activate), gpointer(this));
    g_signal_connect ((gpointer) synoptic_parallel_passages_from_the_nt, "activate", G_CALLBACK (on_synoptic_parallel_passages_from_the_nt_activate), gpointer(this));
    g_signal_connect ((gpointer) parallels_from_the_ot, "activate", G_CALLBACK (on_parallels_from_the_ot_activate), gpointer(this));
    g_signal_connect ((gpointer) check_terms, "activate", G_CALLBACK (on_check_terms_activate), gpointer(this));
    g_signal_connect ((gpointer) my_checks, "activate", G_CALLBACK (on_my_checks_activate), gpointer(this));
  }
  if (guifeatures.tools ()) {
    g_signal_connect ((gpointer) menutools, "activate", G_CALLBACK (on_menutools_activate), gpointer(this));
    g_signal_connect ((gpointer) line_cutter_for_hebrew_text1, "activate", G_CALLBACK (on_line_cutter_for_hebrew_text1_activate), gpointer(this));
    g_signal_connect ((gpointer) notes_transfer, "activate", G_CALLBACK (on_notes_transfer_activate), gpointer(this));
    g_signal_connect ((gpointer) tool_origin_references_in_bible_notes, "activate", G_CALLBACK (on_tool_origin_references_in_bible_notes_activate), gpointer(this));
    g_signal_connect ((gpointer) tool_project_notes_mass_update1, "activate", G_CALLBACK (on_tool_project_notes_mass_update1_activate), gpointer(this));
    g_signal_connect ((gpointer) tool_generate_word_lists, "activate", G_CALLBACK (on_tool_generate_word_lists_activate), gpointer(this));
    g_signal_connect ((gpointer) tool_simple_text_corrections, "activate", G_CALLBACK (on_tool_simple_text_corrections_activate), gpointer(this));
  }
  if (guifeatures.preferences ()) {
    g_signal_connect ((gpointer) notes_preferences, "activate", G_CALLBACK (on_notes_preferences_activate), gpointer(this));
    g_signal_connect ((gpointer) printingprefs, "activate", G_CALLBACK (on_printingprefs_activate), gpointer(this));
    g_signal_connect ((gpointer) formatting_objects_processor, "activate", G_CALLBACK (on_formatting_objects_processor_activate), gpointer(this));
    g_signal_connect ((gpointer) reference_exchange1, "activate", G_CALLBACK (on_reference_exchange1_activate), gpointer(this));
    g_signal_connect ((gpointer) ignored_references1, "activate", G_CALLBACK (on_ignored_references1_activate), gpointer(this));
    g_signal_connect ((gpointer) prefs_books, "activate", G_CALLBACK (on_prefs_books_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_windows_outpost, "activate", G_CALLBACK (on_preferences_windows_outpost_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_tidy_text, "activate", G_CALLBACK (on_preferences_tidy_text_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_remote_git_repository, "activate", G_CALLBACK (on_preferences_remote_git_repository_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) preferences_features, "activate", G_CALLBACK (on_preferences_features_activate), gpointer(this));
  if (guifeatures.preferences ()) {
    g_signal_connect ((gpointer) preferences_password, "activate", G_CALLBACK (on_preferences_password_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_text_replacement, "activate", G_CALLBACK (on_preferences_text_replacement_activate), gpointer(this));
    g_signal_connect ((gpointer) pdf_viewer1, "activate", G_CALLBACK (on_pdf_viewer1_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_reporting, "activate", G_CALLBACK (on_preferences_reporting_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_planning, "activate", G_CALLBACK (on_preferences_planning_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_graphical_interface, "activate", G_CALLBACK (on_preferences_graphical_interface_activate), gpointer(this));
    g_signal_connect ((gpointer) preferences_filters, "activate", G_CALLBACK (on_preferences_filters_activate), gpointer(this));
  }
  g_signal_connect ((gpointer) help_context, "activate", G_CALLBACK (on_help_context_activate), gpointer(this));
  g_signal_connect ((gpointer) help_main, "activate", G_CALLBACK (on_help_main_activate), gpointer(this));
  g_signal_connect ((gpointer) system_log1, "activate", G_CALLBACK (on_system_log1_activate), gpointer(this));
  g_signal_connect ((gpointer) about1, "activate", G_CALLBACK (on_about1_activate), gpointer(this));
  g_signal_connect ((gpointer) button_note_ok, "clicked", G_CALLBACK (on_button_ok_clicked), gpointer(this));
  g_signal_connect ((gpointer) button_note_cancel, "clicked", G_CALLBACK (on_button_cancel_clicked), gpointer(this));
  g_signal_connect ((gpointer) notebook_tools, "switch_page", G_CALLBACK (on_notebook_tools_switch_page), gpointer(this));
  g_signal_connect ((gpointer) treeview_references, "key_press_event", G_CALLBACK (on_treeview_references_key_press_event), gpointer(this));
  g_signal_connect ((gpointer) treeview_references, "button_press_event", G_CALLBACK (on_treeview_references_button_press_event), gpointer(this));
  g_signal_connect ((gpointer) treeview_references, "popup_menu", G_CALLBACK (on_treeview_references_popup_menu), gpointer(this));
  g_signal_connect ((gpointer) treeview_references, "move_cursor", G_CALLBACK (on_treeview_references_move_cursor), gpointer(this));
  g_signal_connect ((gpointer) treeview_references, "cursor_changed", G_CALLBACK (on_treeview_references_cursor_changed), gpointer(this));
  g_signal_connect ((gpointer) textview_notes, "motion-notify-event", G_CALLBACK (notes_motion_notify_event), gpointer(this));
  g_signal_connect ((gpointer) textview_notes, "visibility-notify-event", G_CALLBACK (screen_visibility_notify_event), gpointer(this));
  g_signal_connect ((gpointer) textview_notes, "key-press-event", G_CALLBACK (notes_key_press_event), gpointer(this));
  g_signal_connect ((gpointer) textview_notes, "event-after", G_CALLBACK (notes_event_after), gpointer(this));
  g_signal_connect ((gpointer) styles->apply_signal, "clicked", G_CALLBACK (on_style_button_apply_clicked), gpointer (this));
  g_signal_connect ((gpointer) styles->open_signal, "clicked", G_CALLBACK (on_style_button_open_clicked), gpointer (this));
  g_signal_connect ((gpointer) styles->edited_signal, "clicked", G_CALLBACK (on_style_edited), gpointer (this));

  gtk_window_add_accel_group (GTK_WINDOW (mainwindow), accel_group);

  gtk_label_set_mnemonic_widget (GTK_LABEL (label_note_references), textview_note_references);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label_note_category), combobox_note_category);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label_note_project), combobox_note_project);

  // Tools Area: set page number to display, but don't display the notes tab.
  int tools_area_page = settings->genconfig.tools_area_page_number_get ();
  if (tools_area_page == tapntProjectNote) tools_area_page--;
  gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), tapntProjectNote));
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tools_area_page);
  // If the last page currently shows, it won't give a signal that the page switches,
  // because the page already would be at the last page. Simulate this signal.
  if (tools_area_page == tapntEnd - 1) notebook_tools_switch_page (tools_area_page);

  // Store pointer to the relevant text buffers to simplify later use.
  // The reference count on the buffer is not incremented; 
  // the caller of this function won't own a new reference.
  // Therefore the buffers do not need to be freed.
  textbuffer_notes = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_notes));

  // Jumpstart editor.
  jump_start_editors (project_last_session);

  // Communication with BibleTime
  got_new_bt_reference = 0;
  g_timeout_add (100, GSourceFunc (mainwindow_on_external_programs_timeout), gpointer(this));

  #ifndef WIN32
  // Signal handling.
  // Block the signal of a pipe error that otherwise would kill bibledit.
  signal (SIGPIPE, SIG_IGN);
  // USR1, ignore it.
  signal (SIGUSR1, SIG_IGN);
  #endif

  // Load previously saved references, if any.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.load ();
  ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
  references.fill_store (projectconfig->language_get());

  // Display project notes.
  notes_redisplay ();

  // Start the GUI updater.
  g_timeout_add (100, GSourceFunc (on_gui_timeout), gpointer (this));
  
  // Start bibledit http responder.
  g_timeout_add (300, GSourceFunc (on_check_httpd_timeout), gpointer (this));

  // Initialize content manager subsystem.
  git_initialize_subsystem ();
  git_update_intervals_initialize ();

  // Show main window.
  gtk_widget_show (mainwindow);
  // Present it because some window managers do not present it when started from 
  // a terminal.
  gtk_window_present (GTK_WINDOW (mainwindow));
  
  // Interprocess communications.
  extern InterprocessCommunication * ipc;
  ipc->methodcall_add_signal (ipcctGitJobDescription);
  ipc->methodcall_add_signal (ipcctGitTaskDone);
  g_signal_connect ((gpointer) ipc->method_called_signal, "clicked", G_CALLBACK (on_ipc_method_called), gpointer(this));
}


MainWindow::~MainWindow ()
{
  // Save the size and position of the program window.
  // The get_position function is broken, see the gtk documentation for reasons.
  // Anyway, for just now we'll leave the variables in configuration, and we hope
  // that it will be fixed later. The problem now is that the function will
  // return the value that was set with move(x,y). If the user moves the window
  // that is not reflected here, but it will keep returning the values set
  // with move(x,y).
  // If you want to change the default values when Bibledit starts, edit
  // the configuration file.
  ScreenLayoutDimensions dimensions (mainwindow, hpaned1, vpaned1, vpaned_references);
  dimensions.save ();
  // Hide bibledit. Note: This is done after saving the window's size/position
  // to have that done properly.
  gtk_widget_hide (mainwindow);
  
  // No ipc signals anymore.
  extern InterprocessCommunication * ipc;
  ipc->methodcall_remove_all_signals ();

  // Save text in editors.
  editorsgui->save ();
  // Save references.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.get_loaded ();
  references.save ();  
  // Stop possible thread that is displaying notes.
  stop_displaying_more_notes ();
  // Destroy the Outpost
  delete windowsoutpost;
  // Destroy keyterms gui object if it is there.
  destroy_keyterms_object ();
  // Destroy the resources gui if it is there.
  destroy_resources_object ();
  // Finalize content manager subsystem.
  git_finalize_subsystem ();
  // Destroy Outline
  outline_destroy ();
  // Destroy the Styles.
  delete styles;
  // Destroy editors.
  delete editorsgui;
  // Destroy merge GUI.
  delete mergegui;
  // Do shutdown actions.
  shutdown_actions ();
  // Destroying the window is done by gtk itself.
}


int MainWindow::run ()
{
  return gtk_dialog_run (GTK_DIALOG (mainwindow));
}


/*
|
|
|
|
|
Initialization
|
|
|
|
|
 */


void MainWindow::enable_or_disable_widgets (bool enable)
{
  // Set some widgets (in)sensitive depending on whether a project is open.
  if (close1) gtk_widget_set_sensitive (close1, enable);
  if (properties1) gtk_widget_set_sensitive (properties1, enable);
  if (import1) gtk_widget_set_sensitive (import1, enable);
  if (notes2) gtk_widget_set_sensitive (notes2, enable);
  if (menuitem_edit) gtk_widget_set_sensitive (menuitem_edit, enable);
  if (file_references) gtk_widget_set_sensitive (file_references, enable);
  if (export_project) gtk_widget_set_sensitive (export_project, enable);
  if (parallel_bible) gtk_widget_set_sensitive (parallel_bible, enable);
  if (project_changes) gtk_widget_set_sensitive (project_changes, enable);
  if (menuitem_view) gtk_widget_set_sensitive (menuitem_view, enable);
  if (menuitem_goto) gtk_widget_set_sensitive (menuitem_goto, enable);
  if (compare_with1) gtk_widget_set_sensitive (compare_with1, enable);
  if (copy_project_to) gtk_widget_set_sensitive (copy_project_to, enable);
  if (print_references) gtk_widget_set_sensitive (print_references, enable);
  if (print_project) gtk_widget_set_sensitive (print_project, enable);
  if (insert1) gtk_widget_set_sensitive (insert1, enable);
  if (check1) gtk_widget_set_sensitive (check1, enable);
  if (menutools) gtk_widget_set_sensitive (menutools, enable);
  if (preferences_remote_git_repository) gtk_widget_set_sensitive (preferences_remote_git_repository, enable);  
  if (preferences_planning) gtk_widget_set_sensitive (preferences_planning, enable);
  if (project_backup) gtk_widget_set_sensitive (project_backup, enable);
  if (file_projects_merge) gtk_widget_set_sensitive (file_projects_merge, enable);
  navigation.sensitive (enable);
}


/*
|
|
|
|
|
Menu callbacks
|
|
|
|
|
 */


void MainWindow::on_open1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->open ();
}


void MainWindow::open ()
// Do the logic for opening a project.
{
  // Get new project, bail out if none.
  ustring newproject;
  if (!project_select (newproject)) 
    return;
  editorsgui->open (newproject, -1);
}


void MainWindow::on_close1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->close ();
}


void MainWindow::close ()
{
  // Close focused editor.
  editorsgui->close ();
}


void MainWindow::on_new1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->newproject ();
}


void MainWindow::newproject ()
{
  git_command_pause (true);
  ProjectDialog projectdialog (true);
  if (projectdialog.run () == GTK_RESPONSE_OK) {
    editorsgui->open (projectdialog.newprojectname, -1);
  }
  git_command_pause (false);
}


void MainWindow::on_print_project_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_print_project ();
}


void MainWindow::on_print_project ()
{
  // Save any changes.
  editorsgui->save ();
  // Run the dialog for printing the project to pdf.
  {
    PrintProjectDialog dialog (0);
    if (dialog.run () != GTK_RESPONSE_OK)
      return;
  }
  extern Settings * settings;
  ProjectMemory projectmemory (settings->genconfig.project_get(), true);
  PrintProject printproject (&projectmemory);
  printproject.print();
}


void MainWindow::on_properties1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->editproject ();
}


void MainWindow::editproject ()
{
  git_command_pause (true);
  editorsgui->save ();
  // Show project dialog.
  ProjectDialog projectdialog (false);
  if (projectdialog.run () == GTK_RESPONSE_OK) {
    // Reload dictionaries.
    editorsgui->reload_dictionaries ();
    // As anything could have been changed to the project, reopen it.
    reload_project ();
  }
  git_command_pause (false);
}


void MainWindow::on_delete1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->deleteproject ();
}


void MainWindow::deleteproject ()
{
  // Get all projects, leave the current one and the non-editable ones out.
  extern Settings * settings;
  vector <ustring> all_projects = projects_get_all ();
  vector<ustring> projects;
  for (unsigned int i = 0; i < all_projects.size(); i++) {
    bool include = true;
    if (all_projects[i] == settings->genconfig.project_get()) include = false;
    ProjectConfiguration * projectconfig = settings->projectconfig (all_projects[i]);
    if (!projectconfig->editable_get ()) include = false;    
    if (include) projects.push_back (all_projects[i]);
  }
  // User interface.
  ListviewDialog dialog ("Delete project", projects, "", true, NULL);
  if (dialog.run () == GTK_RESPONSE_OK) {
    int result;
    result = gtkw_dialog_question (mainwindow, "Are you sure you want to delete project " + dialog.focus + "?");
    if (result == GTK_RESPONSE_YES) {
      result = gtkw_dialog_question (mainwindow, "Are you really sure to delete project " + dialog.focus + ", something worth perhaps years of work?");
    }
    if (result == GTK_RESPONSE_YES) {
      project_delete (dialog.focus);
    }
  }
}


void MainWindow::on_quit1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  gtk_main_quit ();
}


void MainWindow::on_system_log1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->viewlog ();
}


void MainWindow::viewlog ()
{
  ShowScriptDialog showscript (0);
  showscript.run ();
}


void MainWindow::on_help_context_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_help_context ();
}


void MainWindow::on_help_context ()
{
  help_open (NULL, gpointer ("none"));
}


void MainWindow::on_help_main_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_help_main ();
}


void MainWindow::on_help_main ()
{
  htmlbrowser ("localhost:51516", true);
}


void MainWindow::on_about1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->showabout ();
}


void MainWindow::showabout ()
{
  gtk_show_about_dialog (GTK_WINDOW (mainwindow), 
                         "version", PACKAGE_VERSION, 
                         "website", PACKAGE_BUGREPORT, 
                         "copyright", "Copyright (©) 2003-2008 Teus Benschop",
                         "license", "This program is free software; you can redistribute it and/or modify\n"
                                    "it under the terms of the GNU General Public License as published by\n"
                                    "the Free Software Foundation; either version 3 of the License, or\n"
                                    "(at your option) any later version.\n"
                                    "\n"
                                    "This program is distributed in the hope that it will be useful,\n"
                                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                                    "GNU General Public License for more details.\n"
                                    "\n"
                                    "You should have received a copy of the GNU General Public License\n"
                                    "along with this program; if not, write to the Free Software\n"
                                    "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.\n",
                         NULL);
}


void MainWindow::on_undo1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_undo ();
}


void MainWindow::menu_undo ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (editor) editor->undo ();
}


void MainWindow::on_redo1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_redo ();
}


void MainWindow::menu_redo ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (editor) editor->redo ();
}


void MainWindow::on_edit1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_edit ();
}


void MainWindow::menu_edit ()
{
  // Set the sensitivity of some items in the Edit menu.
  Editor * editor = editorsgui->focused_editor ();
  if (editor) {
    gtk_widget_set_sensitive (copy_without_formatting, editor->has_focus ());
    gtk_widget_set_sensitive (undo1, editor->can_undo ());
    gtk_widget_set_sensitive (redo1, editor->can_redo ());
  }

  // Sensitivity of the clipboard operations.
  // There is also the "owner-change" signal of the clipboard, but this is not
  // a reliable indicator for pastable content.
  bool cut = true;
  bool copy = true;
  bool paste = true;
  gtk_widget_set_sensitive (cut1, cut);
  gtk_widget_set_sensitive (copy1, copy);
  gtk_widget_set_sensitive (paste1, paste);

  // Enable/disable based on whether we're editing a note.
  bool enable;
  enable = (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) == 1);
  // References can only be taken from a note when it is opened.
  gtk_widget_set_sensitive (get_references_from_note, enable);
  
  // The Bible notes can only be edited when the cursor is in a note text.
  enable = false;
  if (editor)
    if (editor->last_focused_type () == etvtNote)
      enable = true;
  gtk_widget_set_sensitive (edit_bible_note, enable);
}


void MainWindow::on_find_and_replace1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_replace ();
}


void MainWindow::menu_replace ()
{
  extern Settings * settings;
  // Before finding, save the current file.
  editorsgui->save ();
  // Start find/replace dialog.
  vector <Reference> results;
  {
    ReplaceDialog replacedialog (0);
    if (replacedialog.run () == GTK_RESPONSE_OK) {
      results.assign (replacedialog.results.begin (), replacedialog.results.end ());
      References references (liststore_references, treeview_references, treecolumn_references);
      references.set_references (replacedialog.results);
      ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
      references.fill_store (projectconfig->language_get());
    } else {
      return;
    }
  }
  // Replace text.
  if (results.size ()) {
    ustring prj = settings->genconfig.project_get();
    ReplacingDialog replacedialog (results);
    replacedialog.run ();
    reload_project ();
  } else {
    gtkw_dialog_info (mainwindow, "There was nothing to replace");
  }
}


void MainWindow::on_findspecial1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_findspecial ();
}


void MainWindow::menu_findspecial ()
{
  // Before finding, save the current file.
  editorsgui->save ();
  // Switch to the References Area.
  on_file_references ();
  // Start dialog.
  {
    SearchSpecialDialog dialog (&bibletime);
    if (dialog.run () != GTK_RESPONSE_OK)
      return;
  }
  // Carry out the search. 
  search_string (liststore_references, treeview_references, treecolumn_references, &bibletime);
}


void MainWindow::on_import1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->menu_import ();
}


void MainWindow::menu_import ()
{
  ImportTextDialog dialog (0);
  int result = dialog.run ();
  if (result == GTK_RESPONSE_OK) {
    extern Settings * settings;
    ustring prj = settings->genconfig.project_get();
    close ();
    editorsgui->open (prj, 1);
  }
}


gboolean MainWindow::on_mainwindow_delete_event (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
/*
  This solves a bug:
  When quitting the program though the menu, all went fine.
  But when quitting the program by clicking on the cross at the top of the program,
  a segmentation fault occurred.
  This segmentation occurred in the destructor of the object MainWindow.
  In the destructor properties of GTK were accessed, which obviously were 
  already in the process of being destroyed.
  This is the solution:
  When the window receives the delete_event (somebody clicks on the cross to
  close the program), the window is not closed, but instead a timeout is 
  installed that calls qtk_main_quit after a short while.
  The destructor of the object MainWindow can not access the properties of GTK
  without a segmentation fault.
  */
  g_timeout_add (10, GSourceFunc (gtk_main_quit), user_data);
  return true;
}


gboolean MainWindow::on_mainwindow_focus_in_event (GtkWidget *widget, GdkEventFocus *event, gpointer user_data)
{
  ((MainWindow *) user_data)->on_mainwindow_focus_in (event);
  return FALSE;
}


void MainWindow::on_mainwindow_focus_in (GdkEventFocus *event)
{
  // When bibledit receives focus, immediately after that synchronize it with 
  // external programs.
  // No longer used now.
}


void MainWindow::on_insert1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_insert ();
}


void MainWindow::on_menu_insert ()
// Sets the labels of the underlying menu items right.
{
  // Write the proper labels.
  Editor * editor = editorsgui->focused_editor ();
  extern Settings * settings;
  ustring std_txt = "Standard text ";
  ustring label;
  label = std_txt + "_1: " + settings->genconfig.edit_note_standard_text_one_get ();
  if (standard_text_1) gtk_label_set_text_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (standard_text_1))), label.c_str());
  label = std_txt + "_2: " + settings->genconfig.edit_note_standard_text_two_get ();
  if (standard_text_2) gtk_label_set_text_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (standard_text_2))), label.c_str());
  label = std_txt + "_3: " + settings->genconfig.edit_note_standard_text_three_get ();
  if (standard_text_3) gtk_label_set_text_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (standard_text_3))), label.c_str());
  label = std_txt + "_4: " + settings->genconfig.edit_note_standard_text_four_get ();
  if (standard_text_4) gtk_label_set_text_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (standard_text_4))), label.c_str());
  // Enable or disable depending on situation.
  bool enable;
  enable = (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) == 1);
  if (standard_text_1) gtk_widget_set_sensitive (standard_text_1, enable);
  if (standard_text_2) gtk_widget_set_sensitive (standard_text_2, enable);
  if (standard_text_3) gtk_widget_set_sensitive (standard_text_3, enable);
  if (standard_text_4) gtk_widget_set_sensitive (standard_text_4, enable);
  // Allow inserting reference when we edit a note and the reference is different 
  // from any of the references loaded already.
  enable = false;
  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) == 1) {
    // Get all references from the note.
    vector<Reference> references;
    vector<ustring> messages;
    notes_get_references_from_editor (note_editor->textbuffer_references, references, messages);
    // See whether the current reference is already in it.
    bool already_in = false;
    for (unsigned int i = 0; i < references.size(); i++) {
      if (editor) 
        if (references[i].equals (editor->current_reference)) 
          already_in = true;
    }
    if (!already_in) {
      // No, not yet, enable menu, so user can add it.
      enable = true;
    }      
  }
  // Update menu.
  ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
  if (editor) {
    label = "_Add " + editor->current_reference.human_readable (projectconfig->language_get()) + " to project note";
  } else {
    enable = false;
  }
  if (current_reference1) gtk_label_set_text_with_mnemonic (GTK_LABEL (gtk_bin_get_child (GTK_BIN (current_reference1))), label.c_str());
  if (current_reference1) gtk_widget_set_sensitive (current_reference1, enable);
  // Inserting special character.
  gtk_widget_set_sensitive (insert_special_character, (editor && editor->has_focus ()));
}


void MainWindow::on_menuitem_view_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menuitem_view ();
}


void MainWindow::on_menuitem_view ()
{
}


void MainWindow::on_notes_preferences_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_preferences ();
}


void MainWindow::on_notes_preferences ()
{
  NotesDialog dialog (0);
  dialog.run();
}


void MainWindow::on_copy_project_to_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_copy_project_to ();
}


void MainWindow::on_copy_project_to ()
{
  extern Settings * settings;
  EntryDialog dialog ("New project name", 
                      "Enter a name of a non-existent project\nwhere this project will be copied to.",
                      settings->genconfig.project_get());
  if (dialog.run () == GTK_RESPONSE_OK) {
    // Does the project exist?
    if ((project_exists (dialog.entered_value)) || (dialog.entered_value == "data")) {
      // Yes, give message that project exists.
      ustring error = "Project ";
      error.append (dialog.entered_value);
      error.append (" already exists.");
      error.append ("\nIf you still intend to copy the project,");
      error.append ("\ndelete project ");
      error.append (dialog.entered_value);
      error.append (" first.");
      gtkw_dialog_error (mainwindow, error);
    } else {
      // Ok, go ahead with the copy.
      project_copy (settings->genconfig.project_get(), dialog.entered_value);
      // Give message when through.
      ustring message;
      message.append ("The project has been copied to a new project\n");
      message.append ("named ");
      message.append (dialog.entered_value);
      message.append (".");
      gtkw_dialog_info (mainwindow, message);
    }
  }
}


void MainWindow::on_compare_with1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_compare_with ();
}


void MainWindow::on_compare_with ()
{
  editorsgui->save ();
  git_command_pause (true);
  References references (liststore_references, treeview_references, treecolumn_references);
  CompareDialog dialog (&references);
  dialog.run ();
  git_command_pause (false);
}


void MainWindow::on_printingprefs_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_printing_preferences ();
}


void MainWindow::on_printing_preferences ()
{
  PrintPreferencesDialog dialog (0);
  dialog.run();
}


void MainWindow::on_formatting_objects_processor_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_formatter ();
}


void MainWindow::on_formatter ()
{
  FormatterDialog dialog (0);
  dialog.run();
}


void MainWindow::on_parallel_bible_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_parallel_bible ();
}


void MainWindow::on_parallel_bible ()
{
  {
    ParallelBibleDialog dialog (0);
    if (dialog.run () != GTK_RESPONSE_OK)
      return;
  }
  editorsgui->save ();
  view_parallel_bible_pdf ();
}


void MainWindow::on_screen_layout_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_screen_layout ();
}


void MainWindow::on_screen_layout ()
{
  ScreenLayoutDialog dialog (0);
  dialog.run ();
}


void MainWindow::on_prefs_books_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_prefs_books ();
}


void MainWindow::on_prefs_books ()
{
  extern Settings * settings;
  BookDialog dialog (settings->genconfig.project_get());
  if (dialog.run () == GTK_RESPONSE_OK) {
    ustring project = settings->genconfig.project_get();
    close ();
    editorsgui->open (project, 1);
  }
}


void MainWindow::on_preferences_tidy_text_activate (GtkMenuItem *menuitem,  gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_tidy_text ();
}


void MainWindow::on_preferences_tidy_text ()
{
  TidyDialog dialog (0);
  dialog.run ();
}


/*
|
|
|
|
|
Navigation
|
|
|
|
|
 */


void MainWindow::on_navigation_new_reference_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_navigation_new_reference ();
}


void MainWindow::on_navigation_new_reference ()
{
  extern Settings * settings;
  settings->genconfig.book_set (navigation.reference.book);
  settings->genconfig.chapter_set (convert_to_string (navigation.reference.chapter));
  settings->genconfig.verse_set (navigation.reference.verse);
  go_to_new_reference ();
  if (outline) outline->goto_reference (settings->genconfig.project_get(), navigation.reference);
  // Optionally display the parallel passages in the reference area.
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (parallel_passages1))) {
    parallel_passages_display (navigation.reference, liststore_references, treeview_references, treecolumn_references);
  }
}


void MainWindow::on_next_verse_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_next_verse ();
}


void MainWindow::goto_next_verse ()
{
  navigation.nextverse();
}


void MainWindow::on_previous_verse_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_previous_verse ();
}


void MainWindow::goto_previous_verse ()
{
  navigation.previousverse();
}


void MainWindow::on_next_chapter_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_next_chapter ();
}


void MainWindow::goto_next_chapter ()
{
  navigation.nextchapter();
}


void MainWindow::on_previous_chapter_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_previous_chapter ();
}


void MainWindow::goto_previous_chapter ()
{
  navigation.previouschapter();
}


void MainWindow::on_next_book_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_next_book ();
}


void MainWindow::goto_next_book ()
{
  navigation.nextbook();
}


void MainWindow::on_previous_book_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_previous_book ();
}


void MainWindow::goto_previous_book ()
{
  navigation.previousbook();
}


void MainWindow::on_reference_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->goto_reference_interactive ();
}


void MainWindow::goto_reference_interactive ()
{
  Editor * editor = editorsgui->focused_editor ();
  GotoReferenceDialog dialog (editor->current_reference.book, editor->current_reference.chapter, editor->current_reference.verse);
  if (dialog.run () == GTK_RESPONSE_OK) {
    if (dialog.newreference) {
      navigation.display (dialog.reference);
    }
  }
}


void MainWindow::go_to_new_reference ()
// This starts the procedure to carries out a requested change of reference.
{
  // Let the editor(s) show the right reference.
  editorsgui->go_to (navigation.reference);

  // Create a reference for the external programs.
  // These do not take verses like 10a or 10-12, but only numbers like 10 or 12.
  Reference goto_reference (navigation.reference.book, navigation.reference.chapter, number_in_string (navigation.reference.verse));
  
  // Send it to the external programs.
  extern Settings * settings;
  if (settings->genconfig.reference_exchange_send_to_bibleworks_get ())
    windowsoutpost->BibleWorksReferenceSet (goto_reference);
  if (settings->genconfig.reference_exchange_send_to_santafefocus_get ())
    windowsoutpost->SantaFeFocusReferenceSet (goto_reference);
  if (settings->genconfig.reference_exchange_send_to_bibledit_resource_viewer_get ())
    if (resourcesgui) resourcesgui->go_to (goto_reference);

  // Update the notes view.
  notes_redisplay ();
}

  
void MainWindow::on_new_verse_signalled (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_new_verse ();
}


void MainWindow::on_new_verse ()
/*
When the cursor has moved, the navigation system needs to be updated
so that it shows the right reference. If the user was, for example
on Matthew 1:10, and the cursor moves, the move might have brought him
to another reference, though this is not necessarily so. Therefore, as we 
don't know where the user is now after the cursor moved, we need to find
it out. The book is known, the chapter is known, because both stay the same.
The only thing we don't know is the verse. 
*/
{
  Editor * editor = editorsgui->focused_editor ();
  if (editor) {
    Reference reference (navigation.reference.book, navigation.reference.chapter, editor->verse_number_get ());
    navigation.display (reference);
    if (outline) outline->goto_reference (editor->project, navigation.reference);
  }
}


void MainWindow::on_synchronize_other_programs2_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_synchronize_other_programs ();
}


void MainWindow::on_synchronize_other_programs ()
{
  // Get the focused editor. If none, bail out.
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  // Send the reference.
  extern Settings * settings;
  if (settings->genconfig.reference_exchange_send_to_bibleworks_get ()) {
    windowsoutpost->BibleWorksReferenceSet (editor->current_reference);
  }
  if (settings->genconfig.reference_exchange_send_to_santafefocus_get ()) {
    windowsoutpost->SantaFeFocusReferenceSet (editor->current_reference);
  }
  if (settings->genconfig.reference_exchange_send_to_bibletime_get ()) {
    bibletime.sendreference (editor->current_reference);
  }
  if (settings->genconfig.reference_exchange_send_to_bibledit_resource_viewer_get ()) {
    if (resourcesgui) {
      resourcesgui->go_to (editor->current_reference);
    }
  }
}


void MainWindow::on_text_area1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_text_area_activate();
}


void MainWindow::on_text_area_activate ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (editor) gtk_widget_grab_focus (editor->last_focused_textview ());
}


void MainWindow::on_goto_bible_notes_area1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_bible_notes_area_activate();
}


void MainWindow::on_bible_notes_area_activate ()
{
  switch (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_notes_area))) {
    case 0:
    {
      Editor * editor = editorsgui->focused_editor ();
      if (editor) gtk_widget_grab_focus (editor->last_focused_textview ());
      break;
    }
    case 1:
      gtk_widget_grab_focus (textview_keyterm_text);      
      break;
  }
}


void MainWindow::on_tools_area1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_tools_area_activate();
}


void MainWindow::on_tools_area_activate ()
{
  // Get the page that now shows.
  ToolsAreaPageNumberType page = (ToolsAreaPageNumberType) gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_tools));
  
  // See if this page has focus.
  bool focused = false;
  switch (page) {
    case tapntReferences:
    {
      focused = GTK_WIDGET_HAS_FOCUS (treeview_references);
      break;
    }
    case tapntStyles:
    {
      focused = GTK_WIDGET_HAS_FOCUS (styles->treeview);
      break;
    }
    case tapntProjectNote:
    {
      focused = GTK_WIDGET_HAS_FOCUS (combobox_note_category)
              | GTK_WIDGET_HAS_FOCUS (textview_note_references)
              | GTK_WIDGET_HAS_FOCUS (combobox_note_project)
              | GTK_WIDGET_HAS_FOCUS (textview_note_logbook);
      break;
    }
    case tapntKeyterms:
    {
      focused = (keytermsgui && keytermsgui->focused ());
      break;
    }
    case tapntOutline:
    {
      focused = (outline && outline->focused());
      break;
    }
    case tapntResources:
    {
      focused = true;
      break;
    }
    case tapntMerge:
    {
      focused = mergegui->has_focus ();
      break;
    }
    case tapntEnd:
    {
      focused = true;
      break;
    }
  }

  // If the page is focused already, switch to the next one.
  if (focused) {
    
    // Iterate through the possible tabs to find the next visible one.
    bool pageshows = false;
    int iterations = 0;
    while (!pageshows && (iterations < 100)) {
      iterations++;
      // Take next page if available. Else take the first one.
      int ipage = page;
      ipage++;
      page = (ToolsAreaPageNumberType) ipage;
      // See if that next page is visible.
      switch (page) {
        case tapntReferences:
        case tapntStyles:
        case tapntProjectNote:
        case tapntKeyterms:
        case tapntOutline:
        case tapntResources:
        case tapntMerge:
          pageshows = GTK_WIDGET_VISIBLE (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), page));
          break;
        case tapntEnd:
          pageshows = false;
          page = (ToolsAreaPageNumberType) -1;
          break;
      }
    }
  
    // If no next page was available, bail out.
    if (!pageshows) return;  

    // Activate the next page.  
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), page);
    
  }    
  
  // Focus the widget belonging to the page now showing.
  switch (page) {
    case tapntReferences:
      gtk_widget_grab_focus (treeview_references);
      break;
    case tapntStyles:
      gtk_widget_grab_focus (styles->treeview);
      break;
    case tapntProjectNote:
      gtk_widget_grab_focus (textview_note_references);      
      break;
    case tapntKeyterms:
      keytermsgui->focus();
      break;
    case tapntOutline:
      outline->focus();      
      break;
    case tapntResources:
      gtk_widget_grab_focus (vbox_resources);
      break;
    case tapntMerge:
      mergegui->give_focus();
      break;
    case tapntEnd:
      break;
  }
}


void MainWindow::on_notes_area1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_area_activate();
}


void MainWindow::on_notes_area_activate ()
{
  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) == 0)
    gtk_widget_grab_focus (textview_notes);
  else
    gtk_widget_grab_focus (textview_note);
}


void MainWindow::on_goto_next_project_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->editorsgui->next_previous_project (true);
}


void MainWindow::on_goto_previous_project_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->editorsgui->next_previous_project (false);
}


/*
|
|
|
|
|
Clipboard
|
|
|
|
|
 */


void MainWindow::on_cut1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_cut ();
}


void MainWindow::on_cut ()
{
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (editorsgui->has_focus ()) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      gtk_clipboard_set_text (clipboard, editor->text_get_selection ().c_str(), -1);
      editor->text_erase_selection ();
    }
  }
  if (GTK_WIDGET_HAS_FOCUS (textview_notes))
    gtk_text_buffer_cut_clipboard (textbuffer_notes, clipboard, true);
  if (GTK_WIDGET_HAS_FOCUS (textview_note))
    gtk_text_buffer_cut_clipboard (note_editor->textbuffer_note, clipboard, true);
  if (GTK_WIDGET_HAS_FOCUS (textview_note_references))
    gtk_text_buffer_cut_clipboard (note_editor->textbuffer_references, clipboard, true);
}


void MainWindow::on_copy1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_copy ();
}


void MainWindow::on_copy ()
{
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (editorsgui->has_focus ()) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      // In case of the text editor, the USFM code is copied, not the plain text. 
      gtk_clipboard_set_text (clipboard, editor->text_get_selection ().c_str(), -1);
    }
  }
  if (GTK_WIDGET_HAS_FOCUS (textview_notes))
    gtk_text_buffer_copy_clipboard (textbuffer_notes, clipboard);
  if (GTK_WIDGET_HAS_FOCUS (textview_note))
    gtk_text_buffer_copy_clipboard (note_editor->textbuffer_note, clipboard);
  if (GTK_WIDGET_HAS_FOCUS (textview_note_references))
    gtk_text_buffer_copy_clipboard (note_editor->textbuffer_references, clipboard);
  if (keytermsgui) {
    if (GTK_WIDGET_HAS_FOCUS (keytermsgui->textview_check_text)) {
      GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (keytermsgui->textview_check_text));
      gtk_text_buffer_copy_clipboard (buffer, clipboard);
      keytermsgui->copy_clipboard ();
    }
  }
  if (resourcesgui) {
    ToolsAreaPageNumberType page = (ToolsAreaPageNumberType) gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_tools));
    if (page == tapntResources) {
      Resource * resource = resourcesgui->focused_resource ();
      if (resource) {
        resource->copy ();
      }
    }
  }
}


void MainWindow::on_copy_without_formatting_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_copy_without_formatting ();
}


void MainWindow::on_copy_without_formatting ()
{
  GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  if (editorsgui->has_focus ()) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      gtk_text_buffer_copy_clipboard (editor->last_focused_textbuffer (), clipboard);
    }
  }
}


void MainWindow::on_paste1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_paste ();
}


void MainWindow::on_paste ()
{
  // Get the clipboard.
  GtkClipboard *clipboard;
  clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
  // Bail out if no text is available.
  if (!gtk_clipboard_wait_is_text_available (clipboard)) return;
  // Paste text in the focused textview.  
  if (editorsgui->has_focus ()) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      gchar * text = gtk_clipboard_wait_for_text (clipboard);
      if (text) {
        editor->text_insert (text);
        g_free (text);
      }
    }
  }
  if (GTK_WIDGET_HAS_FOCUS (textview_notes))
    gtk_text_buffer_paste_clipboard (textbuffer_notes, clipboard, NULL, true);
  if (GTK_WIDGET_HAS_FOCUS (textview_note))
    gtk_text_buffer_paste_clipboard (note_editor->textbuffer_note, clipboard, NULL, true);
  if (GTK_WIDGET_HAS_FOCUS (textview_note_references))
    gtk_text_buffer_paste_clipboard (note_editor->textbuffer_references, clipboard, NULL, true);
}


/*
|
|
|
|
|
Tools Area
|
|
|
|
|
 */


void MainWindow::on_notebook_tools_switch_page (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data)
{
  ((MainWindow *) user_data)->notebook_tools_switch_page (page_num);  
}


void MainWindow::notebook_tools_switch_page (guint page_num)
{
  // Store the page number in the configuration.
  extern Settings * settings;
  settings->genconfig.tools_area_page_number_set (page_num);
  // If the user switches to another page, and the note is being edited, do not 
  // return to the page displayed previously, but leave it as it is.
  if (note_editor) {
    note_editor->previous_tools_page = -1;
  }
  // If we're on the keyterms page, create the object if need be, and do
  // other tasks.
  unsigned int notebook_notes_area_page_num = 0;
  if (page_num == tapntKeyterms) {
    editorsgui->save ();
    activate_keyterms_object ();
    notebook_notes_area_page_num = 1;
  }
  // If we're on the resources page create the object if need be.
  if (page_num == tapntResources) {
    activate_resources_object ();
  }
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_notes_area), notebook_notes_area_page_num);
  // Initialize outline system
  if (page_num == tapntOutline) outline_initialize ();
  // Whether the outlines are produced.
  if (outline) outline->operate = (page_num == tapntOutline);
  // Tell the merge object whether we're on the merge page.
  mergegui->set_active (page_num == tapntMerge);
}


void MainWindow::on_file_references_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_references ();
}


void MainWindow::on_file_references ()
{
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntReferences);
}




/*
|
|
|
|
|
List store and reference handling
|
|
|
|
|
 */


gboolean MainWindow::on_treeview_references_key_press_event (GtkWidget * widget, GdkEventKey * event, gpointer user_data)
{
  /* Pressing Return on the keyboard, or Enter on the numerical keypad
   * make us go to the reference.
   */
  if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
    ((MainWindow *) user_data)->on_list_goto ();
  // Pressing Delete takes the reference(s) out that have been selected.
  if (event->keyval == GDK_Delete || event->keyval == GDK_KP_Delete)
    ((MainWindow *) user_data)->on_delete_references ();
  return FALSE;
}


gboolean MainWindow::on_treeview_references_button_press_event (GtkWidget * widget, GdkEventButton * event, gpointer user_data)
{
  // Double-clicking a references makes us go to the reference.
  if (event->type == GDK_2BUTTON_PRESS) {
    ((MainWindow *) user_data)->on_list_goto ();
    return true;
  }
  // Popup menu handler.
  if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
    ((MainWindow *) user_data)->show_references_popup_menu (widget, event);
    return true;
  }  
  return false;
}


void MainWindow::on_list_goto ()
// Handler for when the user pressed Enter in the list and wants to go to a reference.
{
  // Get the Editor. If none, bail out.
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  try
  {
    // Get the string the user selected.
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_references));
    gtk_tree_selection_selected_foreach (selection, mainwindow_selection_foreach_function, gpointer(this));
    // Jump to the reference.
    Reference reference (selected_reference.book, selected_reference.chapter, selected_reference.verse);
    navigation.display (reference);
    editor->go_to_new_reference_highlight = true;
  }
  catch (exception & ex)
  {
    cerr << ex.what () << endl;
  }
}


void MainWindow::mainwindow_selection_foreach_function (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, gpointer data)
{
  unsigned int book, chapter;
  gchar *verse;
  gtk_tree_model_get (model, iter, 2, &book, 3, &chapter, 4, &verse, -1);
  ((MainWindow *) data)->selected_reference.book = book;
  ((MainWindow *) data)->selected_reference.chapter = chapter;
  ((MainWindow *) data)->selected_reference.verse = verse;
  g_free (verse);
}


void MainWindow::on_open_references1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_open_references ();
}


void MainWindow::on_open_references ()
{
  // Settings.
  extern Settings * settings;
  // Ask for a file.
  ustring filename = gtkw_file_chooser_open (mainwindow, "Open File", settings->genconfig.references_file_get ());
  if (filename.empty())
    return;
  // Allow for up to three words to search for in these references.
  ustring searchword1, searchword2, searchword3;
  vector <ustring> import_references_searchwords = settings->session.import_references_searchwords;
  for (unsigned int i = 0; i < import_references_searchwords.size(); i++) {
    if (i == 0) searchword1 = import_references_searchwords[i];
    if (i == 1) searchword2 = import_references_searchwords[i];
    if (i == 2) searchword3 = import_references_searchwords[i];
  }
  Entry3Dialog dialog2 ("Search for", true, "Optionally enter _1st searchword", searchword1, "Optionally enter _2nd searchword", searchword2, "Optioanlly enter _3rd searchword", searchword3);
  int result = dialog2.run();
  if (result == GTK_RESPONSE_OK) {
    searchword1 = dialog2.entered_value1;
    searchword2 = dialog2.entered_value2;
    searchword3 = dialog2.entered_value3;
    import_references_searchwords.clear();
    if (!searchword1.empty())
      import_references_searchwords.push_back (searchword1);
    if (!searchword2.empty())
      import_references_searchwords.push_back (searchword2);
    if (!searchword3.empty())
      import_references_searchwords.push_back (searchword3);
    settings->session.import_references_searchwords = import_references_searchwords;
    settings->genconfig.references_file_set (filename);
    References references (liststore_references, treeview_references, treecolumn_references);
    references.load (settings->genconfig.references_file_get ());
    ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
    references.fill_store (projectconfig->language_get());
    if (import_references_searchwords.size() > 0) {
      settings->session.highlights.clear();
      for (unsigned int i = 0; i < import_references_searchwords.size(); i++) {
        SessionHighlights sessionhighlights (import_references_searchwords[i], false, false, false, false, atRaw, false, false, false, false, false, false, false, false);
        settings->session.highlights.push_back (sessionhighlights);
      }
    }
  }
}


void MainWindow::on_references_save_as_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_save_references ();
}


void MainWindow::on_save_references ()
{
  extern Settings * settings;
  try
  {
    ustring filename = gtkw_file_chooser_save (mainwindow, "", settings->genconfig.references_file_get ());
    if (filename.empty())
      return;
    settings->genconfig.references_file_set (filename);
    References references (liststore_references, treeview_references, treecolumn_references);
    // Hack: set references with a dummy, then load the real ones from the editor.
    vector <Reference> dummy;
    references.set_references (dummy);
    references.get_loaded ();
    references.save (filename);
  }
  catch (exception & ex)
  {
    cerr << "Saving references: " << ex.what () << endl;
  }
}


void MainWindow::on_print_references_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_print_references ();
}


void MainWindow::on_print_references ()
{
  // Activate references area.
  on_file_references ();
  // Save any changes.
  editorsgui->save ();
  {    
    PrintReferencesDialog dialog (0);
    if (dialog.run () != GTK_RESPONSE_OK) return;
  }
  // Load refs from the editor.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.get_loaded();
  vector<Reference> refs;
  references.get_references (refs);
  if (refs.empty()) {
    gtkw_dialog_info (mainwindow, "There are no references to print");
  } else {
    // Run the function for printing the references.
    extern Settings * settings;
    vector <ustring> extra_projects = settings->genconfig.print_references_projects_get ();
    ProjectMemory projectmemory (settings->genconfig.project_get(), true);
    view_parallel_references_pdf (projectmemory, &extra_projects, refs, true, NULL, true);
  }
}


void MainWindow::on_close_references_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_clear_references ();
}


void MainWindow::on_clear_references ()
{
  References references (liststore_references, treeview_references, treecolumn_references);
  references.fill_store ("");
  extern Settings* settings;
  settings->session.highlights.clear();
}


void MainWindow::on_delete_references_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_delete_references ();
}


void MainWindow::on_delete_references ()
{
  // Delete each selected row.
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_references));
  vector < GtkTreeIter > iters;
  gtk_tree_selection_selected_foreach (selection, MainWindow::on_references_collect_iters, gpointer (&iters));
  for (unsigned int i = 0; i < iters.size (); i++)
  {
    GtkTreeIter iter = iters[i];
    gtk_list_store_remove (liststore_references, &iter);
  }
  // Update heading.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.get_loaded ();
  references.set_header ();
}


void MainWindow::on_references_collect_iters (GtkTreeModel * model, GtkTreePath * path, GtkTreeIter * iter, gpointer data)
{
  ((vector < GtkTreeIter > *)data)->push_back (*iter);
}


void MainWindow::on_next_reference1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_next_reference ();
}


void MainWindow::on_next_reference ()
// This goes to the next reference, if there is any.
// If no item has been selected it chooses the first, if it's there.
{
  // Show references area.
  on_file_references ();
  // Select next reference in the list.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.goto_next ();
  // Actually open the reference in the editor.
  on_list_goto ();
}


void MainWindow::on_previous_reference1_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_previous_reference ();
}


void MainWindow::on_previous_reference ()
{
  /* 
   * This goes to the previous reference, if there is any.
   * If no item has been selected it chooses the first, if it's there.
   */
  // Show references area.
  on_file_references ();
  // Select previous reference in the list.
  References references (liststore_references, treeview_references, treecolumn_references);
  references.goto_previous ();
  // Actually open the reference in the editor.
  on_list_goto ();
}


void MainWindow::on_ignored_references1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_ignored_references ();
}


void MainWindow::on_ignored_references ()
{
  vector<ustring> hidden_references;
  hidden_references = references_hidden_ones_load ();
  EditListDialog dialog (&hidden_references, "Hidden references", 
    "of references and comments that will never be shown in the reference area.",
    true, false, true, false, false, false, false, NULL);
  if (dialog.run() == GTK_RESPONSE_OK)
    references_hidden_ones_save (hidden_references);
}


gboolean MainWindow::on_treeview_references_popup_menu (GtkWidget *widget, gpointer user_data)
{
  ((MainWindow *) user_data)->treeview_references_popup_menu (widget);
  return true; // Do not call the original handler.
}


void MainWindow::treeview_references_popup_menu (GtkWidget *widget)
{
  show_references_popup_menu (widget, NULL);
}


void MainWindow::on_reference_hide_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_reference_hide ();
}


void MainWindow::on_reference_hide ()
// Deals with hiding references.
{
  // Load currently hidden references.
  vector <ustring> hidden_references = references_hidden_ones_load ();
  // Get the model.
  GtkTreeModel * model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_references));
  // Get all selected iterators.
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_references));
  vector < GtkTreeIter > iters;
  gtk_tree_selection_selected_foreach (selection, MainWindow::on_references_collect_iters, gpointer (&iters));
  // Get the strings describing the references, and add them to the ones already loaded.
  for (unsigned int i = 0; i < iters.size (); i++)
  {
    ustring hidden_reference;
    gint book, chapter;
    gchar *verse;
    gchar *comment;
    gtk_tree_model_get (model, &iters[i], 1, &comment, 2, &book, 3, &chapter, 4, &verse, -1);
    Reference reference (book, chapter, verse);
    hidden_reference = reference.human_readable ("");
    hidden_reference.append (" ");
    hidden_reference.append (comment);
    g_free (verse);
    g_free (comment);
    hidden_references.push_back (hidden_reference);
  }
  // Save new list of hidden refs.
  references_hidden_ones_save (hidden_references);
  // Actually delete them from the window, for user feedback.
  on_delete_references ();
}


void MainWindow::show_references_popup_menu (GtkWidget *my_widget, GdkEventButton *event)
{
  int button, event_time;
  if (event) {
    button = event->button;
    event_time = event->time;
  } else {
    button = 0;
    event_time = gtk_get_current_event_time ();
  }
  gtk_menu_popup (GTK_MENU (file_references_menu), NULL, NULL, NULL, NULL, button, event_time);
}


gboolean MainWindow::on_treeview_references_move_cursor (GtkTreeView *treeview, GtkMovementStep step, gint count, gpointer user_data)
{
  return false;
}


void MainWindow::on_treeview_references_cursor_changed (GtkTreeView *treeview, gpointer user_data)
{
  ((MainWindow *) user_data)->treeview_references_display_quick_reference ();
}


void MainWindow::treeview_references_display_quick_reference ()
{
  extern Settings * settings;
  // Clear the quick reference area.
  GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_quick_refs));
  gtk_text_buffer_set_text (buffer, "", 0);
  // Get the model.
  GtkTreeModel * model;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview_references));
  // Get all selected iterators.
  GtkTreeSelection *selection;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview_references));
  vector <GtkTreeIter> iters;
  gtk_tree_selection_selected_foreach (selection, MainWindow::on_references_collect_iters, gpointer (&iters));
  // Get the first selected reference, if there is any.
  if (iters.size() == 0)
    return;
  gint book, chapter;
  gchar *verse;
  gtk_tree_model_get (model, &iters[0], 2, &book, 3, &chapter, 4, &verse, -1);
  // Get / display the verse.
  ustring text;
  text = project_retrieve_verse (settings->genconfig.project_get(), book, chapter, verse);
  gtk_text_buffer_insert_at_cursor (buffer, text.c_str (), -1);
  gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
  // Free memory.
  g_free (verse);
}


/*
|
|
|
|
|
Bibledit Windows Outpost
BibleTime
|
|
|
|
|
*/


bool MainWindow::mainwindow_on_external_programs_timeout (gpointer data)
{
  return ((MainWindow *) data)->on_external_programs_timeout ();
}


bool MainWindow::on_external_programs_timeout ()
{
  // Deal with exchanging references between Bibledit and BibleTime.
  // The trick of the trade here is that we look which of the two made a change.
  // If Bibledit made a change, then it sends its reference to BibleTime.
  // And vice versa.
  // This avoids race conditions, i.e. they keep sending
  // references to each other, and the reference keeps changing between the 
  // one Bibledit had and the one BibleTime had.
  
  extern Settings * settings;
  // A reference is taken from BibleTime only when it changed reference.
  if (settings->genconfig.reference_exchange_receive_from_bibletime_get ()) {
    Reference btreference (0);
    if (bibletime.getreference (btreference)) {
      ustring new_reference = btreference.human_readable ("");
      if (new_reference != bibletime_previous_reference) {
        bibletime_previous_reference = new_reference;
        // As BibleTime does not support chapter 0 or verse 0, let Bibledit not 
        // receive any reference from it if it is on such a chapter or verse.
        bool receive = true;
        if (navigation.reference.chapter == 0) receive = false;
        if (navigation.reference.verse == "0") receive = false;
        if (receive) navigation.display (btreference);
        got_new_bt_reference = 5;
      }
    }
  }

  // The reference is sent to BibleTime only after it changed.
  // But when we received a new reference, nothing will be sent, 
  // to avoid the race condition that both Bibledit and BibleTime keep 
  // alternating between two references on their own.
  if (got_new_bt_reference > 0) got_new_bt_reference--;
  if (got_new_bt_reference == 0) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      if (settings->genconfig.reference_exchange_send_to_bibletime_get ()) {
        ustring bibledit_bt_new_reference = convert_to_string (editor->current_reference.book) + convert_to_string (editor->current_reference.chapter) + editor->current_reference.verse;
        if (bibledit_bt_new_reference != bibledit_bt_previous_reference) {
          bibledit_bt_previous_reference = bibledit_bt_new_reference;
          bibletime.sendreference (editor->current_reference);
        }    
      }
    }
  }

  return true;
}


void MainWindow::on_reference_exchange1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_reference_exchange ();
}


void MainWindow::on_reference_exchange ()
{
  ReferenceExchangeDialog dialog (0);
  dialog.run();
}


void MainWindow::on_send_word_to_toolbox_signalled (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->send_word_to_toolbox ();
}


void MainWindow::send_word_to_toolbox ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  ustring word = editor->word_double_clicked_text;
  if (word.empty()) return;
  gw_message ("Sending to Toolbox: " + word);
  windowsoutpost->SantaFeFocusWordSet (word);
}


void MainWindow::on_preferences_windows_outpost_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_windows_outpost ();
}


void MainWindow::on_preferences_windows_outpost ()
{
  // Dialog for making settings.
  OutpostDialog dialog (0);
  dialog.run ();
  if (dialog.changed) {
    // Changes were made: destroy and recreate the object.
    delete windowsoutpost;
    windowsoutpost = new WindowsOutpost (true);
    // If required, start the Outpost.
    extern Settings * settings;
    if (settings->genconfig.use_outpost_get ()) {
      // Delay a bit so wine debugger doesn't start.
      g_usleep (1000000);
      windowsoutpost->Start ();
    }
  }
}


/*
|
|
|
|
|
Title bar and status bar
|
|
|
|
|
*/


void MainWindow::set_titlebar (const ustring& project)
{
  ustring title ("Bibledit");
  if (project.length() > 0) title.append (" - " + project);
  gtk_window_set_title (GTK_WINDOW (mainwindow), title.c_str());
}


bool MainWindow::on_gui_timeout (gpointer data)
{
  ((MainWindow *) data)->on_gui ();
  return true;
}


void MainWindow::on_gui ()
{
  // The accelerators for undo and redo (Control(+Shift)-Z) stop working after
  // the widget has been set to insensitive and back to sensitive again.
  // As this may happen often, here we keep setting the accelerators.
  gtk_widget_remove_accelerator (undo1, accel_group, GDK_Z, GDK_CONTROL_MASK);
  gtk_widget_add_accelerator (undo1, "activate", accel_group, GDK_Z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_widget_remove_accelerator (redo1, accel_group, GDK_Z, GdkModifierType (GDK_CONTROL_MASK | GDK_SHIFT_MASK));
  gtk_widget_add_accelerator (redo1, "activate", accel_group, GDK_Z, GdkModifierType (GDK_CONTROL_MASK | GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);
  // Update the amount of Git tasks still to be done.
  ustring git;
  if (git_tasks_count () >= 0) 
    git = "Git tasks pending: " + convert_to_string (git_tasks_count ());
  gtk_label_set_text (GTK_LABEL (label_git), git.c_str());
  // Check whether to reopen the project.
  on_git_reopen_project ();
  // Handle the gui part of displaying project notes.
  // These notes are displayed in a thread, and it is quite a hassle to make Gtk
  // thread-safe, therefore rather than going through this hassle, we just 
  // let the widgets be updated in the main thread.
  if (displayprojectnotes) {
    displayprojectnotes->update_progressbar ();
    if (displayprojectnotes->ready) {
      textbuffer_notes = displayprojectnotes->switch_buffer ();
      displayprojectnotes->position_cursor ();
      delete displayprojectnotes;
      displayprojectnotes = NULL;
    }
  }
  // Care for possible restart.
  extern Settings * settings;
  if (settings->session.restart) {
    gtk_main_quit ();
  }
  // Check window size.
  {
    ScreenLayoutDimensions dimensions (mainwindow, hpaned1, vpaned1, vpaned_references);
    dimensions.clip ();
  }
}


gboolean MainWindow::on_mainwindow_window_state_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  ((MainWindow *) user_data)->on_mainwindow_window_state (event);
  return FALSE;
}


void MainWindow::on_mainwindow_window_state (GdkEvent *event)
{
  extern Settings * settings;
  if (settings->session.window_initialized) {
    bool maximized = ((GdkEventWindowState *)event)->new_window_state == GDK_WINDOW_STATE_MAXIMIZED;
    settings->genconfig.window_maximized_set (maximized);
  } else {
    if (settings->genconfig.window_maximized_get ()) gtk_window_maximize (GTK_WINDOW (mainwindow));
    settings->session.window_initialized = true;    
  }
}


/*
|
|
|
|
|
Notes editor
|
|
|
|
|
 */


gboolean MainWindow::notes_motion_notify_event (GtkWidget *text_view, GdkEventMotion *event, gpointer user_data)
{
  return ((MainWindow *) user_data)->on_notes_pointer_moved (text_view, event);
}


gboolean MainWindow::on_notes_pointer_moved (GtkWidget *text_view, GdkEventMotion *event)
// Update the cursor image if the pointer moved. 
{
  gint x, y;
  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), GTK_TEXT_WINDOW_WIDGET,
                                         gint(event->x), gint(event->y), &x, &y);
  screen_set_cursor_hand_or_regular (GTK_TEXT_VIEW (text_view), x, y);
  gdk_window_get_pointer (text_view->window, NULL, NULL, NULL);
  return false;
}



gboolean MainWindow::notes_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  return ((MainWindow *) user_data)->on_notes_key_press_event (widget, event);
}


gboolean MainWindow::on_notes_key_press_event (GtkWidget *widget, GdkEventKey *event)
/*
Links can be activated by pressing Enter.
Notes can be deleted by pressing Delete.
 */
{
  GtkTextIter iter;
  GtkTextIter start;
  GtkTextIter end;
  GtkTextBuffer *buffer;
  switch (event->keyval) {
    case GDK_Return: 
    case GDK_KP_Enter:
      buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_insert (buffer));
      notes_edit_if_link (widget, &iter);
      break;
    case GDK_Delete: 
    case GDK_KP_Delete:
    {
      GuiFeatures guifeatures (0);
      if (guifeatures.project_notes_management ()) {
        buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
        gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
        notes_delete_if_link (widget, &start, &end);
      }
      break;
    }
    default:
      break;
  }
  return false;
}


void MainWindow::notes_event_after (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_event_after (widget, event);  
}


void MainWindow::on_notes_event_after (GtkWidget *text_view, GdkEvent *ev)
// Notes links can also be activated by clicking.
// References are obtained by control-click.
{
  GtkTextIter start, end, iter;
  GtkTextBuffer *buffer;
  GdkEventButton *event;
  gint x, y;
  if (ev->type != GDK_BUTTON_RELEASE)
    return;
  event = (GdkEventButton *)ev;
  if (event->button != 1)
    return;
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view));
  // We shouldn't follow a link if the user has selected something.
  gtk_text_buffer_get_selection_bounds (buffer, &start, &end);
  if (gtk_text_iter_get_offset (&start) != gtk_text_iter_get_offset (&end))
    return;
  // Get iterator at clicking location.
  gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW (text_view), GTK_TEXT_WINDOW_WIDGET,
                                         gint(event->x), gint(event->y), &x, &y);
  gtk_text_view_get_iter_at_location (GTK_TEXT_VIEW (text_view), &iter, x, y);
  // Depending on whether the control key was down, take appropriate action.
  if (keyboard_control_state (event)) {
    // Ctrl down: get references.
    notes_get_references_from_link (text_view, &iter);
  } else {
    // Control not active: edit note.
    notes_edit_if_link (text_view, &iter);
  } 
}


void MainWindow::notes_edit_if_link (GtkWidget *text_view, GtkTextIter *iter)
/* Looks at all tags covering the position of iter in the text view, 
 * and if one of them is a link, follow it by showing the page identified
 * by the data attached to it.
 */
{
  GSList *tags = NULL, *tagp = NULL;
  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next) {
    GtkTextTag *tag = (GtkTextTag *) tagp->data;
    gint id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tag), "id"));
    if (id != 0) {
      notes_fill_edit_screen (id, false);
      break;
    }
  }
  if (tags) 
    g_slist_free (tags);
}


void MainWindow::on_new_note_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_new_note ();  
}


void MainWindow::on_new_note () 
{
  // If we are currently editing a note, do nothing.
  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) > 0)
    return;

  // Get the unique id for the new note.
  int id = notes_database_get_unique_id ();
  // Create the new note.
  notes_fill_edit_screen (id, true);
}


void MainWindow::notes_delete_if_link (GtkWidget *text_view, GtkTextIter *start, GtkTextIter *end)
/* Looks at all tags covering the positions of the iterator, from start to end,
   in the text view, and if there are links among them, delete it using the 
   data attached to it.
 */
{
  // Storage for the list of ids to delete.
  set <gint> ids;
  // Ensure text iterators go from low to high.
  gtk_text_iter_order (start, end);
  // Go through the range from start to end.
  // Collect the ids of the notes to be deleted.
  GtkTextIter iter;
  iter = * start;
  while (gtk_text_iter_in_range (&iter, start, end) || gtk_text_iter_equal (&iter, start)) {
    GSList *tags = NULL, *tagp = NULL;
    tags = gtk_text_iter_get_tags (&iter);
    for (tagp = tags;  tagp != NULL;  tagp = tagp->next) {
      GtkTextTag *tag = (GtkTextTag *) tagp->data;
      gint id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tag), "id"));
      if (id != 0) {
        ids.insert(id);
      }
    }
    if (tags) 
      g_slist_free (tags);
    gtk_text_iter_forward_char (&iter);
  }
  // Get vector with ids to delete, and delete all notes that are in it.
  vector<gint> ids_to_delete (ids.begin(), ids.end());
  if (ids_to_delete.size() > 0) {
    if (gtkw_dialog_question (mainwindow, "Are you sure you want to delete these project notes?") != GTK_RESPONSE_YES)
      return;
    for (unsigned int i = 0; i < ids_to_delete.size(); i++)
      notes_delete_one(ids_to_delete[i]);
    // Redisplay.
    if (ids_to_delete.size() > 0)
      notes_redisplay();
  }
}


void MainWindow::on_delete_note_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_delete_note();
}


void MainWindow::on_delete_note ()
{
  // There was a bug of an infinite loop when a note was deleted through the menu,
  // while the notes view was not focused. Solved here.
  if (GTK_WIDGET_HAS_FOCUS (textview_notes)) {
    GtkTextIter start;
    GtkTextIter end;
    gtk_text_buffer_get_selection_bounds (textbuffer_notes, &start, &end);
    notes_delete_if_link (textview_notes, &start, &end);
  }
}


void MainWindow::on_viewnotes_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_view_notes ();
}


void MainWindow::on_view_notes ()
{
  ShowNotesDialog dialog (0);
  if (dialog.run() == GTK_RESPONSE_OK)
    notes_redisplay ();
}


void MainWindow::notes_redisplay ()
{
  // Do not display notes while a note is being edited.
  if (gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook1)) != 0) return;
  // Stop any previous notes display.
  stop_displaying_more_notes ();
  // Display the notes with a little delay. This improves navigation speed.
  gw_destroy_source (notes_redisplay_source_id);
  notes_redisplay_source_id = g_timeout_add_full (G_PRIORITY_DEFAULT, 500, GSourceFunc (on_notes_redisplay_timeout), gpointer(this), NULL);
}


bool MainWindow::on_notes_redisplay_timeout (gpointer data)
{
  ((MainWindow *) data)->notes_redisplay_timeout ();
  return false;
}


void MainWindow::notes_redisplay_timeout ()
{
  // Get the focused Editor. If none, bail out.
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  // Get actual verse from the editor.
  ustring reference = books_id_to_english (editor->current_reference.book) + " " + convert_to_string (editor->current_reference.chapter) + ":" + editor->current_reference.verse;
  // Create displaying object.
  displayprojectnotes = new DisplayProjectNotes (reference, textbuffer_notes, textview_notes, NULL);
}


void MainWindow::on_find_in_notes1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->find_in_notes ();
}


void MainWindow::find_in_notes ()
{
  {
    FindNoteDialog findnotedialog (0);
    if (findnotedialog.run () == GTK_RESPONSE_OK) {
      stop_displaying_more_notes ();
      displayprojectnotes = new DisplayProjectNotes ("", textbuffer_notes, textview_notes, &findnotedialog.ids);
    }
  }
}


void MainWindow::on_import_notes_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_import_notes ();
}


void MainWindow::on_import_notes ()
{
  ImportNotesDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_APPLY) {
    notes_redisplay ();
  }
}


void MainWindow::on_export_notes_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_export_notes ();
}


void MainWindow::on_export_notes ()
{
  int result;
  ustring filename;
  ExportNotesFormat format;
  bool save_all_notes;
  {
    ExportNotesDialog dialog (0);
    result = dialog.run();
    filename = dialog.filename;
    format = dialog.exportnotesformat;
    save_all_notes = dialog.save_all_notes;
  }
  if (result == GTK_RESPONSE_OK) {
  vector <unsigned int> ids_to_display;
  export_translation_notes (filename, format, ids_to_display, save_all_notes, mainwindow);    
  }
}


void MainWindow::on_button_cancel_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_button_cancel ();
}


void MainWindow::on_notes_button_cancel ()
{
  // Do standard functions for both ok and cancel.
  on_notes_button_ok_cancel ();
}


void MainWindow::on_button_ok_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_button_ok ();
}


void MainWindow::on_notes_button_ok ()
{
  sqlite3 *db;
  int rc;
  char *error = NULL;
  try
  {
    /*
    Validate and normalize the references.
    Bad ones are removed and a message will be given.
    If no valid references remain, stop the whole transaction and give a message.
    */
    ustring encoded_references;
    ustring osis_references;
    // Get and validate all references from the textview.
    {
      // Store references.
      vector<Reference> references;
      // Store possible messages here for later display.
      vector<ustring> messages;
      // Get all references from the editor.
      notes_get_references_from_editor (note_editor->textbuffer_references, references, messages);
      // Encode all references.
      for (unsigned int i = 0; i < references.size(); i++) {
        // Encode the reference.
        vector<int> verses = verses_encode (references[i].verse);
        Reference ref = references[i];
        ref.verse = "0";
        int book_chapter = reference_to_numerical_equivalent (ref);
        for (unsigned int i2 = 0; i2 < verses.size(); i2++) {
          encoded_references.append(" ");
          encoded_references.append (convert_to_string (int (book_chapter + verses[i2])));
        }
        // Store the references in OSIS format too.
        ustring osis_book = books_id_to_osis (references[i].book);
        ustring osis_reference = osis_book + "." + convert_to_string (references[i].chapter) + "." + references[i].verse;
        if (!osis_references.empty())
          osis_references.append (" ");
        osis_references.append (osis_reference);
      }
      encoded_references.append (" ");
      // See whether there are messages to display.
      if (messages.size() > 0) {
        ustring message;
        for (unsigned int i = 0; i < messages.size(); i++) {
          message.append (messages[i]);
          message.append ("\n");
        }
        gtkw_dialog_error (mainwindow, message);
      }
    }
    // See whether any references are left. If not give a message.
    if (encoded_references.empty()) {
      gtkw_dialog_error (mainwindow, "No valid references. Note was not stored");
      return;
    }
    // Connect to database and start transaction.
    rc = sqlite3_open(notes_database_filename ().c_str (), &db);
    if (rc) throw runtime_error (sqlite3_errmsg(db));
    sqlite3_busy_timeout (db, 1000);
    rc = sqlite3_exec (db, "begin;", NULL, NULL, &error);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
    // Delete previous data with "id".
    gchar * sql;
    sql = g_strdup_printf ("delete from %s where id = %d;", TABLE_NOTES, note_editor->id);
    rc = sqlite3_exec (db, sql, NULL, NULL, &error);
    g_free (sql);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
    // Put new data in the database.
    // ID (integer), we take variable "myid"
    // References (text), we take variable "encoded_references"
    // Project (text)
    ustring project = combobox_get_active_string (combobox_note_project);
    // Status (integer) This field is not used, and could be reused.
    // Category (text)
    ustring category = combobox_get_active_string (combobox_note_category);
    // Note (text)
    ustring note;
    {
      vector<ustring> lines;
      textbuffer_get_lines (note_editor->textbuffer_note, lines, false);
      for (unsigned int i = 0; i < lines.size(); i++) {
        if (!note.empty())
          note.append("\n");
        note.append(lines[i]);
      }
    }
    // Trim off extra newlines at the end, and ensure it always has one.
    note = trim(note);
    note.append ("\n");
    // Apostrophies need to be doubled before storing them.
    note = double_apostrophy (note);
    // Casefolded (text)
    ustring casefolded = note.casefold();
    // Date created. Variabele note_info_date_created
    // Date modified.
    int date_modified;
    date_modified = date_time_julian_day_get_current ();
    // Username. Use: note_info_user_created
    // Logbook (text)
    ustring logbook;
    {
      // Get and clean the lines.
      vector<ustring> lines;
      textbuffer_get_lines (note_editor->textbuffer_logbook, lines);
      for (unsigned int i = 0; i < lines.size(); i++) {
        if (!logbook.empty())
          logbook.append("\n");
        logbook.append(lines[i]);
      }
      // Trim off extra newlines at the end.
      logbook = trim(logbook);
      // Now add new data to the logbook.
      ustring date_user_text = date_time_julian_human_readable (date_modified, true);
      date_user_text.append (", ");
      date_user_text.append (g_get_real_name());
      date_user_text.append (" ");
      if (note_editor->newnote) {
        if (!logbook.empty())
          logbook.append ("\n");
        logbook.append (date_user_text);
        logbook.append ("created a new note, category \"");
        logbook.append (category);
        logbook.append ("\", project \"");
        logbook.append (project);   
        logbook.append ("\".");
      } else {
        vector<ustring> actions;
        if (gtk_text_buffer_get_modified (note_editor->textbuffer_references)) {
          actions.push_back ("modified the references");
        }
        if (gtk_text_buffer_get_modified (note_editor->textbuffer_note)) {
          actions.push_back ("modified the note");
        }
        if (category != note_editor->previous_category) {
          actions.push_back ("changed the category to \"" + category + "\"");
        }
        if (project != note_editor->previous_project) {
          actions.push_back ("changed the project to \"" + project + "\"");
        }
        if (actions.size() > 0) {
          if (!logbook.empty())
            logbook.append ("\n");
          logbook.append (date_user_text);
          for (unsigned int i = 0; i < actions.size(); i++) {
            if (i > 0)
              logbook.append (", ");
            logbook.append (actions[i]);
          }
          logbook.append (".");
        }
      }
      // Apostrophies need to be doubled before storing them.
      logbook = double_apostrophy (logbook);
    }
    // Insert data in database.
    sql = g_strdup_printf ("insert into %s values (%d, '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, '%s', '%s');", TABLE_NOTES, note_editor->id, encoded_references.c_str(), osis_references.c_str(), project.c_str(), category.c_str(), note.c_str(), casefolded.c_str(), note_editor->date_created, date_modified, note_editor->created_by.c_str(), logbook.c_str());
    rc = sqlite3_exec (db, sql, NULL, NULL, &error);
    g_free (sql);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
    // Commit the transaction.
    rc = sqlite3_exec (db, "commit;", NULL, NULL, &error);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
  }
  catch (exception & ex)
  {
    gw_critical (ex.what ());
  }
  // Close connection.  
  sqlite3_close (db);
  // Do standard functions for both ok and cancel.
  on_notes_button_ok_cancel ();
  // New note, or note edited, so display notes again.
  notes_redisplay ();
}


void MainWindow::on_notes_button_ok_cancel ()  
// Functions common to both the ok and cancel buttons.
{
  // Show the normal notes display again.
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook1), 0);
  // Focus widget that was focused previously.
  // This must be done after switching the page of the notebook,
  // to ensure the widget (usually the text editor) does indeed get focus.
  gtk_window_set_focus (GTK_WINDOW (mainwindow), note_editor->previous_focus);
  gtk_widget_grab_focus (note_editor->previous_focus);
  // Same for Tools Area tab that displayed previously (unless user changed it).
  if (note_editor->previous_tools_page >= 0)
    gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), note_editor->previous_tools_page);
  // Clear some widgets.
  combobox_clear_strings (combobox_note_category);
  gtk_text_buffer_set_text (note_editor->textbuffer_references, "", -1);
  combobox_clear_strings (combobox_note_project);  
  gtk_label_set_text (GTK_LABEL (label_note_created_on), "");
  gtk_label_set_text (GTK_LABEL (label_note_created_by), "");
  gtk_label_set_text (GTK_LABEL (label_note_edited_on), "");
  gtk_text_buffer_set_text (note_editor->textbuffer_logbook, "", -1);
  // Hide the notebook page.
  gtk_widget_hide (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), tapntProjectNote));
  
  // Destroy NoteEditor object.
  if (note_editor)
    delete note_editor;
  note_editor = NULL;
}


void MainWindow::notes_fill_edit_screen (int id, bool newnote)
/*
When a new note is made, or an existing one edited, this function
sets up the edit screen.
*/
{
  // Create the NoteEditor object.
  if (note_editor)
    delete note_editor;
  note_editor = new NoteEditor (0);
  // Save variables.
  note_editor->id = id;
  note_editor->newnote = newnote;
  
  // Show the notebook page.
  gtk_widget_show (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_tools), tapntProjectNote)); 
  
  // Initialize pointers to the text buffers.
  note_editor->textbuffer_note = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note));
  note_editor->textbuffer_references = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note_references));
  note_editor->textbuffer_logbook = gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note_logbook));

  // Save the widget that had the focus, so that when the notes had been edited,
  // the same widget gets the focus again.
  note_editor->previous_focus = gtk_window_get_focus (GTK_WINDOW (mainwindow));
  // Same for the tab that now displays in the Tools Area.
  int tools_page_displayed_previously = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook_tools));

  // Get the Editor that has the focus.
  Editor * editor = editorsgui->focused_editor ();
  
  // Get our language.
  extern Settings * settings;
  ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
  ustring language = projectconfig->language_get();

  // Fetch the data for the note from the database. And fill comboboxes.
  // Or in case of a new note, deal appropriately with that.
  sqlite3 *db;
  int rc;
  char *error = NULL;
  try
  {
    rc = sqlite3_open(notes_database_filename ().c_str (), &db);
    if (rc) throw runtime_error (sqlite3_errmsg(db));
    sqlite3_busy_timeout (db, 1000);
    SqliteReader sqlitereader (0);
    char * sql;
    sql = g_strdup_printf ("select ref_osis, project, category, note, created, modified, user, logbook from %s where id = %d;", TABLE_NOTES, id);
    rc = sqlite3_exec(db, sql, sqlitereader.callback, &sqlitereader, &error);
    g_free (sql);
    if (rc != SQLITE_OK) {
      throw runtime_error (error);
    }
    if ((sqlitereader.ustring0.size() > 0) || newnote) {
      ustring reference;
      gtk_text_buffer_set_text (note_editor->textbuffer_references, "", -1);
      if (newnote) {
        // New note, so get the current reference from the editor.
        if (editor) {
          reference = books_id_to_english (editor->current_reference.book) + " " + convert_to_string (editor->current_reference.chapter) + ":" + editor->current_reference.verse;
        }
        gtk_text_buffer_insert_at_cursor (note_editor->textbuffer_references, reference.c_str(), -1);
        gtk_text_buffer_insert_at_cursor (note_editor->textbuffer_references, "\n", -1);
      } else {
        // Existing note, so get the reference(s) from the database.
        reference = sqlitereader.ustring0[0];
        // Read the reference(s) and show them.
        Parse parse (reference, false);
        for (unsigned int i = 0; i < parse.words.size(); i++) {
          Reference reference (0);
          reference_discover (0, 0, "", parse.words[i], reference.book, reference.chapter, reference.verse);
          ustring ref = reference.human_readable (language);
          gtk_text_buffer_insert_at_cursor (note_editor->textbuffer_references, ref.c_str(), -1);
          gtk_text_buffer_insert_at_cursor (note_editor->textbuffer_references, "\n", -1);
        }
      }
      // Read the note.
      ustring note;
      if (!newnote) {
        note = sqlitereader.ustring3[0];
      }
      gtk_text_buffer_set_text (note_editor->textbuffer_note, note.c_str(), -1);
      gtk_text_buffer_set_modified (note_editor->textbuffer_note, false);
      gtk_text_buffer_set_modified (note_editor->textbuffer_references, false);
      if (newnote) {
        // Select the informative text so that is disappears as soon as the user
        // starts to type.
        GtkTextIter start;
        GtkTextIter end;
        gtk_text_buffer_get_bounds (note_editor->textbuffer_note, &start, &end);
        gtk_text_buffer_move_mark_by_name (note_editor->textbuffer_note, "insert", &start);
        gtk_text_buffer_move_mark_by_name (note_editor->textbuffer_note, "selection_bound", &end);
      }
      /*
      Fill the category combo.
      */
      {
        ReadText rt (notes_categories_filename());
        combobox_set_strings (combobox_note_category, rt.lines);
        if (rt.lines.size() > 0)
          combobox_set_string (combobox_note_category, rt.lines[0]);
      }
      // Read the "category" variable.
      {
        if (!newnote) {
          ustring category = sqlitereader.ustring2[0];
          combobox_set_string (combobox_note_category, category);
        }
      }
      // Read the project. Fill the combo.
      {
        ustring project;
        if (newnote) {
          project = settings->genconfig.project_get();
        } else {
          project = sqlitereader.ustring1[0];
        }
        vector<ustring> projects;
        if (project != "All")
          projects.push_back(project);
        if (settings->genconfig.project_get() != project)
          projects.push_back(settings->genconfig.project_get());
        projects.push_back("All");
        combobox_set_strings (combobox_note_project, projects);
        combobox_set_string (combobox_note_project, project);
      }
      // Read the date created.
      {
        // Fetch and store variable for display and later use.
        if (newnote) {
          note_editor->date_created = date_time_julian_day_get_current ();
        } else {
          note_editor->date_created = convert_to_int (sqlitereader.ustring4[0]);
        }
        ustring s;
        s = "Created on " + date_time_julian_human_readable (note_editor->date_created, true);
        gtk_label_set_text (GTK_LABEL (label_note_created_on), s.c_str());        
      }
      // Read the date modified.
      {
        if (newnote) {
          note_editor->date_modified = date_time_julian_day_get_current ();
        } else {
          note_editor->date_modified = convert_to_int (sqlitereader.ustring5[0]);
        }
        ustring s;
        s = "Edited on " + date_time_julian_human_readable (note_editor->date_modified, true);
        gtk_label_set_text (GTK_LABEL (label_note_edited_on), s.c_str());        
      }
      // Read the user that created the note.
      {
        if (newnote) {
          note_editor->created_by = g_get_real_name();
        } else {
          note_editor->created_by = sqlitereader.ustring6[0];
        }
        ustring s;
        s = "Created by " + note_editor->created_by;
        gtk_label_set_text (GTK_LABEL (label_note_created_by), s.c_str());        
      }
      // Read the logbook.
      {
        ustring logbook;
        if (!newnote)
          logbook = sqlitereader.ustring7[0];
        gtk_text_buffer_set_text (note_editor->textbuffer_logbook, logbook.c_str(), -1);
      }
    }
  }
  catch (exception & ex)
  {
    gw_critical (ex.what ());
  }
  // Close connection.  
  sqlite3_close (db);

  // Store category and project.
  note_editor->previous_category = combobox_get_active_string (combobox_note_category);
  note_editor->previous_project = combobox_get_active_string (combobox_note_project);
 
  // Switch screen to displaying the tabs for editing.
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook1), 1);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntProjectNote);
  // Store current page, so we can switch back to it later.
  note_editor->previous_tools_page = tools_page_displayed_previously;
  // Focus the widget the user is most likely going to type in.
  gtk_widget_grab_focus (textview_note);
}


void MainWindow::note_insert_date_and_text (const ustring& text)
{
  // If buffer does not end with a new line, insert one.
  GtkTextIter enditer;
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note)), &enditer);
  if (!gtk_text_iter_starts_line (&enditer)) {
    gtk_text_buffer_insert (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note)), &enditer, "\n", -1);
  }
  // Insert message at the end of the note.
  ustring message;
  message.append (g_get_real_name());
  message.append (": ");
  message.append (text);
  message.append (".");
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note)), &enditer);
  gtk_text_buffer_insert (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_note)), &enditer, message.c_str(), -1);
}


void MainWindow::on_standard_text_1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_standard_text (menuitem);
}


void MainWindow::on_standard_text_2_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_standard_text (menuitem);
}


void MainWindow::on_standard_text_3_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_standard_text (menuitem);
}


void MainWindow::on_standard_text_4_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_standard_text (menuitem);
}


void MainWindow::on_current_reference1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_standard_text (menuitem);
}


void MainWindow::on_insert_standard_text (GtkMenuItem *menuitem)
{
  // Find out which standard text to insert, and where to insert it, and how.
  extern Settings * settings;
  ustring standardtext;
  bool addspace = false;
  GtkWidget * textview = textview_note;
  if (menuitem == GTK_MENU_ITEM (standard_text_1)) {
    standardtext = settings->genconfig.edit_note_standard_text_one_get ();
    addspace = true;
    textview = textview_note;
  } else if (menuitem == GTK_MENU_ITEM (standard_text_2)) {
    standardtext = settings->genconfig.edit_note_standard_text_two_get ();
    addspace = true;
    textview = textview_note;
  } else if (menuitem == GTK_MENU_ITEM (standard_text_3)) {
    standardtext = settings->genconfig.edit_note_standard_text_three_get ();
    addspace = true;
    textview = textview_note;
  } else if (menuitem == GTK_MENU_ITEM (standard_text_4)) {
    standardtext = settings->genconfig.edit_note_standard_text_four_get ();
    addspace = true;
    textview = textview_note;
  } else if (menuitem == GTK_MENU_ITEM (current_reference1)) {
    Editor * editor = editorsgui->focused_editor ();
    if (editor)    
      standardtext = books_id_to_english (editor->current_reference.book) + " " + convert_to_string (editor->current_reference.chapter) + ":" + editor->current_reference.verse;
    addspace = false;
    textview = textview_note_references;
  }
  // Add space.
  if (addspace)
    standardtext.append (" ");
  // If text was selected, erase that text.
  {
    GtkTextIter iter1, iter2;
    if (gtk_text_buffer_get_selection_bounds (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &iter1, &iter2)) {
      gtk_text_buffer_delete (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &iter1, &iter2);
    }
  }
  // If buffer does not end with a new line, insert one.
  GtkTextIter enditer;
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &enditer);
  if (!gtk_text_iter_starts_line (&enditer)) {
    gtk_text_buffer_insert (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &enditer, "\n", -1);
  }
  // Message.
  ustring message;
  // Add the standard text.
  message.append (standardtext);
  // Insert message at the end of the note.
  gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &enditer);
  gtk_text_buffer_insert (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)), &enditer, message.c_str(), -1);
}


void MainWindow::stop_displaying_more_notes ()
// Stop the process of displaying notes.
{
  if (displayprojectnotes) {
    displayprojectnotes->stop ();
  }
  while (displayprojectnotes) {
    // Events must go on to keep the program responsive.
    while (gtk_events_pending ()) gtk_main_iteration ();
    g_usleep (100000);
    // While waiting run the on_gui routine, because this deletes the object if it is ready.
    on_gui ();
  }
}


void MainWindow::on_get_references_from_note_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_get_references_from_note ();
}


void MainWindow::on_get_references_from_note ()
{
  // Store references.
  vector<Reference> references;
  // Store possible messages here, but they will be dumped.
  vector<ustring> messages;
  // Get all references from the editor.
  if (note_editor)
    notes_get_references_from_editor (note_editor->textbuffer_references, references, messages);
  // Sort the references so they appear nicely in the editor.
  sort_references (references);
  // Set none searchwords.
  extern Settings* settings;
  settings->session.highlights.clear();
  // Set references.
  References references2 (liststore_references, treeview_references, treecolumn_references);
  references2.set_references (references);
  ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
  references2.fill_store (projectconfig->language_get());
  // Display the References Area
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntReferences);
}


void MainWindow::notes_get_references_from_link (GtkWidget *text_view, GtkTextIter *iter)
// If there is a link at iter, get the references from it.
{
  // Store references we get.
  vector<Reference> references;
  // Gets all tags at the iterator.
  GSList *tags = NULL, *tagp = NULL;
  tags = gtk_text_iter_get_tags (iter);
  for (tagp = tags;  tagp != NULL;  tagp = tagp->next) {
    GtkTextTag *tag = (GtkTextTag *) tagp->data;
    gint id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (tag), "id"));
    if (id != 0) {
      // Fetch the references for the note from the database.
      sqlite3 *db;
      int rc;
      char *error = NULL;
      try
      {
        rc = sqlite3_open(notes_database_filename ().c_str (), &db);
        if (rc) throw runtime_error (sqlite3_errmsg(db));
        sqlite3_busy_timeout (db, 1000);
        SqliteReader sqlitereader (0);
        char * sql;
        sql = g_strdup_printf ("select ref_osis from %s where id = %d;", TABLE_NOTES, id);
        rc = sqlite3_exec(db, sql, sqlitereader.callback, &sqlitereader, &error);
        g_free (sql);
        if (rc != SQLITE_OK)
          throw runtime_error (error);
        if ((sqlitereader.ustring0.size() > 0)) {
          ustring reference;
          reference = sqlitereader.ustring0[0];
          // Read the reference(s).
          Parse parse (reference, false);
          for (unsigned int i = 0; i < parse.words.size(); i++) {
            Reference ref (0);
            reference_discover (0, 0, "", parse.words[i], ref.book, ref.chapter, ref.verse);
            references.push_back (ref);
          }
        }
      }
      catch (exception & ex)
      {
        gw_critical (ex.what ());
      }
      // Close connection.  
      sqlite3_close (db);
      break;
    }
  }
  if (tags) 
    g_slist_free (tags);
  // Set references.
  References references2 (liststore_references, treeview_references, treecolumn_references);
  references2.set_references (references);
  extern Settings * settings;
  ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
  references2.fill_store (projectconfig->language_get());
  // Display the References Area
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntReferences);
}


/*
|
|
|
|
|
Export
|
|
|
|
|
*/


void MainWindow::on_export_usfm_files_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_export_usfm_files (false);
}


void MainWindow::on_export_zipped_unified_standard_format_markers1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_export_usfm_files (true);
}


void MainWindow::on_export_usfm_files (bool zipped)
{
  editorsgui->save ();
  export_to_usfm (mainwindow, zipped);
}


void MainWindow::on_to_bibleworks_version_compiler_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_to_bibleworks_version_compiler ();
}


void MainWindow::on_to_bibleworks_version_compiler ()
{
  editorsgui->save ();
  export_to_bibleworks (mainwindow);
}


void MainWindow::on_export_to_sword_module_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_export_to_sword_module ();
}


void MainWindow::on_export_to_sword_module ()
{
  editorsgui->save ();
  export_to_sword_interactive ();
  bibletime.reloadmodules ();
}


void MainWindow::on_export_opendocument_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_export_opendocument ();
}


void MainWindow::on_export_opendocument ()
{
  editorsgui->save ();
  export_to_opendocument (mainwindow);
}


/*
|
|
|
|
|
Checks
|
|
|
|
|
*/


void MainWindow::on_check1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check ();
}


void MainWindow::on_menu_check ()
{
  // Switch to the References Area.
  on_file_references ();
}


void MainWindow::on_markers1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check_markers ();
}


void MainWindow::on_menu_check_markers ()
{
}


void MainWindow::on_validate_usfms1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check_markers_validate ();
}


void MainWindow::on_menu_check_markers_validate ()
{
  editorsgui->save ();
  scripture_checks_validate_usfms (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_count_usfms1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check_markers_count ();
}


void MainWindow::on_menu_check_markers_count ()
{
  editorsgui->save ();
  scripture_checks_count_usfms (true);
}


void MainWindow::on_compare_usfm1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check_markers_compare ();
}


void MainWindow::on_menu_check_markers_compare ()
{
  editorsgui->save ();
  scripture_checks_compare_usfms (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_chapters_and_verses1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_menu_check_chapters_and_verses();
}


void MainWindow::on_menu_check_chapters_and_verses ()
{
  editorsgui->save ();
  scripture_checks_chapters_verses (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_count_characters_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_count_characters();
}


void MainWindow::on_count_characters ()
{
  editorsgui->save ();
  scripture_checks_count_characters (true);
}


void MainWindow::on_unwanted_patterns_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_unwanted_patterns ();
}


void MainWindow::on_unwanted_patterns ()
{
  editorsgui->save ();
  scripture_checks_unwanted_patterns (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_check_capitalization_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_capitalization();
}


void MainWindow::on_check_capitalization ()
{
  editorsgui->save ();
  scripture_checks_capitalization (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_check_repetition_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_repetition();
}


void MainWindow::on_check_repetition ()
{
  editorsgui->save ();
  scripture_checks_repetition (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_check_matching_pairs_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_matching_pairs();
}


void MainWindow::on_check_matching_pairs ()
{
  editorsgui->save ();
  scripture_checks_matching_pairs (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_unwanted_words_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_unwanted_words();
}


void MainWindow::on_unwanted_words ()
{
  editorsgui->save ();
  scripture_checks_unwanted_words (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_word_count_inventory_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_word_count_inventory();
}


void MainWindow::on_word_count_inventory ()
{
  editorsgui->save ();
  scripture_checks_word_inventory (true);
}


bool MainWindow::on_check_httpd_timeout (gpointer data)
{
  ((MainWindow *) data)->on_check_httpd ();
  return true;
}


void MainWindow::on_check_httpd ()
{
  // Does the httpd have a request for us?
  if (!httpd.search_whole_word.empty()) {
    // Bibledit presents itself and searches for the word.
    gtk_window_present (GTK_WINDOW (mainwindow));
    extern Settings * settings;
    settings->session.searchword = httpd.search_whole_word;
    httpd.search_whole_word.clear();
    settings->session.search_case_sensitive = true;
    settings->session.search_current_book = false;
    settings->session.search_current_chapter = false;
    settings->session.search_globbing = false;
    settings->session.search_start_word_match = true;
    settings->session.search_end_word_match = true;
    settings->session.search_page = 1;
    settings->session.searchresultstype = sstLoad;
    settings->session.highlights.clear();
    SessionHighlights sessionhighlights (settings->session.searchword, settings->session.search_case_sensitive, settings->session.search_globbing, settings->session.search_start_word_match, settings->session.search_end_word_match, atRaw, false, false, false, false, false, false, false, false);
    settings->session.highlights.push_back (sessionhighlights);
    search_string (liststore_references, treeview_references, treecolumn_references, &bibletime);    
  }
}


void MainWindow::on_my_checks_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_my_checks ();
}


void MainWindow::on_my_checks ()
{
  editorsgui->save ();
  MyChecksDialog dialog (liststore_references, treeview_references, treecolumn_references);
  dialog.run ();
}


void MainWindow::on_check_terms_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_terms ();
}


void MainWindow::on_check_terms ()
{
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntKeyterms);
  keytermsgui->focus_entry();
}


void MainWindow::on_check_markers_spacing_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_markers_spacing ();
}


void MainWindow::on_check_markers_spacing ()
{
  editorsgui->save ();
  scripture_checks_usfm_spacing (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_check_references_inventory_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_references_inventory ();
}


void MainWindow::on_check_references_inventory ()
{
  editorsgui->save ();
  scripture_checks_references_inventory (true);
}


void MainWindow::on_check_references_validate_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_references_validate ();
}


void MainWindow::on_check_references_validate ()
{
  editorsgui->save ();
  scripture_checks_validate_references (liststore_references, treeview_references, treecolumn_references, NULL);
}


void MainWindow::on_check_nt_quotations_from_the_ot_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_check_nt_quotations_from_the_ot ();
}


void MainWindow::on_check_nt_quotations_from_the_ot ()
{
  editorsgui->save ();
  scripture_checks_nt_quotations_from_ot (true);
}


void MainWindow::on_synoptic_parallel_passages_from_the_nt_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_synoptic_parallel_passages_from_the_nt ();
}


void MainWindow::on_synoptic_parallel_passages_from_the_nt ()
{
  editorsgui->save ();
  scripture_checks_synoptic_parallels_from_nt (true);
}


void MainWindow::on_parallels_from_the_ot_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_parallels_from_the_ot ();
}


void MainWindow::on_parallels_from_the_ot ()
{
  editorsgui->save ();
  scripture_checks_parallels_from_ot (true);
}


/*
|
|
|
|
|
Styles
|
|
|
|
|
*/


void MainWindow::on_goto_styles_area_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_goto_styles_area ();
}


void MainWindow::on_goto_styles_area ()
{
  // Set the right page and focus the right treeview to enable the user to start
  // inserting the style using the keyboard.
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntStyles);
  gtk_widget_grab_focus (styles->treeview);
}


void MainWindow::on_file_styles_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_styles ();
}


void MainWindow::on_file_styles ()
{
  // Show the Styles Area.
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntStyles);
}


void MainWindow::stylesheet_open_named (const ustring& stylesheet)
{
  // Load stylesheet.
  styles->load (stylesheet);
  // Set gui.
  ustring s = "Stylesheet " + stylesheet;  
  gtk_label_set_text (GTK_LABEL (statuslabel_stylesheet), s.c_str());
}


void MainWindow::on_style_button_open_clicked (GtkButton *button, gpointer user_data)
// This is activated by the GuiStyles object if another stylesheet should be opened.
{
  ((MainWindow *) user_data)->on_style_button_open ();
}


void MainWindow::on_style_button_open ()
{
  // Save the name of the stylesheet in two locations.
  extern Settings * settings;
  settings->genconfig.stylesheet_set (styles->get_sheet ());
  if (!settings->genconfig.project_get().empty()) {
    ProjectConfiguration * projectconfig = settings->projectconfig (settings->genconfig.project_get());
    projectconfig->stylesheet_set (styles->get_sheet ());
  }
  // Actually open it.  
  stylesheet_open_named (styles->get_sheet ());
}


void MainWindow::on_style_button_apply_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_style_apply ();
}


void MainWindow::on_style_apply ()
{
  // Get the focused Editor. If none, bail out.
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;

  // Get the focused style(s).
  ustring selected_style = styles->get_focus ();
  
  // Only proceed when a style has been selected.
  if (selected_style.empty())
    return;

  // Get the Style object.
  extern Settings * settings;
  Style style (settings->genconfig.stylesheet_get (), selected_style, false);

  // Whether and how the style is used.
  bool style_was_used = true;
  bool style_was_treated_specially = false;
  
  // Special treatment for the chapter style.
  if (style.type == stChapterNumber) {
    // Ask whether the user wishes to insert a new chapter.
    if (gtkw_dialog_question (mainwindow, "Would you like to insert a new chapter?", GTK_RESPONSE_YES) == GTK_RESPONSE_YES) {
      // Insert a new chapter.
      editorsgui->save ();
      ChapterNumberDialog dialog (true);
      if (dialog.run () == GTK_RESPONSE_OK) {
        reload_project ();
      } else {
        style_was_used = false;
      }
      style_was_treated_specially = true;
    }
  }    

  // Inserting footnote or endnote or crossreference.
  {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      if (editor->last_focused_type () == etvtBody) {
        if (style.type == stFootEndNote) {
          if (style.subtype == fentFootnote) {
            InsertNoteDialog dialog (indtFootnote);
            if (dialog.run() == GTK_RESPONSE_OK) {
              editor->insert_note (style.marker, dialog.rawtext, NULL, true);
            } else {
              style_was_used = false;          
            }
            style_was_treated_specially = true;
          }
          if (style.subtype == fentEndnote) {
            InsertNoteDialog dialog (indtEndnote);
            if (dialog.run() == GTK_RESPONSE_OK) {
              editor->insert_note (style.marker, dialog.rawtext, NULL, true);
            } else {
              style_was_used = false;          
            }
            style_was_treated_specially = true;
          }
        }
        if (style.type == stCrossreference) {
          InsertNoteDialog dialog (indtCrossreference);
          if (dialog.run() == GTK_RESPONSE_OK) {
            editor->insert_note (style.marker, dialog.rawtext, NULL, true);
          } else {
            style_was_used = false;          
          }
          style_was_treated_specially = true;
          // If the gui has been set so, display the references in the tools area.
          if (settings->genconfig.inserting_xref_shows_references_get ()) {
            gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntReferences);
            gtk_widget_grab_focus (editor->last_focused_textview ());
          }
        }
      }
    }
  }
  
  // Special treatment for a table style.
  {
    Editor * editor = editorsgui->focused_editor ();
    if (editor) {
      if (editor->last_focused_type () == etvtBody) {
        if (style.type == stTableElement) {
          InsertTableDialog dialog (editor->project);
          if (dialog.run() == GTK_RESPONSE_OK) {
            editor->insert_table (dialog.rawtext, NULL);
          } else {
            style_was_used = false;          
          }
          style_was_treated_specially = true;
        }
      }
    }
  }
  
  // Normal treatment of the style if it was not handled specially.
  if (!style_was_treated_specially) {
    // Normal treatment of the marker: apply it.
    editor->apply_style (selected_style);
  }

  // Take some actions if the style was used.
  if (style_was_used) {
    styles->use (selected_style);
  }
}


void MainWindow::on_editor_style_changed (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->editor_style_changed ();
}


void MainWindow::editor_style_changed ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  set <ustring> styles = editor->get_styles_at_cursor ();
  vector <ustring> styles2 (styles.begin (), styles.end ());
  ustring text = "Style ";
  for (unsigned int i = 0; i < styles2.size (); i++) {
    if (i) text.append (", ");
    text.append (styles2[i]);
  }
  gtk_label_set_text (GTK_LABEL (statuslabel_style), text.c_str ());
}


void MainWindow::on_style_edited (GtkButton *button, gpointer user_data)
// This function is called when the properties of a style have been edited.
{
  ((MainWindow *) user_data)->editorsgui->reload_styles ();
}


/*
|
|
|
|
|
Footnotes, endnotes, crossreferences
|
|
|
|
|
*/


void MainWindow::on_edit_bible_note_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_edit_bible_note ();
}


void MainWindow::on_edit_bible_note ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (editor) {
    EditNoteDialog dialog (editor);
    dialog.run ();
  }
}


/*
|
|
|
|
|
Formatted view
|
|
|
|
|
*/


/*
|
|
|
|
|
Tools
|
|
|
|
|
*/


void MainWindow::on_menutools_activate (GtkMenuItem *menuitem, gpointer user_data)
{
}


void MainWindow::on_line_cutter_for_hebrew_text1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_line_cutter_for_hebrew_text ();
}


void MainWindow::on_line_cutter_for_hebrew_text ()
{
  LineCutterDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK) {
    reload_project ();
  }    
}


void MainWindow::on_notes_transfer_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_transfer ();
}


void MainWindow::on_notes_transfer ()
{
  editorsgui->save ();
  NotesTransferDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK)
    notes_redisplay ();
}


void MainWindow::on_tool_origin_references_in_bible_notes_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_tool_origin_references_in_bible_notes ();
}


void MainWindow::on_tool_origin_references_in_bible_notes ()
{
  editorsgui->save ();
  OriginReferencesDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK)
    reload_project ();
}


void MainWindow::on_tool_project_notes_mass_update1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_tool_project_notes_mass_update ();
}


void MainWindow::on_tool_project_notes_mass_update ()
{
  NotesUpdateDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK) {
    notes_redisplay ();
  }
}


void MainWindow::on_tool_generate_word_lists_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_tool_generate_word_lists ();
}


void MainWindow::on_tool_generate_word_lists ()
{
  editorsgui->save ();
  WordlistDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK)
    reload_project ();
}


void MainWindow::on_preferences_features_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_features ();
}


void MainWindow::on_preferences_features ()
{
  if (password_pass (mainwindow)) {
    FeaturesDialog dialog (0);
    dialog.run ();
  }
}


void MainWindow::on_preferences_password_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_password ();
}


void MainWindow::on_preferences_password ()
{
  password_edit (mainwindow);
}


void MainWindow::on_tool_simple_text_corrections_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_tool_simple_text_corrections ();
}


void MainWindow::on_tool_simple_text_corrections ()
{
  editorsgui->save ();
  FixMarkersDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK)
    reload_project ();
}


void MainWindow::on_preferences_text_replacement_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_text_replacement ();
}


void MainWindow::on_preferences_text_replacement ()
{
  TextReplacementDialog dialog (0);
  dialog.run ();
}


void MainWindow::on_parallel_passages1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_parallel_passages1 ();
}


void MainWindow::on_parallel_passages1 ()
{
  // Act depending on whether to view the parallel passages.
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (parallel_passages1))) {
    // View them: show references tab, view passages.
    on_file_references ();
    parallel_passages_display (navigation.reference, liststore_references, treeview_references, treecolumn_references);
  } else {
    // Don't view them: clear store.
    on_clear_references ();
  }
}


void MainWindow::on_pdf_viewer1_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_pdf_viewer ();
}


void MainWindow::on_pdf_viewer ()
{
  PDFViewerDialog dialog (0);
  dialog.run ();
}


void MainWindow::on_view_usfm_code_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_view_usfm_code ();
}


void MainWindow::on_view_usfm_code ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;  
  editorsgui->save ();
  ustring filename = project_data_filename_chapter (editor->project, editor->book, editor->chapter, false);
  ViewUSFMDialog dialog (filename);
  dialog.run ();
  if (dialog.changed) {
    reload_project ();
  }
}


void MainWindow::on_insert_special_character_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_insert_special_character ();
}


void MainWindow::on_insert_special_character ()
{
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  extern Settings * settings;
  vector <ustring> characters;
  vector <ustring> descriptions;
  characters.push_back ("­");
  descriptions.push_back ("Soft hyphen");
  characters.push_back (" ");
  descriptions.push_back ("No-break space");
  characters.push_back ("“");
  descriptions.push_back ("Left double quotation mark");
  characters.push_back ("”");
  descriptions.push_back ("Right double quotation mark");
  characters.push_back ("‘");
  descriptions.push_back ("Left single quotation mark");
  characters.push_back ("’");
  descriptions.push_back ("Right single quotation mark");
  characters.push_back ("«");
  descriptions.push_back ("Left-pointing double angle quotation mark");
  characters.push_back ("»");
  descriptions.push_back ("Right-pointing double angle quotation mark");
  RadiobuttonDialog dialog ("Insert character", "Insert special character", descriptions, settings->session.special_character_selection);
  if (dialog.run () != GTK_RESPONSE_OK) return;
  settings->session.special_character_selection = dialog.selection;
  editor->text_insert (characters[dialog.selection]);
}


void MainWindow::on_preferences_graphical_interface_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_graphical_interface ();
}


void MainWindow::on_preferences_graphical_interface ()
{
  InterfacePreferencesDialog dialog (0);
  dialog.run ();
}


/*
|
|
|
|
|
Keyterms
|
|
|
|
|
*/


void MainWindow::activate_keyterms_object ()
{
  if (!keytermsgui) {
    keytermsgui = new KeytermsGUI (vbox_keyterms, textview_keyterm_text);
    g_signal_connect ((gpointer) keytermsgui->signal, "clicked", G_CALLBACK (on_keyterms_new_reference), gpointer(this));
  }
}


void MainWindow::destroy_keyterms_object ()
{
  if (keytermsgui) 
    delete keytermsgui;
}


void MainWindow::on_keyterms_new_reference (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->check_move_new_reference ();
}


void MainWindow::check_move_new_reference ()
{
  Reference reference (keytermsgui->new_reference_showing->book, keytermsgui->new_reference_showing->chapter, keytermsgui->new_reference_showing->verse);
  navigation.display (reference);
}


/*
|
|
|
|
|
Backup
|
|
|
|
|
*/


void MainWindow::on_project_backup_incremental_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_project_backup_incremental ();
}


void MainWindow::on_project_backup_incremental ()
{
  editorsgui->save ();
  git_command_pause (true);
  backup_make_incremental ();
  git_command_pause (false);
}


void MainWindow::on_project_backup_flexible_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_project_backup_flexible ();
}


void MainWindow::on_project_backup_flexible ()
{
  editorsgui->save ();
  git_command_pause (true);
  BackupDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK) {
    backup_make_flexible ();
  }
  git_command_pause (false);
}


/*
|
|
|
|
|
Git
|
|
|
|
|
*/


void MainWindow::on_view_git_tasks_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_view_git_tasks ();
}


void MainWindow::on_view_git_tasks ()
{
  ViewGitDialog dialog (0);
  dialog.run ();  
}


void MainWindow::on_preferences_remote_git_repository_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_remote_git_repository ();
}


void MainWindow::on_preferences_remote_git_repository ()
{
  editorsgui->save ();
  GitSetupDialog dialog (0);
  if (dialog.run () == GTK_RESPONSE_OK) 
    reload_project ();  
}


void MainWindow::on_git_reopen_project ()
{
  if (git_reopen_project) {
    reload_project ();
    gtkw_dialog_info (mainwindow, 
                    "The project data has been changed by somebody else.\n"
                    "The changes are of such a nature, that it was needed\n"
                    "to reload the project, so as to update everything.\n"
                    "That has now been done.\n");
    git_reopen_project = false;
  }
}


void MainWindow::on_project_changes_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_project_changes ();
}


void MainWindow::on_project_changes ()
{
  // Save even the very latest changes.
  editorsgui->save ();
  // The changes checker will generate git tasks. Pause git.
  git_command_pause (true);
  // Do the actual changes dialog. 
  // It will delete the unwanted git tasks.
  References references (liststore_references, treeview_references, treecolumn_references);
  ChangesDialog dialog (&references);
  dialog.run ();
  // Resume git operations.
  git_command_pause (false);
}


void MainWindow::on_edit_revert_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_edit_revert ();
}


void MainWindow::on_edit_revert ()
{
  editorsgui->save ();
  RevertDialog dialog (&navigation.reference);
  if (dialog.run () == GTK_RESPONSE_OK) {
    reload_project ();
  }
}


void MainWindow::git_update_intervals_initialize ()
{
  // Make containers with all projects to be updated, and their intervals.
  extern Settings * settings;
  vector <ustring> projects = projects_get_all ();
  for (unsigned int i = 0; i < projects.size (); i++) {
    ProjectConfiguration * projectconfig = settings->projectconfig (projects[i]);
    if (projectconfig->git_use_remote_repository_get ()) {
      git_update_intervals[projects[i]] = 0;
    }
  }
  // Start the timer.
  git_update_interval_event_id = g_timeout_add_full (G_PRIORITY_DEFAULT, 1000, GSourceFunc (on_git_update_timeout), gpointer(this), NULL);
}


bool MainWindow::on_git_update_timeout (gpointer user_data)
{
  ((MainWindow *) user_data)->git_update_timeout ();
  return true;
}


void MainWindow::git_update_timeout ()
// Schedule project update tasks.
{
  // Bail out if git tasks are paused.
  extern Settings * settings;
  if (settings->session.git_pause) return;
  
  // Schedule a task for each relevant project.
  vector <ustring> projects = projects_get_all ();
  for (unsigned int i = 0; i < projects.size(); i++) {
    ProjectConfiguration * projectconfig = settings->projectconfig (projects[i]);
    if (projectconfig->git_use_remote_repository_get ()) {
      int interval = git_update_intervals[projects[i]];
      interval++;
      if (interval >= projectconfig->git_remote_update_interval_get ()) {
        git_schedule (gttUpdateProject, projects[i], 0, 0, projectconfig->git_remote_repository_url_get ());
        interval = 0;
      }
      git_update_intervals[projects[i]] = interval;
    }
  }
}


/*
|
|
|
|
|
Fonts
|
|
|
|
|
*/


void MainWindow::on_view_text_font_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_text_font ();
}


void MainWindow::on_text_font ()
{
  // Get the font and colour settings, either from the project, if it is opened, 
  // or else from genconfig.
  extern Settings * settings;
  bool defaultfont = settings->genconfig.text_editor_font_default_get ();
  ustring fontname = settings->genconfig.text_editor_font_name_get();
  bool defaultcolour = settings->genconfig.text_editor_default_color_get ();
  unsigned int normaltextcolour = settings->genconfig.text_editor_normal_text_color_get ();
  unsigned int backgroundcolour = settings->genconfig.text_editor_background_color_get ();
  unsigned int selectedtextcolour = settings->genconfig.text_editor_selected_text_color_get ();
  unsigned int selectioncolour = settings->genconfig.text_editor_selection_color_get ();
  Editor * editor = editorsgui->focused_editor ();
  if (editor) {
    ProjectConfiguration * projectconfig = settings->projectconfig (editor->project);
    defaultfont = projectconfig->editor_font_default_get ();
    fontname = projectconfig->editor_font_name_get ();
    defaultcolour = settings->genconfig.text_editor_default_color_get ();
    normaltextcolour = projectconfig->editor_normal_text_color_get ();
    backgroundcolour = projectconfig->editor_background_color_get ();
    selectedtextcolour = projectconfig->editor_selected_text_color_get ();
    selectioncolour = projectconfig->editor_selection_color_get ();
  }

  // Display font selection dialog. 
  FontColorDialog dialog (defaultfont, fontname,
                          defaultcolour, normaltextcolour, backgroundcolour, selectedtextcolour,  selectioncolour);
  if (dialog.run () != GTK_RESPONSE_OK) return;

  // Save font, and set it.
  settings->genconfig.text_editor_font_default_set (dialog.new_use_default_font);
  settings->genconfig.text_editor_font_name_set (dialog.new_font);
  settings->genconfig.text_editor_default_color_set (dialog.new_use_default_color);
  settings->genconfig.text_editor_normal_text_color_set (dialog.new_normal_text_color);
  settings->genconfig.text_editor_background_color_set (dialog.new_background_color);
  settings->genconfig.text_editor_selected_text_color_set (dialog.new_selected_text_color);
  settings->genconfig.text_editor_selection_color_set (dialog.new_selection_color);
  if (editor) {
    ProjectConfiguration * projectconfig = settings->projectconfig (editor->project);
    projectconfig->editor_font_default_set (dialog.new_use_default_font);
    projectconfig->editor_font_name_set (dialog.new_font);
    projectconfig->editor_default_color_set (dialog.new_use_default_color);
    projectconfig->editor_normal_text_color_set (dialog.new_normal_text_color);
    projectconfig->editor_background_color_set (dialog.new_background_color);
    projectconfig->editor_selected_text_color_set (dialog.new_selected_text_color);
    projectconfig->editor_selection_color_set (dialog.new_selection_color);
  }
  set_fonts ();
}


void MainWindow::on_view_notes_font_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_notes_font ();
}


void MainWindow::on_notes_font ()
{
  extern Settings * settings;
  FontColorDialog dialog (settings->genconfig.notes_editor_font_default_get (), 
                          settings->genconfig.notes_editor_font_name_get (),
                          settings->genconfig.notes_editor_default_color_get (),
                          settings->genconfig.notes_editor_normal_text_color_get (), 
                          settings->genconfig.notes_editor_background_color_get (),
                          settings->genconfig.notes_editor_selected_text_color_get (), 
                          settings->genconfig.notes_editor_selection_color_get ());
  if (dialog.run () == GTK_RESPONSE_OK) {
    settings->genconfig.notes_editor_font_default_set (dialog.new_use_default_font);
    settings->genconfig.notes_editor_font_name_set (dialog.new_font);
    settings->genconfig.notes_editor_default_color_set (dialog.new_use_default_color);
    settings->genconfig.notes_editor_normal_text_color_set (dialog.new_normal_text_color);
    settings->genconfig.notes_editor_background_color_set (dialog.new_background_color);
    settings->genconfig.notes_editor_selected_text_color_set (dialog.new_selected_text_color);
    settings->genconfig.notes_editor_selection_color_set (dialog.new_selection_color);
    set_fonts ();
  }
}


void MainWindow::set_fonts ()
{
  // Set font in the text editors. Set text direction too.
  editorsgui->set_fonts ();

  // Set font for the translation notes editor.
  PangoFontDescription *font_desc = NULL;
  extern Settings * settings;
  if (!settings->genconfig.notes_editor_font_default_get ()) {
    font_desc = pango_font_description_from_string (settings->genconfig.notes_editor_font_name_get ().c_str ());
  }
  gtk_widget_modify_font (textview_notes, font_desc);
  gtk_widget_modify_font (textview_note, font_desc);
  if (font_desc) pango_font_description_free (font_desc);
    
  // Set the colors for the translation notes editor.
  if (settings->genconfig.notes_editor_default_color_get ()) {
    color_widget_default (textview_notes);
    color_widget_default (textview_note);
  } else {
    color_widget_set (textview_notes,
                      settings->genconfig.notes_editor_normal_text_color_get (),
                      settings->genconfig.notes_editor_background_color_get (),
                      settings->genconfig.notes_editor_selected_text_color_get (),
                      settings->genconfig.notes_editor_selection_color_get ());
    color_widget_set (textview_note,
                      settings->genconfig.notes_editor_normal_text_color_get (),
                      settings->genconfig.notes_editor_background_color_get (),
                      settings->genconfig.notes_editor_selected_text_color_get (),
                      settings->genconfig.notes_editor_selection_color_get ());
  }
}


void MainWindow::on_printer_font_activate (GtkMenuItem * menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_printer_font ();
}


void MainWindow::on_printer_font ()
{
  FontDialog fontdialog (0);
  fontdialog.run();
}


/*
|
|
|
|
|
Outline
|
|
|
|
|
*/


void MainWindow::outline_initialize ()
{
  if (!outline) {
    outline = new Outline (vbox_outline);
    g_signal_connect ((gpointer) outline->reference_changed_signal, "clicked", G_CALLBACK (on_button_outline_clicked), gpointer(this));
  }
  outline->operate = true;
  extern Settings * settings;
  outline->goto_reference (settings->genconfig.project_get(), navigation.reference);
}


void MainWindow::outline_destroy ()
{
  if (outline) delete outline;
  outline = NULL;
}


void MainWindow::on_button_outline_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_button_outline ();
}


void MainWindow::on_button_outline ()
{
  Reference reference (navigation.reference);
  reference.chapter = outline->newchapter;
  reference.verse = convert_to_string (outline->newverse);
  navigation.display (reference);
}


/*
|
|
|
|
|
Interprocess communications
|
|
|
|
|
*/


void MainWindow::on_ipc_method_called (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_ipc_method ();
}


void MainWindow::on_ipc_method ()
{
  // Interprocess Communication pointer.
  extern InterprocessCommunication * ipc;
  
  // Settings pointer.
  extern Settings * settings;
  
  // Handle call for a new git job.
  if (ipc->method_called_type == ipcctGitJobDescription) {
    if (!settings->session.git_pause) {
      vector <ustring> task = git_get_next_task ();
      if (!task.empty ()) {
        ipc->send (ipcstBibleditGit, ipcctGitJobDescription, task);
        // The chapter state looks whether something changed in the chapter 
        // now opened while we pull changes from the remote repository.
        gitchapterstate = NULL;
        if (convert_to_int (task[0]) == gttUpdateProject) {
          if (task[1] == settings->genconfig.project_get ()) {
            gitchapterstate = new GitChapterState (task[1], navigation.reference.book, navigation.reference.chapter);      
          }
        }
      }
    }
  }
  
  // Handle a job done.
  else if (ipc->method_called_type == ipcctGitTaskDone) {
    // Get the errors given by this task.
    vector <ustring> error = ipc->get_payload (ipcctGitTaskDone);
    if (error.empty ()) {
      // No errors: erase the task.
      git_erase_task_done ();
    } else {
      // Errors: Resolve any conflicts in the project the latest task refers to.  
      vector <ustring> task = git_get_next_task ();
      if (!task.empty ()) {
        git_resolve_conflicts (task[1], error);
      }
      // Task failed: re-queue it.
      git_fail_task_done (); 
    }
    // Set a flag if the state of the currently opened chapter changed.
    // (This must be done after conflicting merges have been resolved, as that
    //  could affect the chapter now opened).
    if (gitchapterstate) {
      if (gitchapterstate->changed ()) {
        git_reopen_project = true;
      }
      delete gitchapterstate;
      gitchapterstate = NULL;
    }
  }
}


/*
|
|
|
|
|
Reporting and Planning
|
|
|
|
|
*/


void MainWindow::on_preferences_reporting_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_reporting ();
}


void MainWindow::on_preferences_reporting ()
{
  ReportingSetupDialog dialog (0);
  dialog.run ();
}


void MainWindow::on_editstatus_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_editstatus ();
}


void MainWindow::on_editstatus ()
{
  extern Settings * settings;
  EditStatusDialog dialog (settings->genconfig.project_get (), navigation.reference.book, navigation.reference.chapter);
  dialog.run ();
}


void MainWindow::on_view_status_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_view_status ();
}


void MainWindow::on_view_status ()
{
  ViewStatusDialog dialog (0);
  dialog.run ();  
}


void MainWindow::on_edit_planning_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_edit_planning ();
}


void MainWindow::on_edit_planning ()
{
  extern Settings * settings;
  planning_edit (settings->genconfig.project_get ());
}


void MainWindow::on_view_planning_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_view_planning ();
}


void MainWindow::on_view_planning ()
{
  extern Settings * settings;
  planning_produce_report (settings->genconfig.project_get ());
}


void MainWindow::on_preferences_planning_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_planning ();
}


void MainWindow::on_preferences_planning ()
{
  PlanningSetupDialog dialog (0);
  dialog.run ();
}


/*
|
|
|
|
|
Resources
|
|
|
|
|
*/


void MainWindow::activate_resources_object ()
// Create the resources object if it does not yet exist.
{
  if (!resourcesgui) {
    // The purpose of the next line is to prevent creation of two resource objects
    // in case that bibledit starts with the Resources tab activated.
    resourcesgui++;
    // Create object.
    resourcesgui = new ResourcesGUI (vbox_resources);
  }
}


void MainWindow::destroy_resources_object ()
{
  if (resourcesgui) 
    delete resourcesgui;
}


void MainWindow::on_file_resources_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources ();
}


void MainWindow::on_file_resources ()
{
  // Switch the view to the resources (creating the object if it was not there).
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntResources);
  // Set the Edit menu option sensitive only if a resource has been opened.
  bool enable = false;
  if (resourcesgui) {
    if (resourcesgui->focused_resource ()) {
      enable = true;
    }
  }
  gtk_widget_set_sensitive (file_resources_edit, enable);
}


void MainWindow::on_file_resources_open_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources_open ();
}


void MainWindow::on_file_resources_open ()
{
  resource_open (resourcesgui);
}


void MainWindow::on_file_resources_close_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources_close ();
}


void MainWindow::on_file_resources_close ()
{
  resource_close (resourcesgui);
}


void MainWindow::on_file_resources_new_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources_new ();
}


void MainWindow::on_file_resources_new ()
{
  NewResourceDialog dialog ("");
  dialog.run ();
}


void MainWindow::on_file_resources_edit_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources_edit ();
}


void MainWindow::on_file_resources_edit ()
{
  Resource * resource = resourcesgui->focused_resource ();
  if (!resource) return;
  ustring templatefile = resource->template_get ();
  NewResourceDialog dialog (templatefile);
  if (dialog.run () == GTK_RESPONSE_OK) {
    resourcesgui->reload (templatefile, dialog.edited_template_file);
  }
}


void MainWindow::on_file_resources_delete_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_resources_delete ();
}


void MainWindow::on_file_resources_delete ()
{
  resource_delete (resourcesgui);
}


/*
|
|
|
|
|
Text Editor
|
|
|
|
|
 */


void MainWindow::on_editor_reload_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_editor_reload ();
}


void MainWindow::on_editor_reload ()
{
  // Get the focused editor, if none, bail out.
  Editor * editor = editorsgui->focused_editor ();
  if (!editor) return;
  // Create the reference where to go to after the project has been reopened.
  // The reference should be obtained before closing the project,
  // so that the chapter number to go to is accessible.
  Reference reference (navigation.reference);
  reference.chapter = editor->reload_chapter_number;
  if (editor->reload_chapter_number == 0)
    reference.verse = "0";
  // Reopen.
  reload_project ();  
  // Go to the right reference.
  navigation.display (reference);
}


void MainWindow::on_editorsgui_focus_button_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_editorsgui_focus_button ();
}


void MainWindow::on_editorsgui_focus_button ()
{
  // Get the focused editor and the project.
  Editor * editor = editorsgui->focused_editor ();
  ustring project;
  if (editor) project = editor->project;

  // Enable or disable widgets depending on whether an editor is focused.
  enable_or_disable_widgets (editor);
  
  // Set the project in the titlebar.
  set_titlebar (project);
  
  // Inform the merge GUI about the editors.
  mergegui->set_focused_editor (editor);
  mergegui->set_visible_editors (editorsgui->visible_editors_get ());

  // If we've no project bail out.
  if (project.empty ()) return;

  // Settings.
  extern Settings * settings;
  
  // Re-initialize Navigation.
  navigation.set_project (project, false);
  Reference reference (settings->genconfig.book_get (), convert_to_int (settings->genconfig.chapter_get ()), settings->genconfig.verse_get ());
  navigation.display (reference);
  // Some rumbling internal logic causes the verse to jump to 0 in several cases.
  // To resolve this in a rough manner, after a delay, that is, after the 
  // rumbling is over, cause the navigator to go to the desired verse.
  navigation.display_delayed (reference.verse);
  
  // Set the available books for search/replace functions.
  vector <unsigned int> books = project_get_books (project);
  set<unsigned int> selection (books.begin(), books.end());
  settings->session.selected_books = selection;

  // Redisplay the project notes.
  notes_redisplay ();
  
  // Open the right stylesheet.
  ProjectConfiguration * projectconfig = settings->projectconfig (project);
  settings->genconfig.stylesheet_set (projectconfig->stylesheet_get());
  stylesheet_open_named (settings->genconfig.stylesheet_get());
}


void MainWindow::jump_start_editors (const ustring& project)
// Jump starts the text editors.
{
  enable_or_disable_widgets (false);
  editorsgui->jumpstart (project);
  g_signal_connect ((gpointer) editorsgui->new_verse_button, "clicked", G_CALLBACK (on_new_verse_signalled), gpointer(this));
  g_signal_connect ((gpointer) editorsgui->new_styles_button, "clicked", G_CALLBACK (on_editor_style_changed), gpointer(this));
  g_signal_connect ((gpointer) editorsgui->word_double_clicked_button, "clicked", G_CALLBACK (on_send_word_to_toolbox_signalled), gpointer(this));
  g_signal_connect ((gpointer) editorsgui->editor_reload_button, "clicked", G_CALLBACK (on_editor_reload_clicked), gpointer(this));
  g_signal_connect ((gpointer) editorsgui->editor_changed_button, "clicked", G_CALLBACK (on_editorsgui_changed_clicked), gpointer(this));
  // Set the quick references view.
  editorsgui->quick_references_textview_set (textview_quick_refs);
}


void MainWindow::on_editorsgui_changed_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_editorsgui_changed ();
}


void MainWindow::on_editorsgui_changed ()
{
  mergegui->editors_changed ();
}


void MainWindow::reload_project ()
// This function reloads the projects.
{
  // Project.
  extern Settings * settings;
  ustring project = settings->genconfig.project_get();
  if (project.empty ()) return;

  // Store the reference where to go to after the project has been reloaded.
  Reference reference (navigation.reference);

  // As anything could have happened to the data in the project, force a reload of the navigator.
  navigation.set_project (project, true);

  // Navigate to the old place.
  navigation.display (reference);

  // Reload all editors.
  editorsgui->reload_chapter (reference.book, reference.chapter);
}


/*
|
|
|
|
|
Merge
|
|
|
|
|
*/


void MainWindow::on_file_projects_merge_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_file_projects_merge ();
}


void MainWindow::on_file_projects_merge ()
{
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook_tools), tapntMerge);
}


void MainWindow::on_mergegui_get_text_button_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_mergegui_get_text_button ();
}


void MainWindow::on_mergegui_get_text_button ()
{
  mergegui->main_project_data = editorsgui->get_text (mergegui->current_master_project);
  mergegui->edited_project_data = editorsgui->get_text (mergegui->current_edited_project);
  mergegui->book = navigation.reference.book;
  mergegui->chapter = navigation.reference.chapter;
}


void MainWindow::on_mergegui_new_reference_button_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_mergegui_new_reference_button ();
}


void MainWindow::on_mergegui_new_reference_button ()
{
  Reference reference (mergegui->book, mergegui->chapter, "0");
  navigation.display (reference);
}


void MainWindow::on_mergegui_save_editors_button_clicked (GtkButton *button, gpointer user_data)
{
  ((MainWindow *) user_data)->on_mergegui_save_editors_button ();
}


void MainWindow::on_mergegui_save_editors_button ()
{
  editorsgui->save ();
}


/*
|
|
|
|
|
Diglot
|
|
|
|
|
*/


void MainWindow::on_preferences_filters_activate (GtkMenuItem *menuitem, gpointer user_data)
{
  ((MainWindow *) user_data)->on_preferences_filters ();
}


void MainWindow::on_preferences_filters ()
{
  FiltersDialog dialog (0);
  dialog.run ();
}


// Todo make system that a mail goes out when a commit has been made to git.
// Perhaps on Savannah? Tried, we may have hooks locally that store information in increments.
// And once we push it, we may send that information to a mailing list, and delete that information.

// Todo editing a note with Songs in the Ndebele project goes wrong.
