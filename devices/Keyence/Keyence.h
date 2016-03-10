#ifndef __KEYENCE_H
#define __KEYENCE_H

#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <fstream>
#include <cmath>

#include "VKeyence.h"
#include "KeyenceComHandler.h"

class Keyence : public VKeyence
{
 public:

  Keyence( const ioport_t );
  ~Keyence();

 public:

  bool DeviceAvailable() const;

  bool GetStatus();
  int GetError();

  void Reset();
  void ConfirmErrorRectification();
  void ValidConfig();
  void ValidParameter();
  void ModeChange();
  void MeasurementValueOutput(int out, double value);
  void Timing(int out, int status);
  void AutoZero(int out, bool status);
  void Reset(int out);
  void PanelLock(int status);
  void ProgramChange(int prog_number);
  void ProgramCheck(int value);
  void StatResultOutput(int out, std::string value);
  void ClearStat(int out);
  void StartDataStorage();
  void StopDataStorage();
  void InitDataStorage();
  void OutputDataStorage(int out, std::vector<double> values);
  void DataStorageStatus(std::string value);
  void SetSamplingRate(int mode);
  void SetAveraging(int out, int mode);

  // low level methods                                                                                                                                                                                      
  void SendCommand(const std::string &);
  void ReceiveString(std::string &);

 private:

  void StripBuffer( char* ) const;
  void DeviceInit();

  KeyenceComHandler* comHandler_;
  bool isDeviceAvailable_;
};

#endif
