#pragma once

#include <iostream>
#include <Windows.h>
#include <memory>
#include <QtWidgets/QWidget>
#include "ui_MIDIMap.h"
#include "RtMidi.h"
#include <qstring.h>
#include <qdir.h>
#include <qfile.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlistwidget.h>
#include <qlist.h>
#include <qdebug.h>

#define MAX_KEYMAP 128

struct KEYMAP {
	bool midiIgnore[3];
	int midi[3];
	int numKey;
	int key[4];
};

struct SETTING {
	bool available = true;
	QString nameMidiIn;
	int numKeyMap;
	KEYMAP map[MAX_KEYMAP];
};


class MIDIMap : public QWidget
{
    Q_OBJECT

public:
    MIDIMap(QWidget *parent = Q_NULLPTR);
	

private:
    Ui::MIDIMapClass ui;
	QStringList nameListMidiIn;
	SETTING setting;
	SHELLEXECUTEINFO lpExecInfo = { 0 };
	void PrintSetting();
	void WriteSetting();

public slots:
	void FreshLineEditState();
	void AddMap();
	void GetMidiDevice();
	void RunMIDIMap();
	void StopMIDIMap();
	void RemoveMap();

};
