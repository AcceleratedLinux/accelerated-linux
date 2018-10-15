/* gstreamer_io_peer.c -- Implements native methods for class
   GStreamerNativePeer
   Copyright (C) 2007 Free Software Foundation, Inc.
   
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

#include <stdio.h>
#include <string.h>

#include <jni.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <gdk/gdk.h>

#include <gst/gst.h>

#include "jcl.h"

#include "gst_peer.h"

#include "gnu_javax_sound_sampled_gstreamer_io_GstAudioFileReaderNativePeer.h"

#include "gst_classpath_src.h"
#include "gst_input_stream.h"

/* for caching */
static jfieldID fileFID = NULL;
static jfieldID pointerDataID = NULL;

static jfieldID mimetypeFID = NULL;
static jfieldID endiannessFID = NULL;
static jfieldID channelsFID = NULL;
static jfieldID rateFID = NULL;
static jfieldID widthFID = NULL;
static jfieldID depthFID = NULL;
static jfieldID isSignedFID = NULL;
static jfieldID nameFID = NULL;
static jfieldID layerFID = NULL;
static jfieldID bitrateFID = NULL;
static jfieldID framedFID = NULL;
static jfieldID typeFID = NULL;

typedef struct _AudioProperties AudioProperties;
struct _AudioProperties
{
  /*
   * NOTE: descriptions of the properties are taken from:
   * http://gstreamer.freedesktop.org/data/doc/gstreamer/head/pwg/html/section-types-definitions.html#table-audio-types
   */
  
  /* decoder name */
  const char *name;
  
  /* audio endiannes */
  const char *endianness;
  
  /* header size */
  const char *header_size;
  
  /* mime */  
  const char *mimetype;

  /* The sample rate of the data, in samples (per channel) per second */
  const char *samplerate;
  
  /* The number of channels of audio data */
  const char *channels;

  const char *layer;
  
  const char *bitrate;
  
  const char *framed;
  
  /*
   *  Defines if the values of the integer samples are signed or not.
   * Signed samples use one bit to indicate sign (negative or positive)
   * of the value. Unsigned samples are always positive.
   */
  const char *signess;
  
  /* */
  const char *rate;
  
  /* Number of bits allocated per sample. */
  const char *width;

  /*
   * The number of bits used per sample.
   * If the depth is less than the width, the low bits are assumed to be the
   * ones used. For example, a width of 32 and a depth of 24 means that
   * each sample is stored in a 32 bit word, but only the low
   * 24 bits are actually used.
   */
  const char *depth;
  
  /*
   * This is set in the case of the mpeg files.
   */
  const char *type;
  
  gboolean done;
};

/* ***** PRIVATE FUNCTIONS DECLARATION ***** */

static gboolean
set_strings (JNIEnv *env, const AudioProperties *properties, jobject header);

static gboolean
typefind_callback(GstElement *typefind, guint probability, const GstCaps *caps,
                  gpointer data);

static void
element_added (GstBin *bin, GstElement *element, gpointer data);

static void
new_decoded_pad (GstElement *decoder, GstPad *pad,
                 gboolean last, gpointer data);
                 
static gboolean
fill_info (GstElement *decoder, AudioProperties *properties);

static gchar *
get_string_property (const GstStructure *structure, const gchar *property);

static gchar *
get_boolean_property (const GstStructure *structure, const gchar *property);
                                         
static gboolean
set_string (JNIEnv *env, jobject header, jfieldID fieldID,
            const gchar *property);

static void
free_properties (AudioProperties *properties);

static void
reset_properties (AudioProperties *properties);

static jboolean process_audio (GstElement *source, JNIEnv *env, jobject header);

/* ***** END: PRIVATE FUNCTIONS DECLARATION ***** */

/* ***** NATIVE FUNCTIONS ***** */

