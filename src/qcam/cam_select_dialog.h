/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022, Utkarsh Tiwari <utkarsh02t@gmail.com>
 *
 * cam_select_dialog.h - qcam - Camera Selection dialog
 */

#pragma once

#include <string>

#include <libcamera/camera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/controls.h>
#include <libcamera/property_ids.h>

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

class CameraSelectorDialog : public QDialog
{
	Q_OBJECT
public:
	CameraSelectorDialog(libcamera::CameraManager *cameraManager,
			     bool isScriptRunning, std::string scriptPath, QWidget *parent);

	~CameraSelectorDialog() = default;

	std::string getCameraId();

	std::string getCaptureScript();

	/* Hotplug / Unplug Support. */
	void cameraAdded(libcamera::Camera *camera);

	void cameraRemoved(libcamera::Camera *camera);

	/* Camera Information */
	void updateCamInfo(const std::shared_ptr<libcamera::Camera> &camera);

	/* Capture script support. */
	void handleCaptureScriptButton();
	void informScriptReset();
	void informScriptRunning(std::string scriptPath);

Q_SIGNALS:
	void stopCaptureScript();

private:
	libcamera::CameraManager *cm_;

	bool isScriptRunning_;
	std::string scriptPath_;

	/* UI elements. */
	QComboBox *cameraIdComboBox_;
	QLabel *cameraLocation_;
	QLabel *cameraModel_;

	QVBoxLayout *captureWidgetLayout_;
	QPushButton *captureScriptButton_;
	QLabel *scriptPathLabel_ = new QLabel;
};
