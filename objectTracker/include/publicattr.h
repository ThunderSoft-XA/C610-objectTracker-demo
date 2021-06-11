#ifndef __PUBLIC_ATTR_H__
#define __PUBLIC_ATTR_H__

#include <vector>
#include <string>
#include <gst/gst.h>
#include "../utils/configenv.hpp"

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

namespace c610gst
{
using Index_t = unsigned short;

// Maybe ,I should must using C++`s class to C++ Interface, or not C//
typedef struct _GstElementPack {
    GstElement *gst_element;
    std::string *name;
    std::string *alias;
    //std::vector<char *> property;
    int pro_num;
    const gchar **property;
    /**
     * @brief this index_t to show for caps, 
     * 0 ,no caps; number > 0, the position of  caps in cpas vector
     */
    Index_t index;
} GstElementPack;

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief support factory build or quick build for execute
 * gst_element_factory_make() for factory build 
 * gst_parse_launch() for quick build
 */
typedef enum _BuildType {
    QUICK_BUILD  = 0,
    FACTORY_BUILD
} BuildType;

/**
 * @brief  the local camera for C610 camera on board,gstreamer plugins is qti series
 *                 the rtsp for uri stream
 */
typedef enum _GstType {
    LOCAL_CAMERA = 0,
    RTSP
}GstType;

/**
 * @brief the pipeline`s last output to local file or tcp client or udp client
 *LOCAL_FILE will use filesink or multifilesink
 * TCP will use tcpsink,needing support for tool same like vlc etc...
 * UDP will udp sink
 * SCREEN will directly draw to screen by wayland and gtk
 */
typedef enum _CameraDest {
    LOCAL_FILE = 0,
    TCP,
    UDP,
    SCREEN
} CameraDest;

#ifdef __cplusplus
}
#endif

class FrameAttr
{
private:
    /**
     * @brief These data describe a frame info, some member varitiant isn`t necessary
     * 
     */
    int width;
    int height;
    int channels;
    string format;
    unsigned long index;
    std::string path;

public:
    FrameAttr(/* args */){ }
    ~FrameAttr(){ }

    void set_width(int _frame_w) { 
        this->width = _frame_w;
    } 
    int get_width() {
        return this->width;
    }

    void set_height(int _frame_h) {
        this->height = _frame_h;
    }
    int get_height() {
        return height;
    }

    void set_channels(int _frame_c) {
        this->height = _frame_c;
    }
    int get_channels() {
        return this->channels;
    }

    void set_format(string _frame_format)
    {
        #define FORMATED 0
        this->format = _frame_format;
    }
    string get_format()
    {
        return this->format;
    }

    void set_index(unsigned long _frame_index) {
        this->index = _frame_index;
    }
    unsigned long get_index() {
        return this->index;
    }

    void set_path(std::string _frame_path) {
        this->path = _frame_path;
    }
    std::string get_path() {
        return this->path;
    }
};

/**
 * @brief some key info only video
 * 
 */
class VideoAttr : public FrameAttr
{
private:
    /* data */
    string decode_type;
    int framerate;
public:
    VideoAttr(/* args */){ }
    ~VideoAttr(){ }

    void set_decode_type(string _video_d)
    {
        this->decode_type = _video_d;
    }
    string get_decode_type()
    {
        return this->decode_type;
    }

    void set_framerate(int _video_f) {
        this->framerate = _video_f;
    }
    int get_framerate() {
        return this->framerate;
    }

};

/**
 * @brief Don`t allow modify the base class to abstract class for 
 * Simplify incoming/input parameters for subsequent classes
 */
class GstAttr
{
private:
    /**
     * @brief why all is vector ? my idea is that I only fill a vector for get pipeline  
     *    GstElement  is packed by the GstElementPack struct for easily build
     */
    std::vector<GstElementPack *> gstelement_vec;
    std::vector<GstBus> gstbus_vec;
    std::vector<GstCaps *> gstcaps_vec;
    std::vector<GError> gsterror_vec;
    std::vector<GstMessage> gstmessage_vec;
    std::vector<VideoAttr *> capsprop_vec;
    
public:
    GstAttr(/* args */){ }
    ~GstAttr(){ }

    GstElement* pipeline;
    char *pipeline_name;

    static void CameraEnvInit() { 
        setCameraEnv(); 
    }
    static void GstInit() { 
        gst_init(NULL,NULL); 
    }
    virtual void DescribePipline();
    gboolean setPipelineState(GstElement* _pipeline, GstState _state);
    void DisposeMessage(GstElement *_pipeline);
    void Deinit();

    /**
     * @brief following function for set/get private data object
     */
    void set_gstelement_vec(std::vector<GstElementPack *>  _gst_p) {
        this->gstelement_vec = _gst_p;
    }
    std::vector<GstElementPack *> get_gstelement_vec() {
        return this->gstelement_vec;
    }

    void set_capsprop_vec(std::vector<VideoAttr *> _capsprop_vec) {
        this->capsprop_vec = _capsprop_vec;
    }
    std::vector<VideoAttr *> get_capsprop_vec() {
        return this->capsprop_vec;
    }

    void set_gstbus_vec(std::vector<GstBus> _gst_b) {
        this->gstbus_vec = _gst_b;
    }
    std::vector<GstBus> get_gstbus_vec() {
        return this->gstbus_vec;
    }
    
    void set_gstcaps_vec(std::vector<GstCaps *>  _gst_c) {
        this->gstcaps_vec = _gst_c;
    }
    std::vector<GstCaps *> get_gstcaps_vec() {
        return this->gstcaps_vec;
    }

    void set_gsterror_vec(std::vector<GError> _gst_e) {
        this->gsterror_vec = _gst_e;
    }
    std::vector<GError> get_gsterror_vec() {
        return this->gsterror_vec;
    }

    void set_gstmessage_vec(std::vector<GstMessage> _gst_m) {
        this->gstmessage_vec = _gst_m;
    }
    std::vector<GstMessage> get_gstmessage_vec() {
        return this->gstmessage_vec;
    }
};

#if 0
 /// Base definition for GstElemnet or GstBus etc for factory build...
template <typename T>
struct Gst_T {
    Gst_T() {}
    Gst_T(T* _element,gchar* _name,Order_t _order = 0) {
        this->element = _element;
        this->name = _name;
        this->order = _order;
    }
    T* element;
    /**
     * @brief most of elements don`t need name
     * @type why is gchar * ? avoid meaningless convert
     */
    gchar* name;
    Order_t  order;
};
using GstElementInfo = Gst_T<GstElement>;
using  GstBusInfo = Gst_T<GstBus>;
using GstCapsInfo = Gst_T<GstCaps>;

#endif

} // namespace c610gst

#endif // !__PUBLIC_ATTR_H__