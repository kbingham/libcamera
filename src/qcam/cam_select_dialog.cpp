/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Utkarsh Tiwari <utkarsh02t@gmail.com>
 *
 * cam_select_dialog.cpp - qcam - Camera Selection dialog
 */

#include "cam_select_dialog.h"

#include <string>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QString>

CameraSelectorDialog::CameraSelectorDialog(libcamera::CameraManager *cameraManager,
					   QWidget *parent)
	: QDialog(parent), cm_(cameraManager)
{
	/* Use a QFormLayout for the dialog. */
	QFormLayout *camSelectDialogLayout = new QFormLayout(this);

	/* Setup the camera id combo-box. */
	cameraIdComboBox_ = new QComboBox;
	for (const auto &cam : cm_->cameras())
		cameraIdComboBox_->addItem(QString::fromStdString(cam->id()));

	/* Setup the QDialogButton Box */
	QDialogButtonBox *dialogButtonBox =
		new QDialogButtonBox(QDialogButtonBox::Ok |
				     QDialogButtonBox::Cancel);

	connect(dialogButtonBox, &QDialogButtonBox::accepted,
		this, &QDialog::accept);
	connect(dialogButtonBox, &QDialogButtonBox::rejected,
		this, &QDialog::reject);

	/* Set the layout. */
	camSelectDialogLayout->addRow("Camera: ", cameraIdComboBox_);
	camSelectDialogLayout->addWidget(dialogButtonBox);
}

std::string CameraSelectorDialog::getCameraId()
{
	return cameraIdComboBox_->currentText().toStdString();
}

/* Hotplug / Unplug Support. */
void CameraSelectorDialog::cameraAdded(libcamera::Camera *camera)
{
	cameraIdComboBox_->addItem(QString::fromStdString(camera->id()));
}

void CameraSelectorDialog::cameraRemoved(libcamera::Camera *camera)
{
	int cameraIndex = cameraIdComboBox_->findText(
		QString::fromStdString(camera->id()));

	cameraIdComboBox_->removeItem(cameraIndex);
}
