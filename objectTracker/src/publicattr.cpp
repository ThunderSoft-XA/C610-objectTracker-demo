# include "publicattr.h"


namespace c610gst
{
gboolean GstAttr::setPipelineState(GstElement* _pipeline, GstState _state)
{
    if (GstStateChangeReturn::GST_STATE_CHANGE_FAILURE == gst_element_set_state (_pipeline, _state)) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(_pipeline);
        return false;
    }
    return true;
}

/* called when a new message is posted on the bus */
static void cb_message (GstBus *bus, GstMessage *message, gpointer    user_data)
{
    GstElement *pipeline = GST_ELEMENT (user_data);
    
    switch (GST_MESSAGE_TYPE (message)) {
        case GST_MESSAGE_ERROR:
        case GST_MESSAGE_EOS:
            g_print ("we reached EOS/error\n");
            gst_element_set_state(pipeline,GST_STATE_NULL);
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
            break;
    }
}

void GstAttr::DisposeMessage(GstElement *_pipeline)
 {
    GstBus* gstbus = gst_element_get_bus(_pipeline);
    gst_bus_add_signal_watch(gstbus);
    g_signal_connect (gstbus, "message", (GCallback) cb_message,_pipeline);
 }

void GstAttr::Deinit()
{
    setPipelineState(this->pipeline,GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));
}
    
} // namespace c610gst