JNIEXPORT void JNICALL
Java_gnu_javax_sound_sampled_gstreamer_io_GstAudioFileReaderNativePeer_init_1id_1cache
  (JNIEnv *env, jclass clazz  __attribute__ ((unused)))
{
  jclass pointerClass = NULL;
  jclass GstHeader = NULL;
  
  GstHeader = JCL_FindClass(env, "gnu/javax/sound/sampled/gstreamer/io/GstAudioFileReaderNativePeer$GstHeader");
  fileFID = (*env)->GetFieldID(env, GstHeader, "file", "Ljava/lang/String;");

  mimetypeFID = (*env)->GetFieldID(env, GstHeader, "mimetype",
                                   "Ljava/lang/String;");
  endiannessFID = (*env)->GetFieldID(env, GstHeader, "endianness",
                                     "Ljava/lang/String;");
  channelsFID = (*env)->GetFieldID(env, GstHeader, "channels",
                                   "Ljava/lang/String;");
  rateFID = (*env)->GetFieldID(env, GstHeader, "rate", "Ljava/lang/String;");
  widthFID = (*env)->GetFieldID(env, GstHeader, "width", "Ljava/lang/String;");
  depthFID = (*env)->GetFieldID(env, GstHeader, "depth", "Ljava/lang/String;");
  isSignedFID = (*env)->GetFieldID(env, GstHeader, "isSigned",
                                   "Ljava/lang/String;");
  nameFID = (*env)->GetFieldID(env, GstHeader, "name", "Ljava/lang/String;");
  layerFID = (*env)->GetFieldID(env, GstHeader, "layer", "Ljava/lang/String;");
  bitrateFID = (*env)->GetFieldID(env, GstHeader, "bitrate",
                                  "Ljava/lang/String;");
  framedFID = (*env)->GetFieldID(env, GstHeader, "framed",
                                 "Ljava/lang/String;");
  typeFID = (*env)->GetFieldID(env, GstHeader, "type", "Ljava/lang/String;");

#if SIZEOF_VOID_P == 8
  pointerClass = JCL_FindClass (env, "gnu/classpath/Pointer64");
  if (pointerClass != NULL)
    {
      pointerDataID = (*env)->GetFieldID (env, pointerClass, "data", "J");
    }
#else
# if SIZEOF_VOID_P == 4
  pointerClass = JCL_FindClass (env, "gnu/classpath/Pointer32"); 
  if (pointerClass != NULL)
    { 
      pointerDataID = (*env)->GetFieldID(env, pointerClass, "data", "I");
    }
# else
#   error "Pointer size is not supported."
# endif /* SIZEOF_VOID_P == 4 */
#endif /* SIZEOF_VOID_P == 8 */

}

JNIEXPORT jboolean JNICALL
Java_gnu_javax_sound_sampled_gstreamer_io_GstAudioFileReaderNativePeer_gstreamer_1get_1audio_1format_1stream
  (JNIEnv *env, jclass clazz __attribute__ ((unused)), jobject header,
                                                       jobject pointer)
{
  GstInputStream *istream = NULL;
  GstElement *source = NULL;
  gboolean result = JNI_FALSE;
 
  if (header == NULL)
    return JNI_FALSE;
 
  if (pointer == NULL)
    return JNI_FALSE;
 
  gst_init (NULL, NULL);  
   
  istream = (GstInputStream *) get_object_from_pointer (env, pointer,
                                                        pointerDataID);
  if (istream == NULL)
    return JNI_FALSE;
    
  /* init gstreamer */
  gst_init (NULL, NULL);

  /* SOURCE */
  source = gst_element_factory_make ("classpathsrc", "source");
  if (source == NULL)
    {
      g_warning ("unable to create a source");
      return JNI_FALSE;
    }
  g_object_set (G_OBJECT (source), GST_CLASSPATH_SRC_ISTREAM, istream, NULL);

  result = process_audio (source, env, header);
    
  return result;
}

JNIEXPORT jboolean JNICALL
Java_gnu_javax_sound_sampled_gstreamer_io_GstAudioFileReaderNativePeer_gstreamer_1get_1audio_1format_1file
	(JNIEnv *env, jclass clazz __attribute__ ((unused)), jobject header)
{
  /* source file */
  const char *file = NULL;
    
  /* GStreamer elements */
  GstElement *source = NULL;

  jboolean result = JNI_FALSE;
  
  /* java fields */
  jstring _file = NULL;

  _file = (*env)->GetObjectField(env, header, fileFID);
  file = JCL_jstring_to_cstring (env, _file);
  if (file == NULL)
    {
      return JNI_FALSE;
    }

  gst_init (NULL, NULL);
 
  /* create the source element, will be used to read the file */
  source = gst_element_factory_make ("filesrc", "source");
  if (source == NULL)
    {
      JCL_free_cstring (env, _file, file);
      return JNI_FALSE;
    }
  
  /* set the file name */
  g_object_set (G_OBJECT (source), "location", file, NULL);

  result = process_audio (source, env, header);

  /* free stuff */
  JCL_free_cstring (env, _file, file);
  
  return result;
}

