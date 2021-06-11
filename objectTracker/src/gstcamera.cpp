#include "gstcamera.h"
#include "libyuv/convert.h"
#include "libyuv/planar_functions.h"
#include "libyuv/scale_argb.h"

using namespace libyuv;

namespace c610gst{

#if 0
GstCamera::GstCamera(/*all default parms*/)
{

}

GstCamera::GstCamera(GstAttr _gst_attr, BuildType _build_type, GstType _gst_type, CameraDest _camera_dest)
{
    if(BuildType::FACTORY_BUILD ==  _build_type) {
        FactoryExecute(_gst_type);
    } else if(BuildType::QUICK_BUILD == _build_type) {
        LaunchExecute(_gst_type);
    }
}

void GstCamera::LaunchExecute(GstType _gst_type)
{

}

void GstCamera::FactoryExecute(GstType pip_type)
{

}

void GstCamera::BuildPipline(GstType pip_type) 
{
    std::ostringstream def_pipline;

}
void GstCamera::InitPipeline()
{
    GstAttr::GstInit();

}

GstCamera::~GstCamera()
{
}
#endif


CommandGst::CommandGst() 
{
    this->defaultbuild = true;
    this->set_width(DEFAULT_WIDTH);
    this->set_height(DEFAULT_HEIGHT);
    this->set_framerate(DEFAULT_FRAMERATE);
    this->set_channels(DEFAULT_CHANNEL);
    this->set_path(DEFAULT_PATH);
    this->set_decode_type(DEFAULT_DECODE);

}

CommandGst::CommandGst(std::vector<GstElementPack *> _gst_set,VideoAttr _video_attr)
{
    this->defaultbuild = false;
}

void CommandGst::DescribePipline()
{
    std::ostringstream _pipeline_str;
    for (auto ele : this->get_gstelement_vec()) {
        if(0 == ele->index) {
            _pipeline_str << ele->name;
            if (NULL != ele->alias) {
                _pipeline_str << " name=" << ele->alias << " ";
            } else {
                _pipeline_str << " ";
            }
            if (NULL != ele->property) {
                int i = 0;
                for (; i < ele->pro_num; i ++) {
                        _pipeline_str << ele->property[2 * i] << "=" << ele->property[2 * i + 1] << ",";
                }
                _pipeline_str << " ";
            }
            _pipeline_str << "! ";
        } else {
            _pipeline_str <<"video/x-" << this->get_capsprop_vec()[ele->index]->get_decode_type() << ",";
            _pipeline_str << "format=" <<  this->get_capsprop_vec()[ele->index]->get_format() << ",";
            _pipeline_str << "width=" <<  this->get_capsprop_vec()[ele->index]->get_width() << ",";
            _pipeline_str << "height=" <<  this->get_capsprop_vec()[ele->index]->get_height() << ",";
            _pipeline_str << "framerate" <<  this->get_capsprop_vec()[ele->index]->get_framerate() << " ";
        }
        _pipeline_str << "! ";
    }
    this->pipeline_name = (char *)_pipeline_str.str().substr(0, _pipeline_str.str().size() - 2).c_str();
}

std::ostringstream CommandGst::RtspPipeline(bool _hwdec) 
{
    std::ostringstream _pipeline_str;
    if(true == _hwdec) {
        _pipeline_str << "rtspsrc location=" << this->get_path() << " latency=0 tcp-timeout=500 drop-on-latency=true ntp-sync=true" << " ! ";
        _pipeline_str << "queue ! rtp" << this->get_decode_type() << "depay ! "<< this->get_decode_type() << "parse ! queue ! omx" << this->get_decode_type() << "dec " << " ! ";
        _pipeline_str << "videoscale ! video/x-raw,width=640,height=360 ! ";
    } else {
        _pipeline_str << "rtspsrc location=" << this->get_path() << " latency=0 tcp-timeout=500 drop-on-latency=true ntp-sync=true" << " ! ";
        _pipeline_str << "queue ! rtp" << this->get_decode_type() << "depay ! "<< this->get_decode_type() << "parse ! queue ! avdec_" << this->get_decode_type() << " ! ";
        _pipeline_str << "videoscale ! video/x-raw,width=640,height=360 ! ";
    }

    return _pipeline_str;
}

std::ostringstream CommandGst::LocalPipeline(bool _hwdec)
{
    std::ostringstream _pipeline_str;
    if(true == _hwdec) {
        _pipeline_str << "qtiqmmfsrc name=qmmf ! video/x-" << this->get_decode_type() << ",format=" << this->get_format() << ",width=" << this->get_width() << ",height=" << this->get_height() <<",framerate=" << this->get_framerate() <<"/1";
        _pipeline_str << "! "<< this->get_decode_type() << "parse config-interval=1 ! mpegtsmux name=muxer ! queue ! omx" << this->get_decode_type() << "dec " << " ! ";
    } else {
        _pipeline_str << "qtiqmmfsrc name=qmmf ! video/x-" << this->get_decode_type() << ",format=" << this->get_format() << ",width=" << this->get_width() << ",height=" << this->get_height() <<",framerate=" << this->get_framerate() <<"/1";
        _pipeline_str << "! "<< this->get_decode_type() << "parse config-interval=1 ! mpegtsmux name=muxer ! queue ! avdec_" << this->get_decode_type() << " ! ";
    }

    return _pipeline_str;
}

std::ostringstream CommandGst::FilePipeline(bool _hwdec)
{
    std::ostringstream _pipeline_str;
    if (true ==  _hwdec) {
        _pipeline_str << "filesrc location=" << this->get_path() << " " << " ! ";
        _pipeline_str << "qtdemux name=demux demux.  ! queue ! h264parse ! qtivdec !" ;
        _pipeline_str << " videoscale ! video/x-raw,width=640,height=360  ! ";
    } else {
        _pipeline_str << "filesrc location=" << this->get_path() << " " << " ! ";
        _pipeline_str << "qtdemux name=demux demux. ! queue ! h264parse ! avdec_" << this->get_decode_type() << " ! ";
        _pipeline_str << " videoscale ! video/x-raw,width=640,height=360  ! ";
    }

    return _pipeline_str;
}

std::ostringstream CommandGst::PipelineDest(std::string _pipeline,CameraDest _camera_dest)
{
    std::ostringstream _pipeline_str;
    _pipeline_str << _pipeline;
    switch(_camera_dest) {
        case CameraDest::LOCAL_FILE :
            _pipeline_str << "filesink location=" << "\/" << "data" << "\/" << "default.mp4";
            break;
        case CameraDest::TCP :
            _pipeline_str << "tcpserversink port=8900 host=127.0.0.1";
            break;
        case CameraDest::UDP :
            _pipeline_str << "udpsink host=IP port=8554";
            break;
        case CameraDest::SCREEN :
            break;
        default :
            _pipeline_str << "appsink name=usrappsink sync=false  max-lateness=0 max-buffers=1 drop=true";
            break;
    }

    return _pipeline_str;
}

