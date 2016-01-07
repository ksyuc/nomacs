/*******************************************************************************************************
DkPreferenceWidgets.cpp
Created on:	14.12.2015
 
nomacs is a fast and small image viewer with the capability of synchronizing multiple instances
 
Copyright (C) 2011-2014 Markus Diem <markus@nomacs.org>
Copyright (C) 2011-2014 Stefan Fiel <stefan@nomacs.org>
Copyright (C) 2011-2014 Florian Kleber <florian@nomacs.org>

This file is part of nomacs.

nomacs is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

nomacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************************************/

#include "DkPreferenceWidgets.h"

#include "DkImageStorage.h"
#include "DkWidgets.h"
#include "DkSettings.h"
#include "DkUtils.h"

#pragma warning(push, 0)	// no warnings from includes
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QStyleOption>
#include <QPainter>
#include <QAction>
#include <QFileInfo>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QSpinBox>

#include <QButtonGroup>
#include <QRadioButton>

#include <QDebug>
#pragma warning(pop)

namespace nmc {

DkPreferenceWidget::DkPreferenceWidget(QWidget* parent) : DkWidget(parent) {

	createLayout();

	QAction* nextAction = new QAction(tr("next"), this);
	nextAction->setShortcut(Qt::Key_Down);
	connect(nextAction, SIGNAL(triggered()), this, SLOT(nextTab()));
	addAction(nextAction);

	QAction* previousAction = new QAction(tr("previous"), this);
	previousAction->setShortcut(Qt::Key_Up);
	connect(previousAction, SIGNAL(triggered()), this, SLOT(previousTab()));
	addAction(previousAction);
}

void DkPreferenceWidget::createLayout() {

	// create tab entries
	QWidget* tabs = new QWidget(this);
	tabs->setObjectName("DkPreferenceTabs");

	mTabLayout = new QVBoxLayout(tabs);
	mTabLayout->setContentsMargins(0, 60, 0, 0);
	mTabLayout->setSpacing(0);
	mTabLayout->setAlignment(Qt::AlignTop);

	// create central widget
	QWidget* centralWidget = new QWidget(this);
	mCentralLayout = new QStackedLayout(centralWidget);
	mCentralLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* dummy = new QWidget(this);
	QHBoxLayout* layout = new QHBoxLayout(dummy);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignLeft);
	layout->addWidget(tabs);
	layout->addWidget(centralWidget);

	// add a scroll area
	DkResizableScrollArea* scrollArea = new DkResizableScrollArea(this);
	scrollArea->setObjectName("DkScrollAreaPlots");
	scrollArea->setWidgetResizable(true);
	scrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	scrollArea->setWidget(dummy);

	QVBoxLayout* sL = new QVBoxLayout(this);
	sL->setContentsMargins(1, 1, 1, 1);	// 1 to get the border
	sL->addWidget(scrollArea);

}

void DkPreferenceWidget::addTabWidget(DkPreferenceTabWidget* tabWidget) {

	mWidgets.append(tabWidget);
	mCentralLayout->addWidget(tabWidget);

	DkTabEntryWidget* tabEntry = new DkTabEntryWidget(tabWidget->icon(), tabWidget->name(), this);
	mTabLayout->addWidget(tabEntry);
	connect(tabEntry, SIGNAL(clicked()), this, SLOT(changeTab()));
	mTabEntries.append(tabEntry);

	// tick the first
	if (mTabEntries.size() == 1)
		tabEntry->click();
}

void DkPreferenceWidget::previousTab() {

	// modulo is sign aware in cpp...
	int idx = (mCurrentIndex == 0) ? mWidgets.size()-1 : mCurrentIndex-1;
	setCurrentIndex(idx);
}

void DkPreferenceWidget::nextTab() {
	setCurrentIndex((mCurrentIndex+1) % mWidgets.size());
}

