/*
 ** Copyright (©) 2003-2009 Teus Benschop.
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
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 **  
 */

#ifndef INCLUDED_WINDOW_OUTLINE_H
#define INCLUDED_WINDOW_OUTLINE_H

#include <gtk/gtk.h>
#include "ustring.h"
#include "reference.h"
#include "windows.h"
#include "outline.h"

class WindowOutline : public WindowBase
{
public:
  WindowOutline(GtkAccelGroup *accelerator_group, bool startup, GtkWidget * parent_box);
  virtual ~WindowOutline();
  void go_to(const ustring& project, Reference& reference);
  Outline * outline;
protected:
  GtkWidget *vbox;
private:
  Reference myreference;
};

#endif
