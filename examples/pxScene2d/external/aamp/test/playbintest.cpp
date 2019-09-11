/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>


#define DEFAULT_URI "aamps://tungsten.aaplimg.com/VOD/bipbop_adv_example_v2/master.m3u8"
#define DEFAULT_URI2 "aamp://devimages.apple.com/iphone/samples/bipbop/bipbopall.m3u8"
#define DEFAULT_VIDEO_RECT "0,0,1280,720"

#define PLAYBINTEST_NUMBER_OF_VID 1
#define PLAYBINTEST_MAX_NUMBER_OF_VID 2

GST_DEBUG_CATEGORY_STATIC (playbintest);
#define GST_CAT_DEFAULT playbintest

GstElement *playbin[PLAYBINTEST_NUMBER_OF_VID];
GMainLoop *main_loop;
pthread_t uiThreadID;
const char* uri[2];
const char* playbin_names[PLAYBINTEST_MAX_NUMBER_OF_VID] = {"playbin-0", "playbin-1"};
const char* video_rectangle[PLAYBINTEST_MAX_NUMBER_OF_VID] = {"0,0,640,360","640,0,640,360"};
gboolean g_print_states = FALSE;

#define PLAYBINFLAG_BUFFERING 0x00000100

static void info(int idx = 0)
{
	gint64 position=0, duration=0;
	if (FALSE == gst_element_query_position(playbin[idx], GST_FORMAT_TIME, &position))
	{
		g_printerr("**PLAYBINTEST: gst_element_query_position failed\n");
	}

	if (FALSE == gst_element_query_duration(playbin[idx], GST_FORMAT_TIME, &duration))
	{
		g_printerr("*******PLAYBINTEST: ** gst_element_query_duration failed\n");
	}
	g_print("**PLAYBINTEST: Position:%" G_GINT64_FORMAT " Duration:%" G_GINT64_FORMAT "\n", position, duration);

}

