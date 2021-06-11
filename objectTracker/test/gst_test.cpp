#include <iostream>
#include <mcheck.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <thread>
#include "objecttracker.h"
#include "libyuv/convert.h"
#include "libyuv/planar_functions.h"
#include "libyuv/scale_argb.h"

#include "../utils/configenv.hpp"
#include <gst/gst.h>
#include <gst/app/app.h>

using namespace objecttracker;

#if 0
int main_gst (int aigc, char **argv)
{

    std::vector<GstElementPack *> *gst_set = new (std::vector<GstElementPack *>){
        &(GstElementPack ){NULL,"qtiqmmfsrc","gstsource",3,
            {"af-mode","1","sharpness","4","noise-reduction","2"},0},
        &(GstElementPack ){NULL,"capsfilter","gstsrcfilter",0,NULL,1},
        &(GstElementPack ){NULL,"h264parse","gst264parse",1,(gchar *[]){ "config-interval","-1"},0},
        &(GstElementPack ){NULL,"queue","gstqueue",1,(gchar *[]){"max-size-bytes","42949670"},0},
        &(GstElementPack ){NULL,"qtidec","gstqtivdec",0,NULL,0},
        &(GstElementPack ){NULL,"qtivtransform","gsttrans",1,(gchar *[]){"rotate","1"},0},
        &(GstElementPack ){NULL,"capsfilter","gsttransfilter",0,NULL,2},
        &(GstElementPack ){NULL,"appsink","gstappsink",0,NULL,0}};




    std::vector<VideoAttr *> caps_vec(2);
    VideoAttr *caps_1 = new VideoAttr();
    caps_1->set_decode_type("h264");
    caps_1->set_width(227);
    caps_1->set_height(227);
    caps_1->set_format("NV12");
    caps_1->set_framerate(30);

    VideoAttr *caps_2 = new VideoAttr();
    caps_2->set_decode_type("raw");
    caps_2->set_width(227);
    caps_2->set_height(227);
    caps_2->set_format("BGRA");

    caps_vec.push_back(caps_1);
    caps_vec.push_back(caps_2);

    ConstructGst *my_gst = new ConstructGst(*gst_set,caps_vec);
    ObjectTracker *my_tracker = new ObjectTracker();

    my_tracker->init(runtime_t::GPU);
    std::vector<cv::Mat> mat_vec(2);
    std::vector<int> result_vec;
    // get target object`s Mat
    cv::Mat target_mat = cv::imread(TARGET_PATH,cv::ImreadModes::IMREAD_COLOR);
    cv::Mat next_frame_mat = cv::Mat();
    do {
        // get video frame 
        my_gst->GetAFrameByPad(my_gst->get_gstelement_vec()[0]->gst_element,next_frame_mat);

        // get a vetor of mat from object `s    mat and video frame
        mat_vec.push_back(target_mat);
        mat_vec.push_back(next_frame_mat);
    
        // get a rect point result by exexcutesnpe network
        result_vec = my_tracker->doDetect(mat_vec);
        //get video next frame and refill vetor
        target_mat =next_frame_mat(cv::Rect(result_vec[0],result_vec[1],result_vec[2],result_vec[3]));

    } while(1);  //EOS for exit

    
    return 0;
}
#endif 

#define PICTURE_WIDTH 640
#define PICTURE_HEIGHT 360
#define FRAME_PER_SECOND 30
#define FORMAT_TYPE "NV12"

#define TARGET_PATH "/data/c610gst/object.png"
#define TEST_VIDEO "/data/c610gst/out.mp4"

typedef struct _TrackData {
    ObjectTracker *tracker;
    cv::Mat obj_mat;
    cv::Mat next_mat;
} TrackData;

static const struct option long_option[]={
   {"object",optional_argument,NULL,'i'},
   {"video",optional_argument,NULL,'v'},
   {"model",optional_argument,NULL,'m'},
   {NULL,0,NULL,0}
};

