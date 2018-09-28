/*
 * TI Voxel SDK example.
 *
 * Copyright (c) 2014 Texas Instruments Inc.
 */


#include "CameraSystem.h"
#include "Common.h"
#include "UVCStreamer.h"
#include <iomanip>
#include <fstream>

using namespace Voxel;

class Camera
{
public:
    int init();
    int get_img();

private:
    uint16_t m_pid;
    String m_type;
    int32_t m_frameCount;
    uint16_t m_vid;
    String m_dumpFileName;
    DepthCameraPtr m_depthCamera;
    CameraSystem m_sys;
    DevicePtr m_toConnect;
};


int Camera::init()
{
    logger.setDefaultLogLevel(LOG_INFO);
    m_type = "depth";
    m_frameCount = 10;
    m_vid = (uint16_t)0x0451;
    m_pid = (uint16_t)0x9105;
    m_dumpFileName = "img";
    String serialNumber;


    // Get all valid detected devices
    const Vector<DevicePtr> &devices = m_sys.scan();


    std::cout << "Detected devices: " << std::endl;
    for(auto &d: devices)
    {
      std::cout << d->id() << std::endl;

      if(d->interfaceID() == Device::USB)
      {
        USBDevice &usb = (USBDevice &)*d;

        if(usb.vendorID() == m_vid && (serialNumber.size() == 0 || usb.serialNumber() == serialNumber))
        {
            if(usb.productID() == m_pid)
              m_toConnect = d;
        }
      }
    }
    if(!m_toConnect)
    {
      std::cerr << "No valid device found for the specified VID:PID:serialnumber" << std::endl;
      return -1;
    }

    m_depthCamera = m_sys.connect(m_toConnect);

    if(!m_depthCamera)
    {
      std::cerr << "Could not load depth camera for device " << m_toConnect->id() << std::endl;
      return -1;
    }

    if(!m_depthCamera->isInitialized())
    {
      std::cerr << "Depth camera not initialized for device " << m_toConnect->id() << std::endl;
      return -1;
    }

    std::cout << "Successfully loaded depth camera for device " << m_toConnect->id() << std::endl;
    return 0;

}

int Camera::get_img()
{

    int count = 0;

    TimeStampType lastTimeStamp = 0;

    std::ofstream f(m_dumpFileName, std::ios::binary | std::ios::out);


    if(!f.good())
    {
      std::cerr << "Failed to open '" << m_dumpFileName << "'" << std::endl;
      return -1;
    }

    if (m_type == "raw")
    {
      m_depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_UNPROCESSED, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
        const RawDataFrame *d = dynamic_cast<const RawDataFrame *>(&frame);

        if (!d)
        {
          std::cout << "Null frame captured? or not of type RawDataFrame" << std::endl;
          return;
        }

        std::cout << "Capture frame " << d->id << "@" << d->timestamp;

        if (lastTimeStamp != 0)
          std::cout << " (" << 1E6 / (d->timestamp - lastTimeStamp) << " fps)";

        std::cout << std::endl;

        f.write((char *)&d->id, sizeof(d->id));
        f.write((char *)&d->timestamp, sizeof(d->timestamp));

        lastTimeStamp = d->timestamp;

        f.write((char *)d->data.data(), d->data.size());

        count++;

        if (count >= m_frameCount)
          dc.stop();
      });
    }
    else if (m_type == "raw_processed")
    {
      m_depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_PROCESSED, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
        const ToFRawFrame *d = dynamic_cast<const ToFRawFrame *>(&frame);

        if (!d)
        {
          std::cout << "Null frame captured? or not of type ToFRawFrame" << std::endl;
          return;
        }

        std::cout << "Capture frame " << d->id << "@" << d->timestamp;

        if (lastTimeStamp != 0)
          std::cout << " (" << 1E6 / (d->timestamp - lastTimeStamp) << " fps)";

        std::cout << std::endl;

        f.write((char *)&d->id, sizeof(d->id));
        f.write((char *)&d->timestamp, sizeof(d->timestamp));

        lastTimeStamp = d->timestamp;

        if (d->phase())
          f.write((char *)d->phase(), d->phaseWordWidth()*d->size.width*d->size.height);

        if (d->amplitude())
          f.write((char *)d->amplitude(), d->amplitudeWordWidth()*d->size.width*d->size.height);

        if (d->ambient())
          f.write((char *)d->ambient(), d->ambientWordWidth()*d->size.width*d->size.height);

        if (d->flags())
          f.write((char *)d->flags(), d->flagsWordWidth()*d->size.width*d->size.height);

        count++;

        if (count >= m_frameCount)
          dc.stop();
      });
    }
    else if (m_type == "depth")
    {
        m_depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
        const DepthFrame *d = dynamic_cast<const DepthFrame *>(&frame);
        if (!d)
        {
          std::cout << "Null frame captured? or not of type DepthFrame" << std::endl;
          return;
        }

        std::cout << "Capture frame " << d->id << "@" << d->timestamp;

        if (lastTimeStamp != 0)
          std::cout << " (" << 1E6 / (d->timestamp - lastTimeStamp) << " fps)";

        std::cout << std::endl;

        f.write((char *)&d->id, sizeof(d->id));
        f.write((char *)&d->timestamp, sizeof(d->timestamp));

        lastTimeStamp = d->timestamp;

        f.write((char *)d->depth.data(), sizeof(float)*d->size.width*d->size.height);
        f.write((char *)d->amplitude.data(), sizeof(float)*d->size.width*d->size.height);

        count++;

        if (count >= m_frameCount)
          dc.stop();
      });
    }
    else if (m_type == "pointcloud")
    {
      m_depthCamera->registerCallback(DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
        const XYZIPointCloudFrame *d = dynamic_cast<const XYZIPointCloudFrame *>(&frame);

        if (!d)
        {
          std::cout << "Null frame captured? or not of type XYZIPointCloudFrame" << std::endl;
          return;
        }

        std::cout << "Capture frame " << d->id << "@" << d->timestamp;

        if (lastTimeStamp != 0)
          std::cout << " (" << 1E6 / (d->timestamp - lastTimeStamp) << " fps)";

        std::cout << std::endl;

        f.write((char *)&d->id, sizeof(d->id));
        f.write((char *)&d->timestamp, sizeof(d->timestamp));

        lastTimeStamp = d->timestamp;

        f.write((char *)d->points.data(), sizeof(IntensityPoint)*d->points.size());

        count++;

        if (count >= m_frameCount)
          dc.stop();
      });
    }

    if(m_depthCamera->start())
    {
      FrameRate r;
      if(m_depthCamera->getFrameRate(r))
        logger(LOG_INFO) << "Capturing at a frame rate of " << r.getFrameRate() << " fps" << std::endl;
      m_depthCamera->wait();
    }
    else
      std::cerr << "Could not start the depth camera " << m_depthCamera->id() << std::endl;
    return 0;

}

int main(int argc, char *argv[])
{  
  Camera camera;
  camera.init();
  camera.get_img();
  return 0;
}