void DkPreferenceWidget::changeTab() {

	DkTabEntryWidget* te = qobject_cast<DkTabEntryWidget*>(sender());

	for (int idx = 0; idx < mTabEntries.size(); idx++) {

		if (te == mTabEntries[idx]) {
			setCurrentIndex(idx);
		}
	}
}

void DkPreferenceWidget::setCurrentIndex(int index) {

	// something todo here?
	if (index == mCurrentIndex)
		return;

	mCurrentIndex = index;
	mCentralLayout->setCurrentIndex(index);

	// now check the correct one
	for (int idx = 0; idx < mTabEntries.size(); idx++)
		mTabEntries[idx]->setChecked(idx == index);

}

// DkPreferenceTabWidget --------------------------------------------------------------------
DkPreferenceTabWidget::DkPreferenceTabWidget(const QIcon& icon, const QString& name, QWidget* parent) : DkNamedWidget(name, parent) {

	setObjectName("DkPreferenceTab");
	mIcon = icon;

	createLayout();
}

void DkPreferenceTabWidget::createLayout() {

	QLabel* titleLabel = new QLabel(name(), this);
	titleLabel->setObjectName("DkPreferenceTitle");

	mInfoLabel = new QLabel(tr(""), this);
	mInfoLabel->setObjectName("DkInfoLabel");

	mLayout = new QGridLayout(this);
	mLayout->setContentsMargins(0, 0, 0, 0);
	mLayout->setAlignment(Qt::AlignTop);
	mLayout->addWidget(titleLabel, 0, 0);
	mLayout->addWidget(mInfoLabel, 2, 0);
}

void DkPreferenceTabWidget::setWidget(QWidget* w) {

	mCentralWidget = w;
	mCentralWidget->setObjectName("DkPreferenceWidget");
	mLayout->addWidget(mCentralWidget, 1, 0);

	connect(w, SIGNAL(infoSignal(const QString&)), this, SLOT(setInfoMessage(const QString&)));
}

void DkPreferenceTabWidget::setInfoMessage(const QString& msg) {
	mInfoLabel->setText(msg);
}

QWidget* DkPreferenceTabWidget::widget() const {
	return mCentralWidget;
}

QIcon DkPreferenceTabWidget::icon() const {
	return mIcon;
}

// DkTabEntryWidget --------------------------------------------------------------------
DkTabEntryWidget::DkTabEntryWidget(const QIcon& icon, const QString& text, QWidget* parent) : QPushButton(text, parent) {

	setObjectName("DkTabEntryWidget");

	QPixmap pm = DkImage::colorizePixmap(icon.pixmap(QSize(32, 32)), QColor(255, 255, 255));
	setIcon(pm);
	setIconSize(QSize(24, 24));

	setFlat(true);
	setCheckable(true);
}

void DkTabEntryWidget::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QPushButton::paintEvent(event);
}

// DkGroupWidget --------------------------------------------------------------------
DkGroupWidget::DkGroupWidget(const QString& title, QWidget* parent) : QWidget(parent) {

	setObjectName("DkGroupWidget");
	mTitle = title;

	createLayout();
}

void DkGroupWidget::createLayout() {

	QLabel* titleLabel = new QLabel(mTitle, this);
	titleLabel->setObjectName("subTitle");

	// we create a content widget to have control over the margins
	QWidget* contentWidget = new QWidget(this);
	mContentLayout = new QVBoxLayout(contentWidget);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(titleLabel);
	layout->addWidget(contentWidget);
}

void DkGroupWidget::addWidget(QWidget* widget) {
	
	mContentLayout->addWidget(widget);
}

