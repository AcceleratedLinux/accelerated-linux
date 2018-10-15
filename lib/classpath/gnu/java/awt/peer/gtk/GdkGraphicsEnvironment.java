/* GdkGraphicsEnvironment.java -- information about the graphics environment
   Copyright (C) 2004, 2005, 2006  Free Software Foundation, Inc.

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


package gnu.java.awt.peer.gtk;

import gnu.classpath.Configuration;
import gnu.java.awt.ClasspathGraphicsEnvironment;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.HeadlessException;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;
import java.util.Locale;

import gnu.classpath.Pointer;

public class GdkGraphicsEnvironment extends ClasspathGraphicsEnvironment
{
  private final int native_state = GtkGenericPeer.getUniqueInteger ();
  
  private GdkScreenGraphicsDevice defaultDevice;
  
  private GdkScreenGraphicsDevice[] devices;

  /**
   * The pointer to the native display resource.
   *
   * This field is manipulated by native code. Don't change or remove
   * without adjusting the native code.
   */
  private Pointer display;

  static
  {
    if (Configuration.INIT_LOAD_LIBRARY)
      {
        System.loadLibrary("gtkpeer");
      }

    GtkToolkit.initializeGlobalIDs();
    initIDs();
  }
  
  private static native void initIDs();
  
  public GdkGraphicsEnvironment ()
  {
    nativeInitState();
  }
  
  native void nativeInitState();

  public GraphicsDevice[] getScreenDevices ()
  {
    if (devices == null)
      {
        devices = nativeGetScreenDevices();
      }
    
    return (GraphicsDevice[]) devices.clone();
  }
  
  private native GdkScreenGraphicsDevice[] nativeGetScreenDevices();

  public GraphicsDevice getDefaultScreenDevice ()
  {
    if (GraphicsEnvironment.isHeadless ())
      throw new HeadlessException ();
    
    synchronized (GdkGraphicsEnvironment.class)
      {
        if (defaultDevice == null)
          {
            defaultDevice = nativeGetDefaultScreenDevice();
          }
      }
    
    return defaultDevice;
  }
  
  private native GdkScreenGraphicsDevice nativeGetDefaultScreenDevice();

  public Graphics2D createGraphics (BufferedImage image)
  {
    Raster raster = image.getRaster();
    if(raster instanceof CairoSurface)
      return ((CairoSurface)raster).getGraphics();

    return new BufferedImageGraphics( image );
  }
  
  private native int nativeGetNumFontFamilies();
  private native void nativeGetFontFamilies(String[] family_names);

  public Font[] getAllFonts ()
  {
    throw new java.lang.UnsupportedOperationException ();
  }

  public String[] getAvailableFontFamilyNames ()
  {
    String[] family_names;
    int array_size;

    array_size = nativeGetNumFontFamilies();
    family_names = new String[array_size];

    nativeGetFontFamilies(family_names);
    return family_names;
  }

  public String[] getAvailableFontFamilyNames (Locale l)
  {
    throw new java.lang.UnsupportedOperationException ();
  }

  /**
   * Used by GtkMouseInfoPeer.
   */ 
  native int[] getMouseCoordinates();
  native boolean isWindowUnderMouse(GtkWindowPeer windowPeer);
  
  public WritableRaster createRaster(ColorModel cm, SampleModel sm)
  {
    if (CairoSurface.isCompatibleSampleModel(sm)
        && CairoSurface.isCompatibleColorModel(cm))
      return new CairoSurface(sm.getWidth(), sm.getHeight());
    else
      return null;
  }
}
