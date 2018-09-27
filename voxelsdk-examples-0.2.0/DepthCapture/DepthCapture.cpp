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

enum Options
{
  VENDOR_ID = 0,
  PRODUCT_ID = 1,
  SERIAL_NUMBER = 2,
  DUMP_FILE = 3,
  NUM_OF_FRAMES = 4,
  CAPTURE_TYPE = 5
};


int main(int argc, char *argv[])
{  
  logger.setDefaultLogLevel(LOG_INFO);
    
  Vector<uint16_t> pids;
  String serialNumber;

  String type = "raw";
  
  int32_t frameCount = 1;


  uint16_t vid = (uint16_t)0x0451;
  pids.push_back((uint16_t)0x9105);
  String dumpFileName = "img";
  type = "depth";
  
  std::ofstream f(dumpFileName, std::ios::binary | std::ios::out);
  
  if(!f.good())
  {
    std::cerr << "Failed to open '" << dumpFileName << "'" << std::endl;
    return -1;
  }
  
  CameraSystem sys;
  
  // Get all valid detected devices
  const Vector<DevicePtr> &devices = sys.scan();
  
  DevicePtr toConnect;
  
  std::cout << "Detected devices: " << std::endl;
  for(auto &d: devices)
  {
    std::cout << d->id() << std::endl;
    
    if(d->interfaceID() == Device::USB)
    {
      USBDevice &usb = (USBDevice &)*d;
      
      if(usb.vendorID() == vid && (serialNumber.size() == 0 || usb.serialNumber() == serialNumber))
      {
        for(auto pid: pids)
          if(usb.productID() == pid)
            toConnect = d;
      }
    }
  }
  
  if(!toConnect)
  {
    std::cerr << "No valid device found for the specified VID:PID:serialnumber" << std::endl;
    return -1;
  }
    
  DepthCameraPtr depthCamera = sys.connect(toConnect);
  
  if(!depthCamera)
  {
    std::cerr << "Could not load depth camera for device " << toConnect->id() << std::endl;
    return -1;
  }

  if(!depthCamera->isInitialized())
  {
    std::cerr << "Depth camera not initialized for device " << toConnect->id() << std::endl;
    return -1;
  }
  
  std::cout << "Successfully loaded depth camera for device " << toConnect->id() << std::endl;
  
  int count = 0;
  
  TimeStampType lastTimeStamp = 0;

  if (type == "raw")
  {
    depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_UNPROCESSED, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
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

      if (count >= frameCount)
        dc.stop();
    });
  } 
  else if (type == "raw_processed")
  {
    depthCamera->registerCallback(DepthCamera::FRAME_RAW_FRAME_PROCESSED, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
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

      if (count >= frameCount)
        dc.stop();
    });
  }
  else if (type == "depth")
  {
    depthCamera->registerCallback(DepthCamera::FRAME_DEPTH_FRAME, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
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

      if (count >= frameCount)
        dc.stop();
    });
  }
  else if (type == "pointcloud")
  {
    depthCamera->registerCallback(DepthCamera::FRAME_XYZI_POINT_CLOUD_FRAME, [&](DepthCamera &dc, const Frame &frame, DepthCamera::FrameType c) {
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

      if (count >= frameCount)
        dc.stop();
    });
  }

  if(depthCamera->start())
  {
    FrameRate r;
    if(depthCamera->getFrameRate(r))
      logger(LOG_INFO) << "Capturing at a frame rate of " << r.getFrameRate() << " fps" << std::endl;
    depthCamera->wait();
  }
  else
    std::cerr << "Could not start the depth camera " << depthCamera->id() << std::endl;

  return 0;
}