static void setRate(gdouble rate, int idx=0)
{
	gint64 cur = GST_CLOCK_TIME_NONE;
	gboolean ret;
	if (!gst_element_query_position(playbin[idx], GST_FORMAT_TIME, &cur))
	{
		g_print("**PLAYBINTEST: query failed\n");
	}
	g_print("**PLAYBINTEST: setting rate %f\n", rate);
	if (rate < 0)
	{
		ret = gst_element_seek(playbin[idx], rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE, GST_SEEK_TYPE_SET, cur);
	}
	else
	{
		ret = gst_element_seek(playbin[idx], rate, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, cur,
				GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
	}
	if (!ret)
	{
		g_print("**PLAYBINTEST: seek failed\n");
	}
	else
	{
		g_print("**PLAYBINTEST: seek setrate success\n");
	}
}

static void seek(int seconds, int idx=0)
{
	gint64 cur = GST_SECOND*seconds;

	if (!gst_element_seek(playbin[idx], 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, cur,
			GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
	{
		g_print("**PLAYBINTEST: seek failed\n");
	}
	else
	{
		g_print("**PLAYBINTEST: seek success\n");
	}
}

static void setPause(bool pause, int idx= 0)
{

	GstStateChangeReturn ret;
	GstState state = pause?GST_STATE_PAUSED:GST_STATE_PLAYING;
	ret = gst_element_set_state(playbin[idx], state);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("gst_element_set_state failed\n");
	}
}

static void stop(int idx = 0)
{
	GstStateChangeReturn ret;
	ret = gst_element_set_state(playbin[idx], GST_STATE_NULL);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		g_printerr("gst_element_set_state failed\n");
	}
}

static void changeChannel(const char* uri, int idx= 0)
{
	GstStateChangeReturn ret;
	stop(idx);
	g_object_set(playbin[idx], "uri", uri, NULL);
	setPause(false);
}

static void stressTest(int idx= 0)
{
	int count = 0;
	g_print("**PLAYBINTEST: start stressTest\n");
	while (1)
	{
		changeChannel(uri[0], idx);
		g_usleep(10*1000*1000);
		changeChannel(uri[1], idx);
		g_usleep(10*1000*1000);
		count += 2;
		g_print("\n\n**PLAYBINTEST: ChannelChange Count %d\n\n",count);
	}
}

static void process_command(char* cmd)
{
	int value;
	if (strcmp(cmd, "info") == 0)
	{
		info();
	}
	else if (strcmp(cmd, "ff8") == 0)
	{
		setRate(8);
	}
	else if (strcmp(cmd, "ff16") == 0)
	{
		setRate(16);
	}
	else if (strcmp(cmd, "ff32") == 0)
	{
		setRate(32);
	}
	else if (strcmp(cmd, "ff") == 0)
	{
		setRate(4);
	}
	else if (strcmp(cmd, "rw32") == 0)
	{
		setRate(-32);
	}
	else if (strcmp(cmd, "rw16") == 0)
	{
		setRate(-16);
	}
	else if (strcmp(cmd, "rw8") == 0)
	{
		setRate(-8);
	}
	else if (strcmp(cmd, "rw") == 0)
	{
		setRate(-4);
	}
	else if (strcmp(cmd, "play") == 0)
	{
		setPause(false);
	}
	else if (strcmp(cmd, "pause") == 0)
	{
		setPause(true);
	}
	else if (strcmp(cmd, "1") == 0)
	{
		changeChannel(uri[1]);
	}
	else if (strcmp(cmd, "0") == 0)
	{
		changeChannel(uri[0]);
	}
	else if (strcmp(cmd, "stress") == 0)
	{
		stressTest();
	}
	else if (sscanf(cmd, "seek %d", &value) == 1)
	{
		seek(value);
	}
	else if (strcmp(cmd, "stop") == 0)
	{
		stop();
	}
}

static void* ui_thread(void * arg)
{
	char cmd[1024];
	g_print("**PLAYBINTEST: start ui_thread\n");
	while (fgets(cmd, sizeof(cmd), stdin))
	{
		char *dst = cmd;
		while (*dst >= ' ')
		{
			dst++;
		}
		*dst = 0x00; // NUL terminator
		process_command(cmd);
	}
	g_print("**PLAYBINTEST: exit ui_thread\n");
	return NULL;
}

static gboolean handle_bus_message(GstBus *bus, GstMessage *msg, void* arg)
{
	GError *error;
	gchar *info;
	//g_print("**PLAYBINTEST: Enter\n" );

	switch (GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_EOS:
		{
			g_print("EOS\n");
			//g_main_loop_quit(main_loop);
			break;
		}
		case GST_MESSAGE_ERROR:
		{
			gst_message_parse_error(msg, &error, &info);
			g_printerr("**PLAYBINTEST: Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), error->message);
			g_printerr("**PLAYBINTEST: Debugging information: %s\n", info ? info : "none");
			g_free(info);
			g_main_loop_quit(main_loop);
			break;
		}
		case GST_MESSAGE_STATE_CHANGED:
		{
			GstState old, now, pending;
			gst_message_parse_state_changed(msg, &old, &now, &pending);
			if (g_print_states || memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "playbin", 7) == 0)
			{
				g_print( "**PLAYBINTEST: element %s state change : %s -> %s . pending state %s\n", GST_ELEMENT_NAME(GST_MESSAGE_SRC(msg)),
						gst_element_state_get_name(old), gst_element_state_get_name(now), gst_element_state_get_name(pending) );
			}
			GST_DEBUG_BIN_TO_DOT_FILE((GstBin *)playbin[0], GST_DEBUG_GRAPH_SHOW_ALL, "playbintest");

			if (now == GST_STATE_PLAYING && memcmp(GST_OBJECT_NAME(GST_MESSAGE_SRC(msg)), "brcmvideosink", 13) == 0)
			{ // video scaling patch
				/*
				brcmvideosink doesn't sets the rectangle property correct by default
				gst-inspect-1.0 brcmvideosink
				g_object_get(_this->privateContext->pipeline, "video-sink", &videoSink, NULL); - reports NULL
				note: alternate "window-set" works as well
				*/
				GstElement *pbin = GST_ELEMENT(GST_MESSAGE_SRC(msg));
				while(GST_ELEMENT_PARENT(pbin))
				{
					pbin = GST_ELEMENT_PARENT(pbin);
				}
				const char * rect = DEFAULT_VIDEO_RECT;
				int zorder = 0;
				if (PLAYBINTEST_NUMBER_OF_VID > 1 )
				{
					for (int i=0; i< PLAYBINTEST_NUMBER_OF_VID; i++)
					{
						if (strcmp(GST_OBJECT_NAME(pbin), playbin_names[i]) == 0)
						{
							rect = video_rectangle[i];
							zorder = i;
							break;
						}
					}
				}
				g_print("*%s : setting rectangle to %s zorder to %d*\n", GST_OBJECT_NAME(pbin),  rect, zorder);
				g_object_set(GST_MESSAGE_SRC(msg), "rectangle", rect, NULL);
				g_object_set(GST_MESSAGE_SRC(msg), "zorder", zorder, NULL);
			}
			break;
		}
		case GST_MESSAGE_BUFFERING:
		{
			gint percent;
			gst_message_parse_buffering(msg, &percent);
			g_print("**PLAYBINTEST: eBuffering %d\n", percent);
#ifdef PLAYBINTEST_BUFFERING
			if (percent == 100 )
				setPause(false);
			else if(percent < 10)
				setPause(true);
#endif
			break;
		}
		case GST_MESSAGE_TAG:
			break;
		default:
			g_print("Bus msg type: %s\n", gst_message_type_get_name(msg->type));
			break;
	}
	//g_print("**PLAYBINTEST: Exit\n" );
	return TRUE;
}

int main(int argc, char *argv[])
{
	GstBus *bus;
	GstStateChangeReturn ret;
	gint flags;

	gst_init(&argc, &argv);
	GST_DEBUG_CATEGORY_INIT (playbintest, "playbin test", 0, "Debug Category for Playbin Test");

	GST_FIXME( "Start\n");

	if (argc > 1)
	{
		uri[0] = g_strdup(argv[1]);
		g_print("Using uri provided in command line : ");
	}
	else
	{
		uri[0] = DEFAULT_URI;
		g_print("Using default uri : ");
	}
	if (argc > 2)
	{
		uri[1] = g_strdup(argv[2]);
	}
	else
	{
		uri[1] = DEFAULT_URI2;
	}
	g_print("uri[0] %s uri[1] %s\n", uri[0], uri[1]);

	if (NULL != g_getenv("PLAYBINTEST_DEBUG_STATES"))
	{
		g_print_states = TRUE;
	}
	for(int i = 0; i <PLAYBINTEST_NUMBER_OF_VID; i++)
	{
		playbin[i] = gst_element_factory_make("playbin", playbin_names[i]);
		if (!playbin[i])
		{
			g_printerr("playbin couldn't be created.\n");
			return -1;
		}
		g_object_set(playbin[i], "uri", uri[i], NULL);

		g_object_get(playbin[i], "flags", &flags, NULL);
		flags |= 0x03 | 0x00000040;
#ifdef PLAYBINTEST_BUFFERING
		flags |= PLAYBINFLAG_BUFFERING;
		//g_object_set(playbin[i], "buffer-duration", 500*GST_MSECOND, NULL);
#endif
		g_object_set(playbin[i], "flags", flags, NULL);
//#define PLAYBINTEST_WESTEROSSINK
#ifdef PLAYBINTEST_WESTEROSSINK
		GstElement* west = gst_element_factory_make("westerossink", NULL);
		if (!west)
		{
			g_printerr("westeros-sink couldn't be created.\n");
			return -1;
		}
		g_object_set(playbin[i], "video-sink", west, NULL);
#endif

		bus = gst_element_get_bus(playbin[i]);
		gst_bus_add_watch(bus, (GstBusFunc) handle_bus_message, NULL);

#ifdef PLAYBINTEST_BUFFERING
		ret = gst_element_set_state(playbin[i], GST_STATE_PAUSED);
#else
		GST_FIXME( "Setting to Playing State\n");
		ret = gst_element_set_state(playbin[i], GST_STATE_PLAYING);
		GST_FIXME( "Set to Playing State\n");
#endif
		if (ret == GST_STATE_CHANGE_FAILURE)
		{
			g_printerr("Play failed\n");
			gst_object_unref(playbin[i]);
			return -1;
		}
	}
	pthread_create(&uiThreadID, NULL, &ui_thread, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(main_loop);

	/* Free resources */
	g_main_loop_unref(main_loop);
	gst_object_unref(bus);

	for(int i = 0; i <PLAYBINTEST_NUMBER_OF_VID; i++)
	{
		gst_element_set_state(playbin[i], GST_STATE_NULL);
		gst_object_unref(playbin[i]);
	}
	return 0;
}


