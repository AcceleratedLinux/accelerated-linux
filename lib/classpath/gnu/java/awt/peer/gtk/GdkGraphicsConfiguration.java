/* GdkGraphicsConfiguration.java -- describes characteristics of graphics
   Copyright (C) 2000, 2001, 2002, 2003, 2004, 2006 Free Software Foundation

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

import java.awt.BufferCapabilities;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.ImageCapabilities;
import java.awt.Rectangle;
import java.awt.Transparency;

import java.awt.geom.AffineTransform;

import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.VolatileImage;

public class GdkGraphicsConfiguration 
  extends GraphicsConfiguration
{
  GdkScreenGraphicsDevice gdkScreenGraphicsDevice;
  
  ColorModel opaqueColorModel;

  ColorModel bitmaskColorModel;

  ColorModel translucentColorModel;
  
  public GdkGraphicsConfiguration(GdkScreenGraphicsDevice dev)
  {
    gdkScreenGraphicsDevice = dev;
    
    opaqueColorModel = new DirectColorModel(32, 0xFF0000, 0xFF00, 0xFF, 0);
    bitmaskColorModel = new DirectColorModel(32, 0xFF0000, 0xFF00, 0xFF, 0x1000000);
    translucentColorModel = new DirectColorModel(32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);
  }

  public GraphicsDevice getDevice()
  {
    return gdkScreenGraphicsDevice;
  }

  public BufferedImage createCompatibleImage(int w, int h)
  {
    return new BufferedImage(w, h, BufferedImage.TYPE_INT_ARGB);
  }

  public BufferedImage createCompatibleImage(int w, int h, 
                                             int transparency)
  {
    return createCompatibleImage(w, h);
  }

  public VolatileImage createCompatibleVolatileImage(int w, int h)
  {
    return new GtkVolatileImage(w, h);
  }

  public VolatileImage createCompatibleVolatileImage(int w, int h,
                                                     ImageCapabilities caps)
    throws java.awt.AWTException
  {
    return new GtkVolatileImage(w, h, caps);
  }

  public ColorModel getColorModel()
  {
    return opaqueColorModel;
  }

  public ColorModel getColorModel(int transparency)
  {
    switch (transparency)
    {
      case Transparency.OPAQUE:
        return opaqueColorModel;
      case Transparency.BITMASK:
        return bitmaskColorModel;
      default:
      case Transparency.TRANSLUCENT:
        return translucentColorModel;
    }
  }

  public AffineTransform getDefaultTransform()
  {
    // FIXME: extract the GDK DPI information here.
    return new AffineTransform();
  }

  public AffineTransform getNormalizingTransform()
  {
    // FIXME: extract the GDK DPI information here.
    return new AffineTransform();
  }

  public Rectangle getBounds()
  {
    return gdkScreenGraphicsDevice.getBounds();
  }

  public BufferCapabilities getBufferCapabilities()
  {
    return new BufferCapabilities(getImageCapabilities(), 
                                  getImageCapabilities(),
                                  BufferCapabilities.FlipContents.UNDEFINED);
  }

  public ImageCapabilities getImageCapabilities()
  {
    return new ImageCapabilities(false);
  }

  public VolatileImage createCompatibleVolatileImage(int width, int height, int transparency)
  {
      // FIXME: support the transparency argument
    return new GtkVolatileImage(width, height);
  }

}