int parm_parse(int argc,char *argv[])
{
    int opt=0;
    while((opt = getopt_long(argc,argv,"i:l",long_option,NULL)) != -1) {
        switch(opt) {
            case 'i':break;
            case 'v':
                printf("name:%s ", optarg);
                break;
            case 'm': 
                break;
        }
    }
}
#ifdef SHOW_VIDEO
void preview(std::shared_ptr<cv::Mat> imgframe)
{
  cv::Mat showframe;
#if 1 
        cv::resize(*imgframe, showframe, cv::Size(1920,1080), 0, 0, cv::INTER_LINEAR);
        cv::imshow("sink", showframe);
        cv::waitKey(1);
        //usleep(30*1000);
#endif
		return ;
}

static void thread_show(void)
{
    //int fh = openFB("/dev/fb0");
    double elapsed_time = 0.0;
    struct timeval nframerate_time_end, time_start;
	int m_nDetectFramecount=0;
    gettimeofday(&time_start, nullptr);
    while(true)
    {
        std::shared_ptr<cv::Mat> imgframe;
        int num_w = sqrt(1);
        num_w = num_w + (pow(num_w, 2) < 1 ? 1 : 0);
        int num_h =1 / num_w + (1%num_w > 0 ? 1 :0);
        int width = 640;
        int height = 360;
        int left = 0,top = 0;
        //TIC;

        imgframe = rgb_frame_cache->fetch();
        if(imgframe == NULL || imgframe.get() == NULL)
        {
            usleep(40*1000);
            continue;
        }
    if(imgframe != NULL && imgframe.get() != NULL)
                preview(imgframe);
#ifdef DEBUG_INFO
        usleep(SHOW_SLEEP_TIME*1000);
		m_nDetectFramecount++; 
    gettimeofday(&nframerate_time_end, nullptr);
    elapsed_time = (nframerate_time_end.tv_sec - time_start.tv_sec) * 1000 +
                                (nframerate_time_end.tv_usec - time_start.tv_usec) / 1000;
    if(elapsed_time > 1000*10)
    {
      printf("[show] showframerate=%f\n", GetLocalTimeWithMs().c_str(),
                            m_nDetectFramecount*1000/elapsed_time);
	  memcpy(&time_start,&nframerate_time_end,sizeof(struct timeval));
      m_nDetectFramecount = 0;
    }
#endif
    }
}
#endif // SHOW_VIDEO

static GstFlowReturn on_new_sample (GstAppSink *_appsink, TrackData *_user_data)
{
    GstSample *sample = gst_app_sink_pull_sample(_appsink);
    if(sample == NULL){
      if(gst_app_sink_is_eos(_appsink))
        return GST_FLOW_EOS;
    }
    cout << __FILE__ << "==new sample=="<< __LINE__ << endl;
    GstCaps *caps = gst_sample_get_caps(sample);
    gint width,height;

    GstStructure *structure = gst_caps_get_structure(caps,0);
    gst_structure_get_int(structure,"width",&width);
    gst_structure_get_int(structure,"height",&height);

    if( width && height) {
        cout <<"==mat width="<< width <<",mat height = " << height <<  endl;
        // Convert gstreamer data to OpenCV Mat
        GstMapInfo map_info;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        gst_buffer_map(buffer,&map_info,GST_MAP_READ);

        // still exixst some risk , maybe need corvert style ex.NV12 to RGBA,or other
        unsigned char *ybase = (unsigned char *)map_info.data;
        unsigned char *vubase = &map_info.data[width * height];
        unsigned char rgb24[width*height*3];
        libyuv::NV12ToRGB24(ybase, width, vubase, width, rgb24, width*3, width, height);
        std::cout << __FILE__ << "==finished nv12 convert to rgb24==" << __LINE__ << std::endl;
        // get video frame 
        _user_data->next_mat = cv::Mat(height, width,  CV_8UC3, (unsigned char *)rgb24, cv::Mat::AUTO_STEP);
        cv::imwrite("/data/img/temp.jpeg",_user_data->next_mat );
#ifdef SHOW_VIDEO
        std::shared_ptr<cv::Mat> imgframe;
        imgframe = std::make_shared<cv::Mat>(_user_data->next_mat);
        rgb_frame_cache->feed(imgframe);
#endif // SHOW_VIDEO
        std::cout << __FILE__ << "==get next video frame==" << __LINE__ << std::endl;
        std::vector<cv::Mat> mat_vec(2);
        std::vector<int> result_vec;
        assert(_user_data->obj_mat.empty() != true);
        assert(_user_data->next_mat.empty() != true);
        // get a vetor of mat from object `s    mat and video frame
        mat_vec[0] = _user_data->obj_mat;
        mat_vec[1] = _user_data->next_mat;

        // get a rect point result by exexcutesnpe network
        std::cout << __FILE__ << "==entering doDetect==" << __LINE__ << std::endl;
        result_vec = _user_data->tracker->doDetect(mat_vec);
        for(int i; i < result_vec.size(); i++ ) {
            if(result_vec[i] < 0) {
                result_vec[i] = 0;
            }
            cout << result_vec[i]  << "=====" ;
        }
    
        //get video next frame and refill vetor
        _user_data->obj_mat = _user_data->next_mat(cv::Rect(result_vec[0],result_vec[1],result_vec[2],result_vec[3]));

        cv::Mat display_mat = _user_data->next_mat;
        cv::rectangle(display_mat,cvPoint(result_vec[0],result_vec[1]),cvPoint(result_vec[2],result_vec[3]),cv::Scalar(255,0,0),1,1,0);
#if 0
        char str_path[32];
        sprintf(str_path,"/data/img/%4d.jpeg",num);
        cv::imwrite(str_path,display_mat);
#endif
        gst_buffer_unmap(buffer, &map_info);
        gst_sample_unref(sample);
        return GST_FLOW_OK;
    }
    cout << __FILE__ << "==haven`t next mat=="<< __LINE__ << endl;
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
}