 void CommandGst::DescribePipline(GstType _gst_type, CameraDest _camera_dest)
 {
    std::ostringstream _pipeline_str;
    if(GstType::RTSP == _gst_type) {
        _pipeline_str = this->RtspPipeline(this->hwdec);
    } else if (GstType::LOCAL_CAMERA == _gst_type) {
        _pipeline_str = this->LocalPipeline(this->hwdec);
    } else {
        _pipeline_str = this->FilePipeline(this->hwdec);
    }
    _pipeline_str = this->PipelineDest(_pipeline_str.str(),_camera_dest);
    this->pipeline_name = (char *) _pipeline_str.str().c_str();
 }


/**
 * @brief 
 *  
 * @param _gst_type 
 * @param _camera_dest 
 */
 void CommandGst::BuildPipline(GstType _gst_type, CameraDest _camera_dest)
 {
    if(true == this->defaultbuild) {
        this->DescribePipline(_gst_type, _camera_dest);
    } else {
        this->DescribePipline();
    }
 }

// class construct was specify code part ,as follow://
ConstructGst::ConstructGst()
{

}

ConstructGst::ConstructGst(std::vector<GstElementPack *> _ele_set,std::vector<VideoAttr *> _capsprop_vec)
{
    this->CameraEnvInit();
    this->GstInit();
    this->set_gstelement_vec(_ele_set);
    this->set_capsprop_vec(_capsprop_vec);

    assert(0 == BuildElements(this->get_gstelement_vec()));
    assert(true == setElementProperty(this->pipeline,this->get_gstelement_vec()));
    assert(true == this->setCapsInfo(this->get_gstcaps_vec(),this->get_capsprop_vec()));
    assert(true == AttachElement(this->pipeline,this->get_gstelement_vec()));
}

/**
  * @brief  thinking ......
  * 
  * @param _gst_type 
  * @param _camera_dest 
  */
void ConstructGst::DescribePipline()
{

}

Index_t ConstructGst::BuildElements(std::vector<GstElementPack *> _ele_set)
{
    Index_t index = 0;
    for(auto _ele : _ele_set) {
        _ele->gst_element = gst_element_factory_make(_ele->name->c_str(),_ele->alias->c_str());
        if(NULL == _ele->gst_element) {
            std::cout << __FILE__ << ++index << "th element build failed" << __LINE__ << std::endl;
            return index;
        }
    }
    return index;
}

gboolean ConstructGst::setElementProperty(GstElement* _pipeline,std::vector<GstElementPack *> _ele_set)
{
    for (auto _ele : _ele_set) {
        if(NULL != _ele->property) {
            int total = _ele->pro_num;
            for(int i = 0; i < total; i ++) {
                g_object_set(G_OBJECT(_pipeline), _ele->property[2 * i], _ele->property[2 * i + 1], NULL);
            }
        }
    }
}

/**
 * @brief  for set caps info , in other words, set filter property
 * 
 * @param _capsprop_vec 
 * @return gboolean   false is failed
 */
gboolean ConstructGst::setCapsInfo(std::vector<GstCaps *>_caps_set, std::vector<VideoAttr *> _capsprop_vec)
{
    for (auto _capsprop : _capsprop_vec) {
        GstCaps * _caps;
        _caps = gst_caps_new_simple(string("video/x-").append(_capsprop->get_format()).c_str(),
        "format", G_TYPE_STRING, _capsprop->get_format().c_str(),
#ifdef FORMATED
        "framerate", GST_TYPE_FRACTION, _capsprop->get_framerate(), 1,
#endif
        "width", G_TYPE_INT,_capsprop->get_width(),
        "height", G_TYPE_INT,_capsprop->get_height(),
        NULL);
        _caps_set.push_back(_caps);
    }
    return true;
}

gboolean ConstructGst::AttachElement(GstElement *_pipeline, std::vector<GstElementPack *> _ele_pack)
{
    Index_t error_id = -1;
    if(0 != (error_id = ElementIsEmpty(_ele_pack))) {
        std::cout << __FILE__ << error_id << "th element build failed !" << __LINE__ << std::endl;
        return false;
    }
    int ele_count = _ele_pack.size();
    for(int i = 0; i < ele_count; i++)
    {
        gst_bin_add_many(GST_BIN(_pipeline),_ele_pack[i]->gst_element);
    }
    for(int i = 0; i < ele_count + 1; i++)
    {
        if(!gst_element_link_many(_ele_pack[i]->gst_element,_ele_pack[i+1]->gst_element)) {
            g_printerr ("Elements could not be linked.\n");
            gst_object_unref (_pipeline);
            return false;
        }
    }
    return true;
}

/**
 * @brief  for assert element != NULL
 * 
 * @param _ele_set  
 * @return Index_t  0 is succesfully, otherwise, return the position of error element
 */
Index_t ConstructGst::ElementIsEmpty( std::vector<GstElementPack *> _ele_set)
{
    int position = 0;
    for(auto ele : _ele_set) {
        position++;
        if(NULL == ele->gst_element) {
            return position;
        } 
    }
    return 0;
}

static GstFlowReturn on_new_sample (GstAppSink *_appsink, cv::Mat &_dest)
{
    GstSample *sample = gst_app_sink_pull_sample(_appsink);
    if(sample == NULL){
      if(gst_app_sink_is_eos(_appsink))
        return GST_FLOW_EOS;
    }
#ifdef DEBUG
    cout << __FILE__ << __LINE__ << endl;
#endif // DEBUG
    GstCaps *caps = gst_sample_get_caps(sample);
    gint width,height;

    GstStructure *structure = gst_caps_get_structure(caps,0);
    gst_structure_get_int(structure,"width",&width);
    gst_structure_get_int(structure,"height",&height);

    if( width && height) {
        // Convert gstreamer data to OpenCV Mat
        GstMapInfo map_info;
        GstBuffer *buffer = gst_sample_get_buffer(sample);
        gst_buffer_map(buffer,&map_info,GST_MAP_READ);

        // still exixst some risk , maybe need corvert style ex.NV12 to RGBA,or other
        unsigned char *ybase = (unsigned char *)map_info.data;
        unsigned char *vubase = &map_info.data[width * height];
        unsigned char rgb24[width*height*3];
        libyuv::NV12ToRGB24(ybase, width, vubase, width, rgb24, width*3, width, height);
        cv::Mat tmpmat(height, width, CV_8UC3, (unsigned char *)rgb24, cv::Mat::AUTO_STEP);
        _dest = tmpmat.clone();
        gst_buffer_unmap(buffer, &map_info);
        return GST_FLOW_OK;
    } else {
        return GST_FLOW_EOS;
    }
}
/**
 * @brief  simply convert GstMap to cv::Mat by cv::Mat frame(cv::Size(width, height), CV_8UC3, (char *)map.data, (width * 3) + 2);
 * 
 * @param _appsink 
 * @param _dest 
 * @return GstFlowReturn 
 */
GstFlowReturn ConstructGst::GetAFrameByAppsink(GstAppSink *_appsink, cv::Mat &_dest)
{
    g_signal_connect(_appsink,"new-sample",G_CALLBACK(on_new_sample),&_dest);
}

 
static GstPadProbeReturn cb_have_data(GstPad *_pad, GstPadProbeInfo *_pad_info, gpointer user_data)
{
    GstMapInfo map_info;
    GstBuffer *buffer;
    char *ptr;

    GstCaps *_pad_caps;
    _pad_caps = gst_pad_get_current_caps(_pad);
    gint width,height;

    GstStructure *structure = gst_caps_get_structure(_pad_caps,0);
    gst_structure_get_int(structure,"width",&width);
    gst_structure_get_int(structure,"height",&height);

    buffer = GST_PAD_PROBE_INFO_BUFFER (_pad_info);
    buffer = gst_buffer_make_writable (buffer);

      /* Making a buffer writable can fail (for example if it
   * cannot be copied and is used more than once)
   */
    if (buffer == NULL)
        return GST_PAD_PROBE_OK;
    /* Mapping a buffer can fail (non-writable) */
    if (gst_buffer_map (buffer, &map_info, GST_MAP_WRITE)) {
        ptr = (char *) map_info.data;
        /* invert data */
        {
            // in fact , the problem of convert type will occur, so, have to convert the frame type
            cv::Mat frame(cv::Size(width, height), CV_8UC3, (char *)map_info.data, (width * 3) + 2);
            user_data = frame.clone().data;
        }
        // unmapping
        gst_buffer_unmap (buffer, &map_info);
    }
    
    GST_PAD_PROBE_INFO_DATA (_pad_info) = buffer;
    
    return GST_PAD_PROBE_OK;
}

void ConstructGst::GetAFrameByPad(GstElement *_ele, cv::Mat &_dest_mat)
{
    GstPad *_pad;
    _pad = gst_element_get_static_pad (_ele, "src");
    gst_pad_add_probe (_pad, GST_PAD_PROBE_TYPE_BUFFER,(GstPadProbeCallback) cb_have_data,&_dest_mat, NULL);
    gst_object_unref (_pad);
    
}

gboolean ConstructGst::StoreAImgFromGst(GstElement *_element, char *_img_path)
{

}


};