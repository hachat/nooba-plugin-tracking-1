#include "trackingplugin.h"
#include <QtCore>
#include <opencv2/core/core.hpp>

// opencv includes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <QDebug>

TrackingPlugin::TrackingPlugin()
{


}

TrackingPlugin::~TrackingPlugin()
{

}

bool TrackingPlugin::procFrame( const cv::Mat &in, cv::Mat &out, ProcParams &params )
{


    img_mask = in.clone();

    // bgs->process(...) method internally shows the foreground mask image
    bgs.process(in, img_mask);
    blobTracking.process(in, img_mask, img_blob);

    cv::cvtColor(img_blob, out, CV_BGR2GRAY);

    //cv::imshow("mask", img_mask);
    //cv::imshow("blob", img_blob);
    // Perform blob counting
    blobCounting.setInput(img_blob);
    blobCounting.setTracks(blobTracking.getTracks());
    blobCounting.process();

    return true;
}

bool TrackingPlugin::init()
{
    output_location = "/home/chathuranga/Programming/FYP/data/text/2013-10-13-blob_centroids.txt";
    createStringParam("output_location",output_location,false);
    blobTracking.setOutputFile(output_location);
    return true;
}

bool TrackingPlugin::release()
{
    return true;
}

PluginInfo TrackingPlugin::getPluginInfo() const
{
    PluginInfo pluginInfo(
        "Tracking Plugin",
        0,
        1,
        "Blob tracking and counting using cvBlob and BGS library",
        "Chathuranga Hettiarachchi");
    return pluginInfo;
}

void TrackingPlugin:: onStringParamChanged(const QString& varName, const QString& val){
    if(varName == "output_location"){
        setProperty("output_location",val);
        blobTracking.setOutputFile(output_location);
        debugMsg("output_location set to "  + val);
    }
}


// see qt4 documentation for details on the macro (Qt Assistant app)
// Mandatory  macro for plugins in qt4. Made obsolete in qt5
#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2(TrackingPlugin, TrackingPlugin);
#endif