void DkGroupWidget::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkGeneralPreference --------------------------------------------------------------------
DkGeneralPreference::DkGeneralPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkGeneralPreference::createLayout() {

	// color settings
	DkColorChooser* highlightColorChooser = new DkColorChooser(QColor(0, 204, 255), tr("Highlight Color"), this);
	highlightColorChooser->setObjectName("highlightColor");
	highlightColorChooser->setColor(&DkSettings::display.highlightColor);
	connect(highlightColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* iconColorChooser = new DkColorChooser(QColor(219, 89, 2, 255), tr("Icon Color"), this);
	iconColorChooser->setObjectName("iconColor");
	iconColorChooser->setColor(&DkSettings::display.iconColor);
	connect(iconColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgColorChooser = new DkColorChooser(QColor(100, 100, 100, 255), tr("Background Color"), this);
	bgColorChooser->setObjectName("backgroundColor");
	bgColorChooser->setColor(&DkSettings::display.bgColor);
	connect(bgColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fullscreenColorChooser = new DkColorChooser(QColor(86,86,90), tr("Fullscreen Color"), this);
	fullscreenColorChooser->setObjectName("fullscreenColor");
	fullscreenColorChooser->setColor(&DkSettings::slideShow.backgroundColor);
	connect(fullscreenColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* fgdHUDColorChooser = new DkColorChooser(QColor(255, 255, 255, 255), tr("HUD Foreground Color"), this);
	fgdHUDColorChooser->setObjectName("fgdHUDColor");
	fgdHUDColorChooser->setColor(&DkSettings::display.hudFgdColor);
	connect(fgdHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkColorChooser* bgHUDColorChooser = new DkColorChooser(QColor(0, 0, 0, 100), tr("HUD Background Color"), this);
	bgHUDColorChooser->setObjectName("bgHUDColor");
	bgHUDColorChooser->setColor((DkSettings::app.appMode == DkSettings::mode_frameless) ?
		&DkSettings::display.bgColorFrameless : &DkSettings::display.hudBgColor);
	connect(bgHUDColorChooser, SIGNAL(accepted()), this, SLOT(showRestartLabel()));

	DkGroupWidget* colorGroup = new DkGroupWidget(tr("Color Settings"), this);
	colorGroup->addWidget(highlightColorChooser);
	colorGroup->addWidget(iconColorChooser);
	colorGroup->addWidget(bgColorChooser);
	colorGroup->addWidget(fullscreenColorChooser);
	colorGroup->addWidget(fgdHUDColorChooser);
	colorGroup->addWidget(bgHUDColorChooser);

	// default pushbutton
	QPushButton* defaultSettings = new QPushButton(tr("Reset All Settings"));
	defaultSettings->setObjectName("defaultSettings");
	defaultSettings->setMaximumWidth(300);

	DkGroupWidget* defaultGroup = new DkGroupWidget(tr("Default Settings"), this);
	defaultGroup->addWidget(defaultSettings);

	// the left column (holding all color settings)
	QWidget* leftColumn = new QWidget(this);
	leftColumn->setMinimumWidth(400);

	QVBoxLayout* leftColumnLayout = new QVBoxLayout(leftColumn);
	leftColumnLayout->setAlignment(Qt::AlignTop);
	leftColumnLayout->addWidget(colorGroup);
	leftColumnLayout->addWidget(defaultGroup);

	// checkboxes
	QCheckBox* cbRecentFiles = new QCheckBox(tr("Show Recent Files on Start-Up"), this);
	cbRecentFiles->setObjectName("showRecentFiles");
	cbRecentFiles->setToolTip(tr("Show the History Panel on Start-Up"));
	cbRecentFiles->setChecked(DkSettings::app.showRecentFiles);

	QCheckBox* cbLoopImages = new QCheckBox(tr("Loop Images"), this);
	cbLoopImages->setObjectName("loopImages");
	cbLoopImages->setToolTip(tr("Start with the first image in a folder after showing the last."));
	cbLoopImages->setChecked(DkSettings::global.loop);

	QCheckBox* cbZoomOnWheel = new QCheckBox(tr("Mouse Wheel Zooms"), this);
	cbZoomOnWheel->setObjectName("zoomOnWheel");
	cbZoomOnWheel->setToolTip(tr("If checked, the mouse wheel zooms - otherwise it is used to switch between images."));
	cbZoomOnWheel->setChecked(DkSettings::global.zoomOnWheel);

	QCheckBox* cbSwitchModifier = new QCheckBox(tr("Switch CTRL with ALT"), this);
	cbSwitchModifier->setObjectName("switchModifier");
	cbSwitchModifier->setToolTip(tr("If checked, CTRL + Mouse is switched with ALT + Mouse."));
	cbSwitchModifier->setChecked(DkSettings::sync.switchModifier);

	QCheckBox* cbEnableNetworkSync = new QCheckBox(tr("Enable LAN Sync"), this);
	cbEnableNetworkSync->setObjectName("networkSync");
	cbEnableNetworkSync->setToolTip(tr("If checked, syncing in your LAN is enabled."));
	cbEnableNetworkSync->setChecked(DkSettings::sync.enableNetworkSync);

	QCheckBox* cbCloseOnEsc = new QCheckBox(tr("Close on ESC"), this);
	cbCloseOnEsc->setObjectName("closeOnEsc");
	cbCloseOnEsc->setToolTip(tr("Close nomacs if ESC is pressed."));
	cbCloseOnEsc->setChecked(DkSettings::app.closeOnEsc);

	QCheckBox* cbCheckForUpdates = new QCheckBox(tr("Check For Updates"), this);
	cbCheckForUpdates->setObjectName("checkForUpdates");
	cbCheckForUpdates->setToolTip(tr("Check for updates on start-up."));
	cbCheckForUpdates->setChecked(DkSettings::sync.checkForUpdates);

	DkGroupWidget* generalGroup = new DkGroupWidget(tr("General"), this);
	generalGroup->addWidget(cbRecentFiles);
	generalGroup->addWidget(cbLoopImages);
	generalGroup->addWidget(cbZoomOnWheel);
	generalGroup->addWidget(cbSwitchModifier);
	generalGroup->addWidget(cbEnableNetworkSync);
	generalGroup->addWidget(cbCloseOnEsc);
	generalGroup->addWidget(cbCheckForUpdates);

	// language
	QComboBox* languageCombo = new QComboBox(this);
	languageCombo->setObjectName("languageCombo");
	languageCombo->setToolTip(tr("Choose your preferred language."));
	DkUtils::addLanguages(languageCombo, mLanguages);
	languageCombo->setCurrentIndex(mLanguages.indexOf(DkSettings::global.language));

	QLabel* translateLabel = new QLabel("<a href=\"http://www.nomacs.org/how-to-translate-nomacs/\">How-to translate nomacs</a>", this);
	translateLabel->setToolTip(tr("Info on how to translate nomacs."));
	translateLabel->setOpenExternalLinks(true);

	DkGroupWidget* languageGroup = new DkGroupWidget(tr("Language"), this);
	languageGroup->addWidget(languageCombo);
	languageGroup->addWidget(translateLabel);

	// the right column (holding all checkboxes)
	QWidget* cbWidget = new QWidget(this);
	QVBoxLayout* cbLayout = new QVBoxLayout(cbWidget);
	cbLayout->setAlignment(Qt::AlignTop);
	cbLayout->addWidget(generalGroup);

	// add language
	cbLayout->addWidget(languageGroup);

	QHBoxLayout* contentLayout = new QHBoxLayout(this);
	contentLayout->setAlignment(Qt::AlignLeft);
	contentLayout->addWidget(leftColumn);
	contentLayout->addWidget(cbWidget);

}

void DkGeneralPreference::showRestartLabel() const {
	emit infoSignal(tr("Please Restart nomacs to apply changes"));
}

void DkGeneralPreference::on_backgroundColor_accepted() const {
	DkSettings::display.defaultBackgroundColor = false;
}

void DkGeneralPreference::on_backgroundColor_resetClicked() const {
	DkSettings::display.defaultBackgroundColor = true;
}

void DkGeneralPreference::on_iconColor_accepted() const {
	DkSettings::display.defaultIconColor = false;
}

void DkGeneralPreference::on_iconColor_resetClicked() const {
	DkSettings::display.defaultIconColor = true;
}

void DkGeneralPreference::on_showRecentFiles_toggled(bool checked) const {

	if (DkSettings::app.showRecentFiles != checked)
		DkSettings::app.showRecentFiles = checked;
}

void DkGeneralPreference::on_closeOnEsc_toggled(bool checked) const {

	if (DkSettings::app.closeOnEsc != checked)
		DkSettings::app.closeOnEsc = checked;
}

void DkGeneralPreference::on_zoomOnWheel_toggled(bool checked) const {

	if (DkSettings::global.zoomOnWheel != checked) {
		DkSettings::global.zoomOnWheel = checked;
	}
}

void DkGeneralPreference::on_checkForUpdates_toggled(bool checked) const {

	if (DkSettings::sync.checkForUpdates != checked)
		DkSettings::sync.checkForUpdates = checked;
}

void DkGeneralPreference::on_switchModifier_toggled(bool checked) const {

	if (DkSettings::sync.switchModifier != checked) {

		DkSettings::sync.switchModifier = checked;

		if (DkSettings::sync.switchModifier) {
			DkSettings::global.altMod = Qt::ControlModifier;
			DkSettings::global.ctrlMod = Qt::AltModifier;
		}
		else {
			DkSettings::global.altMod = Qt::AltModifier;
			DkSettings::global.ctrlMod = Qt::ControlModifier;
		}
	}
}

void DkGeneralPreference::on_loopImages_toggled(bool checked) const {

	if (DkSettings::global.loop != checked)
		DkSettings::global.loop = checked;
}

void DkGeneralPreference::on_networkSync_toggled(bool checked) const {

	if (DkSettings::sync.enableNetworkSync != checked)
		DkSettings::sync.enableNetworkSync = checked;
}

void DkGeneralPreference::on_defaultSettings_clicked() {

	int answer = QMessageBox::warning(this, tr("Reset All Settings"), tr("This will reset all personal settings!"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
	
	if (answer == QMessageBox::Yes) {
		DkSettings::setToDefaultSettings();
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
		qDebug() << "answer is: " << answer << "flushing all settings...";
	}

}

void DkGeneralPreference::on_languageCombo_currentIndexChanged(int index) const {

	if (index >= 0 && index < mLanguages.size()) {
		QString language = mLanguages[index];

		if (DkSettings::global.language != language) {
			DkSettings::global.language = language;
			emit infoSignal(tr("Please Restart nomacs to apply changes"));
		}
	}
}

void DkGeneralPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkDisplaySettings --------------------------------------------------------------------
DkDisplayPreference::DkDisplayPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkDisplayPreference::createLayout() {

	// zoom settings
	QCheckBox* invertZoom = new QCheckBox(tr("Invert Zoom Wheel Behaviour"), this);
	invertZoom->setObjectName("invertZoom");
	invertZoom->setChecked(DkSettings::display.invertZoom);

	QLabel* interpolationLabel = new QLabel(tr("Show pixels if zoom level is above"), this);

	QSpinBox* sbInterpolation = new QSpinBox(this);
	sbInterpolation->setObjectName("interpolationBox");
	sbInterpolation->setToolTip(tr("nomacs will not interpolate images if the zoom level is larger."));
	sbInterpolation->setSuffix("%");
	sbInterpolation->setMinimum(0);
	sbInterpolation->setMaximum(10000);
	sbInterpolation->setAlignment(Qt::AlignRight);
	sbInterpolation->setValue(DkSettings::display.interpolateZoomLevel);

	DkGroupWidget* zoomGroup = new DkGroupWidget(tr("Zoom"), this);
	zoomGroup->addWidget(invertZoom);
	zoomGroup->addWidget(interpolationLabel);
	zoomGroup->addWidget(sbInterpolation);

	// keep zoom radio buttons
	QVector<QRadioButton*> keepZoomButtons;
	keepZoomButtons.resize(DkSettings::raw_thumb_end);
	keepZoomButtons[DkSettings::zoom_always_keep] = new QRadioButton(tr("Always keep zoom"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size] = new QRadioButton(tr("Keep zoom if the size is the same"), this);
	keepZoomButtons[DkSettings::zoom_keep_same_size]->setToolTip(tr("If checked, the zoom level is only kept, if the image loaded has the same level as the previous."));
	keepZoomButtons[DkSettings::zoom_never_keep] = new QRadioButton(tr("Never keep zoom"), this);

	// check wrt the current settings
	keepZoomButtons[DkSettings::display.keepZoom]->setChecked(true);

	QButtonGroup* keepZoomButtonGroup = new QButtonGroup(this);
	keepZoomButtonGroup->setObjectName("keepZoom");
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_always_keep], DkSettings::zoom_always_keep);
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_keep_same_size], DkSettings::zoom_keep_same_size);
	keepZoomButtonGroup->addButton(keepZoomButtons[DkSettings::zoom_never_keep], DkSettings::zoom_never_keep);

	DkGroupWidget* keepZoomGroup = new DkGroupWidget(tr("When Displaying New Images"), this);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_always_keep]);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_keep_same_size]);
	keepZoomGroup->addWidget(keepZoomButtons[DkSettings::zoom_never_keep]);
	
	// icon size
	QSpinBox* sbIconSize = new QSpinBox(this);
	sbIconSize->setObjectName("iconSizeBox");
	sbIconSize->setToolTip(tr("Define the icon size in pixel."));
	sbIconSize->setSuffix(" px");
	sbIconSize->setMinimum(16);
	sbIconSize->setMaximum(1024);
	sbIconSize->setValue(DkSettings::display.iconSize);

	DkGroupWidget* iconGroup = new DkGroupWidget(tr("Icon Size"), this);
	iconGroup->addWidget(sbIconSize);

	// left column
	QWidget* leftWidget = new QWidget(this);
	QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
	leftLayout->setAlignment(Qt::AlignTop);
	leftLayout->addWidget(zoomGroup);
	leftLayout->addWidget(keepZoomGroup);
	leftLayout->addWidget(iconGroup);

	// right column
	QWidget* rightWidget = new QWidget(this);
	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);
	rightLayout->setAlignment(Qt::AlignTop);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setAlignment(Qt::AlignLeft);

	layout->addWidget(leftWidget);
	layout->addWidget(rightWidget);
}

