/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//                                                                             //
//                                                                             //
//                   Written by Elise Hinkle (Brown CMS Group)                 //
//                  Modeled on cmstkmodlab/common/ConradModel.h                //
//                          Last Updated August 23, 2019                       //
//                                                                             //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#ifndef VELLEMANMODEL_H
#define VELLEMANMODEL_H

#include <string>

#include <QString>

#include "DeviceState.h"

#ifdef USE_FAKEIO
#include "devices/Velleman/VellemanControllerFake.h"
typedef VellemanControllerFake VellemanController_t;
#else
#include "devices/Velleman/VellemanController.h"
typedef VellemanController VellemanController_t;
#endif // ends #ifdef USE_FAKEIO

/*** 
   Original DESY code written by Jan Olzem and Andreas Mussgiller.
   Invasive refactor by Sander Vanheule. Velleman iteration adapted 
   by Elise Hinkle.
***/

/***
    \brief Model for controlling the Velleman relay card.

    This model can be used to set and query the status of the
    relays. The response of the device will not be instantaneous
    and, as such, device control should be handled in a separate
    thread. Device states INITIALIZING and CLOSING signify that no
    operations on that device should be attempted.

    Currently, any status-changing command that does not comply 
    with the ongoing operation will be ignored (i.e. disable relay
    while relay state is INITIALIZING). 

    Here, "device" primarily refers to the relay card as a whole, 
    while "relay" primarily refers to the relay controlling a 
    particular vacuum channel.
***/

class VellemanModel : public QObject, public AbstractDeviceModel<VellemanController_t>
{
  Q_OBJECT

public:

  explicit VellemanModel(const std::string& port, QObject* parent=nullptr);
  virtual ~VellemanModel();

  // Methods for power control and querying statuses of devices connected to relay
  const State& getRelayState(int device) const;

public slots:
  // Methods for control and querying statuses of device itself (as specified
  // by abstract parent class)
  void setDeviceEnabled(bool enabled);
  void enableRelay(int relay);
  void disableRelay(int relay);
  void setControlsEnabled(bool enabled);
  void setRelayEnabled(int relay, bool enabled);

protected:

  const QString port_;

  void initialize();
  void close();

  // Last known communication state of device
  void setDeviceState(State state);

  std::vector<State> relayStates_;
  void setRelayState(int relay, State state);

private:

  void setDeviceFullOff();
  void setAllRelaysSame(const std::vector<bool>& status);
  void setRelayEnabledRaw(int relay, bool enabled);
  
signals:
  // Classname identifiers needed because Qt can't resolve internal enums
  // NOTE: is this happening here though? 
  void deviceStateChanged(State newState);
  void relayStateChanged(int relay, State newState);
  void controlStateChanged(bool);
  
};

#endif // ends #ifndef VELLEMANMODEL_H
