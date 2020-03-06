/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//               Copyright (C) 2011-2017 - The DESY CMS Group                  //
//                           All rights reserved                               //
//                                                                             //
//      The CMStkModLab source code is licensed under the GNU GPL v3.0.        //
//      You have the right to modify and/or redistribute this source code      //
//      under the terms specified in the license, which may be found online    //
//      at http://www.gnu.org/licenses or at License.txt.                      //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

#ifndef ASSEMBLYMAINWINDOW_H
#define ASSEMBLYMAINWINDOW_H

#include <AssemblyVUEyeCamera.h>
#include <AssemblyUEyeCameraThread.h>
#ifdef NOUEYE
#include <AssemblyUEyeFakeModel.h>
typedef AssemblyUEyeFakeModel AssemblyUEyeModel_t;
#else
#include <AssemblyUEyeModel.h>
typedef AssemblyUEyeModel AssemblyUEyeModel_t;
#endif
#include <AssemblyUEyeWidget.h>
#include <AssemblyUEyeCameraWidget.h>
#include <AssemblyUEyeView.h>
#include <AssemblyUEyeTest.h>
#include <AssemblyUEyeSnapShooter.h>
#include <AssemblyZFocusFinder.h>
#include <AssemblyImageController.h>
#include <AssemblyImageView.h>
#include <AssemblyThresholder.h>
#include <AssemblyThresholderView.h>
#include <AssemblyObjectFinderPatRec.h>
#include <AssemblyObjectFinderPatRecView.h>
#include <AssemblyObjectFinderPatRecThread.h>
#include <AssemblyObjectAligner.h>
#include <AssemblyObjectAlignerView.h>
#include <AssemblyAssembly.h>
#include <AssemblyAssemblyView.h>
#include <AssemblyMultiPickupTester.h>
#include <AssemblyToolboxView.h>
#include <AssemblySmartMotionManager.h>
#include <AssemblyParameters.h>
#include <AssemblyParametersView.h>
#include <AssemblyHardwareControlView.h>
#include <LStepExpressModel.h>
#include <LStepExpressMotionManager.h>
#include <LStepExpressMotionView.h>
#include <LStepExpressMotionThread.h>
#include <LStepExpressSettings.h>
#include <LStepExpressSettingsWidget.h>
#include <LStepExpressWidget.h>
#include <LStepExpressJoystickWidget.h>
#include <LStepExpressMeasurement.h>
#include <LStepExpressMeasurementWidget.h>
#include <LStepExpressPositionWidget.h>
#include <LStepExpressStatusWindow.h>
// #include <ConradModel.h>   // CONRAD
// #include <ConradManager.h> // CONRAD
#include <VellemanModel.h>    // VELLEMAN
#include <VellemanManager.h>  // VELLEMAN

#include <QMainWindow>
#include <QString>
#include <QTabWidget>
#include <QDir>
#include <QTimer>
#include <QToolBar>
#include <QScrollArea>

class AssemblyMainWindow : public QMainWindow
{
 Q_OBJECT

 public:

  explicit AssemblyMainWindow(const QString& outputdir_path, const QString& logfile_path, const unsigned int camera_ID=1, QWidget* parent=nullptr);
  virtual ~AssemblyMainWindow() {}

 public slots:

  void  enable_images();
  void disable_images();
  void     get_image ();

  void    connect_images();
  void disconnect_images();

  void changeState_autofocus(const int);

  void start_objectAligner(const AssemblyObjectAligner::Configuration&);
  void disconnect_objectAligner();

  void start_multiPickupTest(const AssemblyMultiPickupTester::Configuration&);
  void disconnect_multiPickupTest();

  void testTimer();

  void quit_thread(QThread*, const QString&) const;
  void quit();

 signals:

  void images_ON();
  void images_OFF();

  void image_request();

  void updateVacuumChannelsStatus();

  void autofocus_ON();
  void autofocus_OFF();

  void objectAligner_connected();
  void objectAligner_disconnected();

  void multiPickupTest_disconnected();

 protected slots:

  void liveUpdate();

 protected:

  // Low-Level Controllers (Motion, Camera, Vacuum)
  //  ConradModel*   conradModel_;   // CONRAD
  //  ConradManager* conradManager_; // CONRAD
  VellemanModel*   vellemanModel_;   // VELLEMAN
  VellemanManager* vellemanManager_; // VELLEMAN

  LStepExpressModel*          motion_model_;
  LStepExpressMotionManager*  motion_manager_;
  LStepExpressMotionView*     motion_manager_view_;
  LStepExpressMotionThread*   motion_thread_;
  LStepExpressSettings*       motionSettings_;
  LStepExpressSettingsWidget* motionSettingsWidget_;

  AssemblyUEyeModel_t*      camera_model_;
  AssemblyUEyeCameraThread* camera_thread_;
  AssemblyUEyeCameraWidget*      camera_widget_;
  AssemblyUEyeTest*        camera_test_;
  AssemblyVUEyeCamera*      camera_;
  unsigned int              camera_ID_;

  // High-Level Controllers
  AssemblyImageController*    image_ctr_;
  AssemblyZFocusFinder*       zfocus_finder_;
  AssemblyThresholder*        thresholder_;
  AssemblyObjectAligner*      aligner_;
  AssemblyAssembly*           assembly_;
  AssemblyMultiPickupTester*  multipickup_tester_;

  AssemblyObjectFinderPatRec*       finder_;
  AssemblyObjectFinderPatRecThread* finder_thread_;

  AssemblySmartMotionManager* smart_motion_;

  AssemblyParameters* params_;

  // Views
  QToolBar*   toolBar_;
  QTabWidget* tabWidget_;

//  AssemblyUEyeSnapShooter* finderView_;
//  AssemblyUEyeSnapShooter* edgeView_;
//  AssemblyUEyeSnapShooter* rawView_;
  AssemblyImageView* image_view_;
  AssemblyThresholderView* thresholder_view_;
  AssemblyObjectFinderPatRecView* finder_view_;
  AssemblyObjectAlignerView* aligner_view_;
  AssemblyAssemblyView* assembly_view_;
  AssemblyToolboxView* toolbox_view_;
  AssemblyParametersView* params_view_;
  AssemblyHardwareControlView* hwctr_view_;

  QCheckBox* autofocus_checkbox_;

  // flags
  bool images_enabled_;
  bool aligner_connected_;

  // timing
  double testTimerCount_;
  QTimer* liveTimer_;
};

#endif // ASSEMBLYMAINWINDOW_H
