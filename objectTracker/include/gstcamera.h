#ifndef __GST_CAMERA_H__
#define __GST_CAMERA_H__

#include <iostream>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <gst/app/app.h>
#include <cairo/cairo.h>
#include "publicattr.h"

#define DEFAULT_WIDTH 227
#define DEFAULT_HEIGHT 227
#define DEFAULT_CHANNEL 3
#define DEFAULT_FORMAT "NV12"
#define DEFAULT_FRAMERATE 30
#define DEFAULT_DECODE "h264"
#define DEFAULT_PATH "file://data/${pwd}/test.mp4"

namespace c610gst
{
#if 0
/**
 * @brief this class is concrete implementation
 * gstreamer`s modifiable parameters was configured by these bool flags,
 * the pipeline of default parameters will been execute if don`t configure
 */
class GstCamera
{
private:
    /* data */
    bool store_video;
    bool store_img;

    void LaunchExecute(GstType pip_type);
    void FactoryExecute(GstType pip_type);

public:
    EXPORT_API GstCamera();
    EXPORT_API GstCamera(GstAttr _gst_attr, BuildType _build_type, GstType _gst_type, CameraDest _camera_dest);
    ~GstCamera();
    EXPORT_API void BuildPipline(GstType _pip_type);
    EXPORT_API void InitPipeline();

};
#endif

/**
 * @brief using gst_launch_parsh() function to parse command line for execute gst
 * usually,advantage in some short gst command
 */
class CommandGst : public GstAttr,public VideoAttr
{
private:
    /* data */
    bool defaultbuild;
    bool hwdec;

    void DescribePipline();
   std::ostringstream RtspPipeline(bool _hwdec);
   std::ostringstream LocalPipeline(bool _hwdec);
   std::ostringstream FilePipeline(bool _hwdec);
   std::ostringstream PipelineDest(std::string _pipeline_str,CameraDest _camera_dest);

public:
    /**
     * @brief none parameters construct function build default pipeline
     */
    CommandGst(/* args */);
    /**
     * @brief Construct a new Construct Gst object,the pipeline will been build according to the order of GstElementPack
     * 
     * @param _gst_set a vector of record GstElements
     */
    CommandGst(std::vector<GstElementPack *> _gst_set,VideoAttr _video_attr);
    ~CommandGst();


    void DescribePipline(GstType _gst_type, CameraDest _camera_dest);
    void BuildPipline(GstType _gst_type, CameraDest _camera_dest);

    void set_hwdec(bool _hwdec) {
        this->hwdec = _hwdec;
    }
};

/**
 * @brief this class have to use gst_pipeline_new() function for build a pipeline
 * usually,advantage in flexible
 */
class ConstructGst : public GstAttr,public VideoAttr
{
private:
    /* data */
    bool defaultbuild;

public:
    /**
     * @brief none parameters construct function build default pipeline
     */
    ConstructGst(/* args */);
    /**
     * @brief Construct a new Construct Gst object,the pipeline will been build according to the order of GstElementPack
     * 
     * @param _gst_set a vector of record GstElements
     */
    ConstructGst(std::vector<GstElementPack *> _gst_set,std::vector<VideoAttr *>  _capsprop_vec);
    ~ConstructGst();

    void DescribePipline();
    Index_t BuildElements(std::vector<GstElementPack *>  _ele_set);
    gboolean setElementProperty(GstElement* _pipeline,std::vector<GstElementPack *> _ele_set);
    Index_t ElementIsEmpty( std::vector<GstElementPack *>  _ele_pack);
    gboolean setCapsInfo(std::vector<GstCaps *> _caps_set, std::vector<VideoAttr *>  _capsprop_vec);
    gboolean AttachElement(GstElement *_pipeline, std::vector<GstElementPack *>  _ele_pack);
    GstFlowReturn GetAFrameByAppsink(GstAppSink *_appsink, cv::Mat &_dest);
    void GetAFrameByPad(GstElement *_ele, cv::Mat &_dest);
    gboolean StoreAImgFromGst(GstElement *_element, char *_img_path);

};

} // namespace c610gst


#endif // !__GST_CAMERA_H__