void DkDisplayPreference::on_interpolationBox_valueChanged(int value) const {

	if (DkSettings::display.interpolateZoomLevel != value)
		DkSettings::display.interpolateZoomLevel = value;

}

void DkDisplayPreference::on_iconSizeBox_valueChanged(int value) const {

	if (DkSettings::display.iconSize != value) {
		DkSettings::display.iconSize = value;
		emit infoSignal(tr("Please Restart nomacs to apply changes"));
	}

}

void DkDisplayPreference::on_keepZoom_buttonClicked(int buttonId) const {
	
	if (DkSettings::display.keepZoom != buttonId)
		DkSettings::display.keepZoom = buttonId;
}

void DkDisplayPreference::on_invertZoom_toggled(bool checked) const {

	if (DkSettings::display.invertZoom != checked)
		DkSettings::display.invertZoom = checked;
}


void DkDisplayPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

// DkAdvancedSettings --------------------------------------------------------------------
DkAdvancedPreference::DkAdvancedPreference(QWidget* parent) : QWidget(parent) {

	createLayout();
	QMetaObject::connectSlotsByName(this);
}

void DkAdvancedPreference::createLayout() {

	QVBoxLayout* layout = new QVBoxLayout(this);
}
void DkAdvancedPreference::paintEvent(QPaintEvent *event) {

	// fixes stylesheets which are not applied to custom widgets
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	QWidget::paintEvent(event);
}

}