/* gtkmenubarpeer.c -- Native implementation of GtkMenuBarPeer
   Copyright (C) 1999, 2006 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


#include "gtkpeer.h"
#include "gnu_java_awt_peer_gtk_GtkMenuBarPeer.h"

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkMenuBarPeer_create
  (JNIEnv *env, jobject obj)
{
  GtkWidget *widget;

  gdk_threads_enter ();
  
  gtkpeer_set_global_ref (env, obj);

  widget = gtk_menu_bar_new ();
  gtk_widget_show (widget);

  gtkpeer_set_widget (env, obj, widget);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkMenuBarPeer_addMenu
  (JNIEnv *env, jobject obj, jobject menupeer)
{
  void *mbar, *menu;

  gdk_threads_enter ();

  mbar = gtkpeer_get_widget (env, obj);
  menu = gtkpeer_get_widget (env, menupeer);

  gtk_menu_shell_append (GTK_MENU_SHELL (mbar), GTK_WIDGET (menu));

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkMenuBarPeer_delMenu
  (JNIEnv *env, jobject obj, jint index)
{
  void *ptr;
  GList *list;

  gdk_threads_enter ();

  ptr = gtkpeer_get_widget (env, obj);

  list = gtk_container_get_children (GTK_CONTAINER (ptr));
  list = g_list_nth (list, index);
  gtk_container_remove (GTK_CONTAINER (ptr), GTK_WIDGET (list->data));

  gdk_threads_leave ();
}