/* ***** END: NATIVE FUNCTIONS ***** */

/* ***** PRIVATE FUNCTIONS IMPLEMENTATION ***** */

static jboolean process_audio (GstElement *source, JNIEnv *env, jobject header)
{
  /* will contain the properties we need to put into the given GstHeader */
  AudioProperties *properties = NULL;
  
  /* GStreamer elements */
  GstElement *pipeline = NULL;
  GstElement *decoder = NULL;
  
  GstElement *typefind = NULL;
  
  GstStateChangeReturn res;

  jboolean result = JNI_FALSE;
  
  properties = (AudioProperties *) g_malloc0 (sizeof (AudioProperties));
  if (properties == NULL)
    {
      return result;
    }
  reset_properties(properties);

  /* 
   * create the decoder element, this will decode the stream and retrieve
   * its properties.
   * We connect a signal to this element, to be informed when it is done
   * in decoding the stream and to get the needed informations about the
   * audio file.
   */
  decoder = gst_element_factory_make ("decodebin", "decoder");
  if (decoder == NULL)
    {
      free_properties(properties);
      return result;
    }
  
  /* now, we create a pipeline and fill it with the other elements */
  pipeline = gst_pipeline_new ("pipeline");
  if (pipeline == NULL)
    {
      gst_object_unref (GST_OBJECT (decoder));
      free_properties(properties);   
      return result;
    }
 
  g_signal_connect (decoder, "new-decoded-pad", G_CALLBACK (new_decoded_pad),
                    pipeline);
  g_signal_connect (G_OBJECT (decoder), "element-added",
                    G_CALLBACK (element_added), properties);
  
  /*
   * we get the typefind from the decodebin to catch the additional properties
   * that the decodebin does not expose to us
   */
  typefind = gst_bin_get_by_name (GST_BIN (decoder), "typefind");
  if (typefind != NULL)
    {
      /* 
       * NOTE: the above is not a typo, we can live without the typefind,
       * just, our stream detection will not be as accurate as we would.
       * Anyway, if this fails, there is some problem, probabily a memory
       * error.
       */
       g_signal_connect (G_OBJECT (typefind), "have-type",
                         G_CALLBACK (typefind_callback), properties);
    }
  
  gst_bin_add_many (GST_BIN (pipeline), source, decoder, NULL);
  gst_element_link (source, decoder);
  
  /* 
   * now, we set the pipeline playing state to pause and traverse it
   * to get the info we need.
   */
   
  res = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  if (res == GST_STATE_CHANGE_FAILURE)
    {
      gst_element_set_state (pipeline, GST_STATE_NULL);
      gst_object_unref (GST_OBJECT (pipeline));
      
      free_properties(properties);
      
      return result;
    }
  
  res = gst_element_get_state (pipeline, NULL, NULL, GST_CLOCK_TIME_NONE);
  if (res != GST_STATE_CHANGE_SUCCESS)
    {
      gst_element_set_state (pipeline, GST_STATE_NULL);
      gst_object_unref (GST_OBJECT (pipeline));
      
      free_properties(properties);
      
      return result;
    }
  
  if (fill_info (decoder, properties))
    {
      result = set_strings (env, properties, header);
    }
 
  /* free stuff */
  gst_element_set_state (pipeline, GST_STATE_NULL);
   
  free_properties (properties);
  
  gst_object_unref (GST_OBJECT (pipeline));
 
  return result;
}


