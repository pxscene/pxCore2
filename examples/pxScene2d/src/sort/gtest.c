#include <gst/gst.h>
#include <inttypes.h>

static GMainLoop *loop;

#define MAX_PIPELINE 10
static GstElement *play[MAX_PIPELINE];
int numPlay = 0;


static void element_added(GstBin* bin, GstElement* element, gpointer user_data)
{
  printf("@@@@@@@@@@@ element added: %s\n", GST_OBJECT_NAME(element));
}

static void cb_typefound(GstElement* typefind, guint probability,
                         GstCaps *caps, gpointer data)
{
  printf("!!!!!!!!!!!!!!!3###In cb_typefound\n");
  gchar *type;

  type = gst_caps_to_string (caps);
  g_print ("Media type %s found, probability %d%%\n", type, probability);
  g_free (type);

}


static void
print_one_tag (const GstTagList * list, const gchar * tag, gpointer user_data)
{
  int i, num;

  num = gst_tag_list_get_tag_size (list, tag);
  for (i = 0; i < num; ++i) {
    const GValue *val;

    /* Note: when looking for specific tags, use the gst_tag_list_get_xyz() API,
     * we only use the GValue approach here because it is more generic */
    val = gst_tag_list_get_value_index (list, tag, i);
    if (G_VALUE_HOLDS_STRING (val)) {
      g_print ("\t%20s : %s\n", tag, g_value_get_string (val));
    } else if (G_VALUE_HOLDS_UINT (val)) {
      g_print ("\t%20s : %u\n", tag, g_value_get_uint (val));
    } else if (G_VALUE_HOLDS_DOUBLE (val)) {
      g_print ("\t%20s : %g\n", tag, g_value_get_double (val));
    } else if (G_VALUE_HOLDS_BOOLEAN (val)) {
      g_print ("\t%20s : %s\n", tag,
          (g_value_get_boolean (val)) ? "true" : "false");
    } else if (GST_VALUE_HOLDS_BUFFER (val)) {
#if 0
      GstBuffer *buf = gst_value_get_buffer (val);
      guint buffer_size = gst_buffer_get_size (buf);

      g_print ("\t%20s : buffer of size %u\n", tag, buffer_size);
#else
      g_print("unhandled buffer\n");
#endif
    } else if (GST_VALUE_HOLDS_DATE_TIME (val)) {
#if 0
      GstDateTime *dt = g_value_get_boxed (val);
      gchar *dt_str = gst_date_time_to_iso8601_string (dt);

      g_print ("\t%20s : %s\n", tag, dt_str);
      g_free (dt_str);
#else
      g_print("unhandled date time\n");
#endif
    } else {
      g_print ("\t%20s : tag of type '%s'\n", tag, G_VALUE_TYPE_NAME (val));
    }
  }
}

double volume = 0;
double delta = 0.05;

static gboolean
my_bus_callback (GstBus     *bus,
		 GstMessage *message,
		 gpointer    data)
{

  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_TAG:
  {
    GstTagList *tags = NULL;
    g_print("Got tags from element %s\n",
            GST_OBJECT_NAME(message->src));
    gst_message_parse_tag(message, &tags);
    gst_tag_list_foreach(tags, print_one_tag, NULL);
    // 1.x
    //gst_tag_list_unref(tags);
    // 0.1
    gst_tag_list_free(tags);
  }
  break;
  case GST_MESSAGE_STATE_CHANGED:
  {
    GstState old_state;
    GstState new_state;
    gst_message_parse_state_changed(message, &old_state, &new_state, NULL);
    g_print("Element %s changed state from %s to %s.\n",
            GST_OBJECT_NAME(message->src),
            gst_element_state_get_name(old_state),
            gst_element_state_get_name(new_state));

    if (!strcmp(GST_OBJECT_NAME(message->src),"typefind") && new_state == GST_STATE_PAUSED)
    {
      printf("************ Here\n");
      g_signal_connect(message->src, "have-type", G_CALLBACK(cb_typefound), NULL);
    }
  }
  break;
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

//      g_main_loop_quit (loop);
      break;
    }
  case GST_MESSAGE_BUFFERING:
  {
    gint percent;
    gst_message_parse_buffering(message, &percent);
    g_print("Buffering in %s element: %d%%\n", GST_OBJECT_NAME(message->src), percent);
  }
  break;
  case GST_MESSAGE_EOS:
    /* end-of-stream */
//      g_main_loop_quit (loop);
    g_print("End of Stream\n");
    
    // restart pipeline
#if 1
    printf("Setting pipeline state to null\n");
    gst_element_set_state ((GstElement*)data, GST_STATE_NULL);
    
    volume = volume + delta;
    
    if (volume > 1.0)
    {
      volume = 1.0;
      delta = -0.01;
    }
    else if (volume < 0)
    {
      volume = 0;
      delta = 0.01;
    }

    g_object_set((GstElement*)data, "volume", volume, NULL);
    
    
      printf("Setting pipeline state to playing\n");
      gst_element_set_state ((GstElement*)data, GST_STATE_PLAYING);
#endif
      
      break;
  case GST_MESSAGE_DURATION:
  {
    GstFormat format;
    gint64 duration;
    gst_message_parse_duration(message, &format, &duration);
    g_print("Duration from (%s) element: %s, %"PRId64"ns or %fs\n",
            GST_OBJECT_NAME(message->src), gst_format_get_name(format),
            duration, (double)duration/1000000000.0);
  }
  break;
    default:
      /* unhandled message */
      g_print ("???? Got unhandled %s message\n", GST_MESSAGE_TYPE_NAME (message));
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}


gboolean timeout(gpointer user_data)
{
  printf("************in timer\n");
  
  int i = 0;
  for(i = 0; i < numPlay; i++)
  {
    // gst_element_set_state (play[i], GST_STATE_NULL);
  }
  return TRUE;
}


gint
main (gint   argc,
      gchar *argv[])
{
//  GMainLoop *loop;
//  GstElement *play;
  GstBus *bus;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* make sure we have a URI */
  if (argc < 2) {
    g_print ("Usage: %s <URI>\n", argv[0]);
    return -1;
  }

  numPlay = argc-1;

  /* set up */

  int i;

  for (i = 0; i < argc-1; i++)
  {
    play[i] = gst_element_factory_make ("playbin2", "play");
    g_object_set (G_OBJECT (play[i]), "uri", argv[i+1], NULL);

    printf("playing url %s\n", argv[i+1]);
    
#if 1
    bus = gst_pipeline_get_bus (GST_PIPELINE (play[i]));
    gst_bus_add_watch (bus, my_bus_callback, play[i]);
    gst_object_unref (bus);
#endif


    g_signal_connect(play[i], "element-added", G_CALLBACK(element_added), NULL);
    
    gst_element_set_state (play[i], GST_STATE_PLAYING);

  }

  g_timeout_add(5000, timeout, NULL);

  /* now run */
  g_main_loop_run (loop);

  /* also clean up */
  gst_element_set_state (play[i], GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (play[i]));

  return 0;
}
    
