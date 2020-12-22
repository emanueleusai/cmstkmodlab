/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2020 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#ifndef _VAgilentTwisTorr304_H_
#define _VAgilentTwisTorr304_H_

#include <string>

/** @addtogroup devices
 *  @{
 */

/** @addtogroup Nanotec
 *  @{
 */

typedef const char* ioport_t;

class VAgilentTwisTorr304
{
 public:

  enum CommandCode {
    STX_    = 0x02,
    ETX_    = 0x03,
    RD_     = 0x30,
    WR_     = 0x31,
    OFF_    = 0x30,
    ON_     = 0x31
  };

  enum StatusCode {
    Stop             = 0,
    WaitingInterlock = 1,
    Starting         = 2,
    AutoTuning       = 3,
    Braking          = 4,
    Normal           = 5,
    Fail             = 6
  };

  enum ErrorBit {
    NO_CONNECTION       = 0x01,
    PUMP_OVERTEMP       = 0x02,
    CONTROLLER_OVERTEMP = 0x04,
    POWER_FAIL          = 0x08,
    AUX_FAIL            = 0x10,
    OVERVOLTAGE         = 0x20,
    SHORT_CIRCUIT       = 0x40,
    TOO_HIGH_LOAD       = 0x80
  };

  VAgilentTwisTorr304(ioport_t);
  virtual ~VAgilentTwisTorr304() {}

  const std::string GetPumpStatusText(StatusCode status) const;
  const std::string GetPumpStatusText() const;
  virtual StatusCode GetPumpStatus() const = 0;
  virtual unsigned int GetErrorCode() const = 0;

  virtual bool GetPumpState() const = 0;
  virtual void SwitchPumpOn() = 0;
  virtual void SwitchPumpOff() = 0;

 protected:

  void MakeReadCommand(std::string& command, unsigned int window) const;
  void MakeWriteCommand(std::string& command, unsigned int window, bool data) const;
  void MakeWriteCommand(std::string& command, unsigned int window, int data) const;
  void MakeWriteCommand(std::string& command, unsigned int window, float data) const;
  void MakeWriteCommand(std::string& command, unsigned int window, std::string& data) const;

  bool GetBooleanValue(std::string& reply) const;
  int GetIntegerValue(std::string& reply) const;
  float GetFloatValue(std::string& reply) const;

  void GetChecksum(const std::string& buffer, char& crc0, char& crc1) const;

  void GetCommandAsHex(std::string& hexcommand, const std::string& command) const;
};

/** @} */

/** @} */

#endif // _VAgilentTwisTorr304_H_
