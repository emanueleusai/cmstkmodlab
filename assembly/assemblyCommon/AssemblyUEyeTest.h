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

#ifndef ASSEMBLYUEYETEST_H
#define ASSEMBLYUEYETEST_H

#include <QTabWidget>

#include "AssemblyVUEyeCamera.h"
#include <AssemblyUEyeView.h>
#include <QScrollArea>
#include <QPushButton>
#include <opencv2/opencv.hpp>

class AssemblyUEyeTest : public QTabWidget
{
    Q_OBJECT
public:
  explicit AssemblyUEyeTest(AssemblyVUEyeCamera *camera,
                              QWidget *parent = 0);

  void    connectImageProducer_image    (const QObject* sender, const char* signal);
  void disconnectImageProducer_image    (const QObject* sender, const char* signal);

protected:

  AssemblyVUEyeCamera* camera_;

  AssemblyUEyeView* img_ueye_;
  QScrollArea*      img_scroll_;
  QPushButton*      img_button_;

public slots:

};

#endif // ASSEMBLYUEYETEST_H
