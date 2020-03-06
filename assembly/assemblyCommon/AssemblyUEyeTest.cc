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

#include <nqlogger.h>

#include "AssemblyUEyeTest.h"
#include <QGridLayout>

AssemblyUEyeTest::AssemblyUEyeTest(AssemblyVUEyeCamera *camera,
                                       QWidget *parent)
    : QTabWidget(parent),
      img_ueye_(nullptr),
      img_scroll_(nullptr),
      camera_(camera)
{
	QPalette palette;
  	palette.setColor(QPalette::Background, QColor(220, 220, 220));

	QGridLayout* g0 = new QGridLayout;
  	this->setLayout(g0);

	img_ueye_ = new AssemblyUEyeView(this);
  	img_ueye_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  	img_ueye_->setMinimumSize(200, 200);
        img_ueye_->setMaximumSize(200, 200);
  	img_ueye_->setPalette(palette);
  	img_ueye_->setBackgroundRole(QPalette::Background);
  	img_ueye_->setScaledContents(true);
  	img_ueye_->setAlignment(Qt::AlignTop);

  	img_scroll_ = new QScrollArea(this);
  	img_scroll_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  	img_scroll_->setMinimumSize(300, 300);
        img_scroll_->setMaximumSize(300, 300);
  	img_scroll_->setPalette(palette);
  	img_scroll_->setBackgroundRole(QPalette::Background);
  	img_scroll_->setAlignment(Qt::AlignTop);
  	img_scroll_->setWidget(img_ueye_);

  	g0->addWidget(img_scroll_, 0, 0);

	img_button_ = new QPushButton("Camera", this);
	img_button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        //img_button_->setAlignment(Qt::AlignTop);
        g0->addWidget(img_button_);
	g0->setAlignment(img_button_, Qt::AlignTop);


}

void AssemblyUEyeTest::connectImageProducer_image(const QObject* sender, const char* signal)
{
  NQLog("AssemblyImageView", NQLog::Debug) << "connectImageProducer_image";

  img_ueye_->connectImageProducer(sender, signal);
}

void AssemblyUEyeTest::disconnectImageProducer_image(const QObject* sender, const char* signal)
{
  NQLog("AssemblyImageView", NQLog::Debug) << "disconnectImageProducer_image";

  img_ueye_->disconnectImageProducer(sender, signal);
}


