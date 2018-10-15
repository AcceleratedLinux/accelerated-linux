/* gnu_java_awt_peer_gtk_GtkEmbeddedWindowPeer.c -- Native
   implementation of GtkEmbeddedWindowPeer
   Copyright (C) 2003, 2006  Free Software Foundation, Inc.

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
#include "gnu_java_awt_peer_gtk_GtkEmbeddedWindowPeer.h"

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkEmbeddedWindowPeer_create
  (JNIEnv *env, jobject obj, jlong socket_id)
{
  GtkWidget *window;
  GtkWidget *fixed;

  gdk_threads_enter ();

  gtkpeer_set_global_ref (env, obj);

  window = gtk_plug_new ((GdkNativeWindow) socket_id);

  gtk_window_set_decorated (GTK_WINDOW (window), FALSE);

  fixed = gtk_fixed_new ();
  gtk_container_add (GTK_CONTAINER (window), fixed);

  gtk_widget_show (fixed);

  gtkpeer_set_widget (env, obj, window);

  gdk_threads_leave ();
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkEmbeddedWindowPeer_construct
  (JNIEnv *env, jobject obj, jlong socket_id)
{
  void *ptr;

  gdk_threads_enter ();

  ptr = gtkpeer_get_widget (env, obj);

  if (GTK_WIDGET_REALIZED (GTK_WIDGET (ptr)))
    g_printerr ("ERROR: GtkPlug is already realized\n");

  gtk_plug_construct (GTK_PLUG (ptr), (GdkNativeWindow) socket_id);

  gdk_threads_leave ();
}