/* called when a new message is posted on the bus */
static void cb_message (GstBus *bus, GstMessage *message, gpointer    user_data)
{
    GstElement *pipeline = GST_ELEMENT (user_data);

     cout << __FILE__ << "==message callback=="<< __LINE__ << endl;
    switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_ERROR:
        case GST_MESSAGE_EOS:
            g_print ("we reached EOS/error\n");
            gst_element_set_state(pipeline,GST_STATE_NULL);
            exit(0);
            break;
        case GST_MESSAGE_APPLICATION:
        {
            if (gst_message_has_name (message, "ExPrerolled")) {
                /* it's our message */
                g_print ("we are all prerolled, do seek\n");
                gst_element_seek (pipeline,
                    1.0, GST_FORMAT_TIME,
                    GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
                    GST_SEEK_TYPE_SET, 2 * GST_SECOND,
                    GST_SEEK_TYPE_SET, 5 * GST_SECOND);
                gst_element_set_state (pipeline, GST_STATE_PLAYING);
            }
            break;
        }
        default:
            cout << __FILE__ << "==get unknow message==" << GST_MESSAGE_TYPE (message) << __LINE__ << endl;
            break;
    }
}

static void on_pad_added(GstElement *element, GstPad *pad, gpointer data)
{
    GstPad *sinkpad;
    GstElement *decoder = (GstElement *) data;

     cout << __FILE__ << "==dynamic link=="<< __LINE__ << endl;
    /* We can now link this pad with the its corresponding sink pad */
    g_print("Dynamic pad created, linking demuxer/decoder\n");
    sinkpad = gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);
}