static gboolean typefind_callback(GstElement *typefind __attribute__ ((unused)),
                                  guint probability __attribute__ ((unused)),
                                  const GstCaps *caps,
                                  gpointer data)
{
  GstStructure *structure = NULL;
  AudioProperties *properties = NULL;
  
  const char *mpeg = NULL;
  
  properties = (AudioProperties *) data;
  
  structure = gst_caps_get_structure (caps, 0);
  
  /* MIMETYPE */
  properties->mimetype = gst_structure_get_name (structure); 
  mpeg = get_string_property(structure, "mpegversion");
  
  if (mpeg != NULL)
    {
      properties->layer = get_string_property(structure, "layer");
      properties->type = (gchar *) g_malloc0 (_GST_MALLOC_SIZE_);
      g_snprintf ((gpointer) properties->type, _GST_MALLOC_SIZE_,
                  "MPEG%sV%s", mpeg,
                  properties->layer);
              
      g_free ((gpointer) mpeg);
    }
    
  return TRUE;
}

static void
new_decoded_pad (GstElement *decoder  __attribute__ ((unused)),
                 GstPad *pad,
                 gboolean last        __attribute__ ((unused)),
                 gpointer data)
{
  GstElement *pipeline = NULL;
  GstElement *fakesink = NULL;
  GstPad *sinkpad = NULL;
  
  pipeline = (GstElement *) data;
  if (pipeline == NULL)
    return;
  
  fakesink = gst_element_factory_make ("fakesink", NULL);
  if (fakesink == NULL)
    return;
    
  gst_bin_add (GST_BIN (pipeline), fakesink);
  sinkpad = gst_element_get_pad (fakesink, "sink");
  if (GST_PAD_LINK_FAILED (gst_pad_link (pad, sinkpad)))
    {
      gst_bin_remove (GST_BIN (pipeline), fakesink);
    }
  else
    {
      gst_element_set_state (fakesink, GST_STATE_PAUSED);
    }
}

static gboolean
set_strings (JNIEnv *env, const AudioProperties *properties, jobject header)
{
  gboolean result = FALSE;
  
  /* 
   * we only need at least one of them to be sure we can handle this
   * kind of audio data.
   */
      
  /* now, map our properties to the java class */
  set_string (env, header, mimetypeFID, properties->mimetype);
                      
  if (set_string (env, header, endiannessFID, properties->endianness))
    result = JNI_TRUE;
                      
  if (set_string (env, header, channelsFID, properties->channels))
    result = JNI_TRUE;
      
  if (set_string (env, header, rateFID, properties->rate))
    result = JNI_TRUE;
      
  if (set_string (env, header, widthFID, properties->width))
    result = JNI_TRUE;
      
  if (set_string (env, header, depthFID, properties->depth))
    result = JNI_TRUE;
      
  if (set_string (env, header, isSignedFID, properties->signess))
    result = JNI_TRUE;
     
  if (set_string (env, header, nameFID, properties->name))
    result = JNI_TRUE;
     
  /* non primary properties */
  set_string (env, header, layerFID, properties->layer);
  set_string (env, header, bitrateFID, properties->bitrate);
  set_string (env, header, framedFID, properties->framed);
  set_string (env, header, typeFID, properties->type);
 
  return result;    
}

static gboolean fill_info (GstElement *decoder, AudioProperties *properties)
{
  GstIterator *it = NULL;
  gpointer data = NULL;
  gboolean result = FALSE;
  
  it = gst_element_iterate_src_pads (decoder);
  while (gst_iterator_next (it, &data) == GST_ITERATOR_OK)
    {
      GstPad *pad = GST_PAD (data);
      GstCaps *caps;
      
      GstStructure *structure;
      
      const gchar *caps_string = NULL;
     
      caps = gst_pad_get_caps (pad);
      caps_string = gst_caps_to_string (caps);
      
      if (g_str_has_prefix (caps_string, "video"))
        {
          /* no video support, this is an audio library */
          
          g_free ((gpointer) caps_string);
          gst_caps_unref (caps);
          gst_object_unref (pad);
          
          continue; 
        }
      
      g_free ((gpointer) caps_string);
      
      structure = gst_caps_get_structure (GST_CAPS (caps), 0);
      
      /* fill the properties we need */
       
      /* SIGNESS */
      properties->signess = get_boolean_property(structure, "signed");
      if (properties->signess != NULL)
        {
          result = TRUE;
        }
      
      /* ENDIANNESS */
      properties->endianness = get_string_property(structure, "endianness");
      if (properties->endianness != NULL)
        {
          result = TRUE;
        }
      
      /* CHANNELS */
      properties->channels = get_string_property(structure, "channels");
      if (properties->channels != NULL)
        {
          result = TRUE;
        }
      
      /* RATE */
      properties->rate = get_string_property(structure, "rate");
      if (properties->rate != NULL)
        {
          result = TRUE;
        }
      
      /* WIDTH */
      properties->width = get_string_property(structure, "width");
      if (properties->width != NULL)
        {
          result = TRUE;
        }
      
      /* DEPTH */
      properties->depth = get_string_property(structure, "depth");
      if (properties->depth != NULL)
        {
          result = TRUE;
        }
      
      gst_caps_unref (caps);
      gst_object_unref (pad);
    }
     
    return result;
}

