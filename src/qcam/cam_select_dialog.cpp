/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Utkarsh Tiwari <utkarsh02t@gmail.com>
 *
 * cam_select_dialog.cpp - qcam - Camera Selection dialog
 */

#include "cam_select_dialog.h"

#include <memory>
#include <string>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QString>

CameraSelectorDialog::CameraSelectorDialog(libcamera::CameraManager *cameraManager,
					   bool isScriptRunning, QWidget *parent)
	: QDialog(parent), cm_(cameraManager), isScriptRunning_(isScriptRunning)
{
	/* Use a QFormLayout for the dialog. */
	QFormLayout *camSelectDialogLayout = new QFormLayout(this);

	/* Setup the camera id combo-box. */
	cameraIdComboBox_ = new QComboBox;
	for (const auto &cam : cm_->cameras())
		cameraIdComboBox_->addItem(QString::fromStdString(cam->id()));

	/* Set camera information labels. */
	cameraLocation_ = new QLabel;
	cameraModel_ = new QLabel;

	updateCamInfo(cm_->get(getCameraId()));
	connect(cameraIdComboBox_, &QComboBox::currentTextChanged,
		this, [&]() {
			updateCamInfo(cm_->get(
				cameraIdComboBox_->currentText().toStdString()));
		});

	captureScriptButton_ = new QPushButton;
	connect(captureScriptButton_, &QPushButton::clicked,
		this, &CameraSelectorDialog::handleCaptureScriptButton);

	/* Display the action that would be performed when button is clicked. */
	if (isScriptRunning_)
		captureScriptButton_->setText("Stop");
	else
		captureScriptButton_->setText("Open");

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
	camSelectDialogLayout->addRow("Location: ", cameraLocation_);
	camSelectDialogLayout->addRow("Model: ", cameraModel_);
	camSelectDialogLayout->addRow("Capture Script: ", captureScriptButton_);
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

/* Camera Information */
void CameraSelectorDialog::updateCamInfo(const std::shared_ptr<libcamera::Camera> &camera)
{
	if (!camera)
		return;

	const libcamera::ControlList &cameraProperties = camera->properties();

	const auto &location =
		cameraProperties.get(libcamera::properties::Location);
	if (location) {
		switch (*location) {
		case libcamera::properties::CameraLocationFront:
			cameraLocation_->setText("Internal front camera");
			break;
		case libcamera::properties::CameraLocationBack:
			cameraLocation_->setText("Internal back camera");
			break;
		case libcamera::properties::CameraLocationExternal:
			cameraLocation_->setText("External camera");
			break;
		default:
			cameraLocation_->setText("Unkown");
		}
	} else {
		cameraLocation_->setText("Unkown");
	}

	const auto &model = cameraProperties
				    .get(libcamera::properties::Model)
				    .value_or("Unknown");

	cameraModel_->setText(QString::fromStdString(model));
}

/* Capture script support. */
void CameraSelectorDialog::handleCaptureScriptButton()
{
	if (isScriptRunning_) {
		Q_EMIT stopCaptureScript();
		isScriptRunning_ = false;
		captureScriptButton_->setText("Open");
	} else {
		scriptPath_ = QFileDialog::getOpenFileName(this, "Run Capture Script",
							   QDir::currentPath(), "Capture Script (*.yaml)")
				      .toStdString();

		if (!scriptPath_.empty())
			captureScriptButton_->setText("Loaded");
	}
}

void CameraSelectorDialog::informScriptReset()
{
	qInfo("informed of script reset");
	isScriptRunning_ = false;
	scriptPath_.clear();
	captureScriptButton_->setText("Open");
}

void CameraSelectorDialog::informScriptRunning(std::string scriptPath)
{
	isScriptRunning_ = true;
	scriptPath_ = scriptPath;
	captureScriptButton_->setText("Stop");
}