int main(int argc, char **argv)
{
    cout << __FILE__ << __LINE__ << endl;
    if(setCameraEnv()) {
        printf("camera env init failed\n");
        return -1;
    }
    system("source /etc/gstreamer1.0/set_gst_env.sh");

    setenv("MALLOC_TRACE", "mtrace.log", 1);
    mtrace();

    gst_init (&argc, &argv);

    GMainLoop *loop;
    GError *error;
    GstElement *pipeline,*appsink;
#if 1
    GstElement *source,*demuxer;
    GstElement *h264parse, *queue;
    GstElement *qtivdec,*trans,*videofilter ,*scalefilter;
    GstCaps *videocaps,*scalecaps;
#endif

    GstBus *gstbus;

    loop = g_main_loop_new(NULL, FALSE);
#if 1
    /* Create the empty pipeline */
    cout << __FILE__ << "==Create the empty pipeline=="<<__LINE__ << endl;
    pipeline = gst_pipeline_new ("file-pipeline");
    /* Create the elements */
    source = gst_element_factory_make("filesrc","localsrc");
    demuxer = gst_element_factory_make("qtdemux","demuxer");
    videofilter = gst_element_factory_make("capsfilter","vfilter");
    h264parse = gst_element_factory_make("h264parse","parse");
    queue = gst_element_factory_make("queue","queue");
    qtivdec  = gst_element_factory_make("qtivdec","dec");
    trans = gst_element_factory_make("qtivtransform","trans");
    scalefilter = gst_element_factory_make("capsfilter","scalefilter");
    appsink = gst_element_factory_make("appsink","appsink");

    /* Modify the source's properties */
    cout << __FILE__ << "==setup element`s properties==" << __LINE__ << endl;
    g_object_set(G_OBJECT(source), "location", TEST_VIDEO, NULL);

    videocaps = gst_caps_new_simple("video/x-h264",
        "framerate",GST_TYPE_FRACTION,FRAME_PER_SECOND,1,
        "width",G_TYPE_INT,PICTURE_WIDTH,
        "height",G_TYPE_INT,PICTURE_HEIGHT,
        NULL);

    scalecaps = gst_caps_new_simple("video/x-raw",
        "format",G_TYPE_STRING,FORMAT_TYPE,
        NULL);

    g_object_set(G_OBJECT(videofilter),"caps",videocaps,NULL);
    g_object_set(G_OBJECT(scalefilter), "caps", scalecaps, NULL);
    //sync=false  max-lateness=0 max-buffers=1 drop=true
    g_object_set(G_OBJECT(appsink),"sync",false,
        "max-lateness",0,
        "max-buffers",1,
        NULL);
    g_object_set(appsink,"emit-signals",TRUE,NULL);

    if (!pipeline || !source || !demuxer || !videofilter || !h264parse || !queue || !qtivdec || !trans || !scalefilter || !appsink) {
        g_printerr ("Not all elements could be created.\n");
        return false;
    }
#endif

#if 1
    gst_bin_add_many (GST_BIN (pipeline), source, demuxer,videofilter,h264parse,queue,qtivdec,trans,scalefilter,appsink, NULL);
    gst_element_link(source, demuxer);
    if (!gst_element_link_many(videofilter,h264parse,queue, qtivdec, trans,scalefilter,appsink,NULL)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return false;
    }
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), videofilter); // link dynamic pad

#endif

    /* link the pipeline */
#if 0
    gst_bin_add_many (GST_BIN (pipeline), source, muxer,h264parse,qtivdec,scalefilter,appsink, NULL);
    if (!gst_element_link_many(source, muxer,h264parse,qtivdec,scalefilter, appsink, NULL)) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return false;
    }
#endif
    cout << __FILE__ << "==build pipline successed==" << __LINE__ << endl;

#if 0
    string pipe = "filesrc location=/data/c610gst/out.mp4 ! qtdemux name=demux demux.video_0 ! avdec_h264 ! videoconvert ! videoscale ! video/x-raw,width=640,height=360 ! appsink name=sink";
    pipeline = gst_parse_launch(pipe.c_str(),&error);

    if (error != NULL) {
        g_print ("could not construct pipeline: %s\n", error->message);
        g_clear_error (&error);
        exit (-1);
    }

    /* get sink */
    cout << __FILE__ << "quick pipe successfully == " << __LINE__ << endl;
    appsink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");
    g_object_set(appsink,"emit-signals",TRUE,NULL);
#endif
    gst_element_set_state (pipeline, GST_STATE_PAUSED);

    TrackData *user_track_data;
    user_track_data->tracker = new ObjectTracker(runtime_t::GPU);
    user_track_data->obj_mat = cv::imread(TARGET_PATH,cv::ImreadModes::IMREAD_COLOR);
    cout << __FILE__ << "== repre Tracker finished==" << __LINE__ << endl;

    g_signal_connect(appsink,"new-sample",G_CALLBACK(on_new_sample),user_track_data);

    gstbus = gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch(gstbus);
    g_signal_connect (gstbus, "message", (GCallback) cb_message,pipeline);

    if ((gst_element_set_state(pipeline,GST_STATE_PLAYING)) == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }
    cout << __FILE__ << __LINE__ << endl;
#ifdef SHOW_VIDEO
        std::thread showThread(thread_show);
        showThread.join();
#endif // SHOW_VIDEO
    g_main_loop_run(loop);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref (gstbus);
    gst_object_unref(GST_OBJECT(pipeline));
    user_track_data->tracker->deInit();

    return 0;

}