static void
free_properties (AudioProperties *properties __attribute__ ((unused)))
{
  /* FIXME this causes a segfault, a string not allocated by us? double free? */
  /*
  if (properties->name != NULL) g_free((gpointer) properties->name);
  if (properties->endianness != NULL) g_free((gpointer) properties->endianness);
  if (properties->channels != NULL) g_free((gpointer) properties->channels);
  if (properties->rate != NULL) g_free((gpointer) properties->rate);
  if (properties->width != NULL) g_free((gpointer) properties->width);
  if (properties->depth != NULL) g_free((gpointer) properties->depth);
  if (properties->layer != NULL) g_free((gpointer) properties->layer);
  if (properties->bitrate != NULL) g_free((gpointer) properties->bitrate);
  if (properties->framed != NULL) g_free((gpointer) properties->framed);
  
  if (properties != NULL) g_free ((gpointer) properties);
  */
}

static void reset_properties (AudioProperties *properties)
{
  properties->done = FALSE;
  properties->signess = FALSE;
  properties->name = NULL;
  properties->endianness = NULL;
  properties->channels = NULL;
  properties->rate = NULL;
  properties->width  = NULL;
  properties->depth = NULL;
  properties->layer = NULL;
  properties->bitrate = NULL;
  properties->framed = NULL;
}

static gchar *get_string_property (const GstStructure *structure,
                                   const gchar *property)
{
  int props = 0;
  gchar *result = NULL;
  
  if (property == NULL)
    {
      return NULL;
    }
  
  /* we don't need more */
  result = (gchar *) g_malloc0 (_GST_MALLOC_SIZE_);
  if (result == NULL)
    {
      /* huston, we have a problem here... */
      return NULL;
    }
  
  if (gst_structure_get_int (structure, property, &props))
    {
      g_snprintf (result, _GST_MALLOC_SIZE_, "%d", props);
    }
  else
    {
      g_free ((gpointer) result);
      return NULL;
    }
    
  return result;
}

static gchar *get_boolean_property (const GstStructure *structure,
                                    const gchar *property)
{
  gchar *result = NULL;
  gboolean props = FALSE;
  
  result = (gchar *) g_malloc0 (_GST_MALLOC_SIZE_);
  if (result == NULL)
    {
      /* huston, we have a problem here... */
      return NULL;
    }
  
  if (gst_structure_get_boolean (structure, property, &props))
    {
      g_snprintf (result, _GST_MALLOC_SIZE_, "%s", (props ? "true" : "false" ));
    }
  else
    {
      g_free ((gpointer) result);
      return NULL;
    }
  
  return result;
}

static gboolean set_string (JNIEnv *env, jobject header, jfieldID fieldID,
                            const gchar *property)
{ 
  jstring property_string_field = NULL; 
  
  if (property == NULL || header == NULL)
    {
      return JNI_FALSE;
    }
  
  property_string_field = (*env)->NewStringUTF(env, property);
  if (property_string_field == NULL)
    {
      return JNI_FALSE;
    }
    
  (*env)->SetObjectField(env, header, fieldID, property_string_field);

  return JNI_TRUE;
}

static void element_added (GstBin *bin, GstElement *element, gpointer data)
{
  GstElementFactory *factory;
  
  factory = gst_element_get_factory (element);
  ((AudioProperties *) data)->name = gst_element_factory_get_longname (factory);
}

/* ***** END: PRIVATE FUNCTIONS IMPLEMENTATION ***** */
