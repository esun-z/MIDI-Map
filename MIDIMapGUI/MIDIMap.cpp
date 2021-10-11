#include "MIDIMap.h"

MIDIMap::MIDIMap(QWidget *parent)
    : QWidget(parent){
    ui.setupUi(this);
	
	qDebug() << "Launched.\n";
	QObject::connect(ui.cbChannel, SIGNAL(clicked()), this, SLOT(FreshLineEditState()));
	QObject::connect(ui.cbMKey, SIGNAL(clicked()), this, SLOT(FreshLineEditState()));
	QObject::connect(ui.cbDynamic, SIGNAL(clicked()), this, SLOT(FreshLineEditState()));
	QObject::connect(ui.pbAdd, SIGNAL(clicked()), this, SLOT(AddMap()));
	QObject::connect(ui.pbFreshDevice, SIGNAL(clicked()), this, SLOT(GetMidiDevice()));
	QObject::connect(ui.pbRun, SIGNAL(clicked()), this, SLOT(RunMIDIMap()));
	QObject::connect(ui.pbRemove, SIGNAL(clicked()), this, SLOT(RemoveMap()));

	//limit the range of lineedit input
	ui.leChannel->setValidator(new QIntValidator(0, 65535, this));
	ui.leMKey->setValidator(new QIntValidator(0, 65535, this));
	ui.leDynamic->setValidator(new QIntValidator(0, 65535, this));
	ui.leKeyCode->setValidator(new QIntValidator(0, 65535, this));

	//disable lineedit at launching
	ui.leChannel->setEnabled(false);
	ui.leMKey->setEnabled(false);
	ui.leDynamic->setEnabled(false);

	//set list widget to singal selection
	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.listWidget->setCurrentRow(0);

	//refresh midi devices at launching
	GetMidiDevice();
}



void MIDIMap::PrintSetting() {

	std::cout << "\nMIDI input port name: ";
	printf("%s\n", qPrintable(setting.nameMidiIn));
	qDebug() << "Print device name";
	std::cout << "Number of key maps: " << setting.numKeyMap << "\n\n";
	for (int i = 0; i < setting.numKeyMap; ++i) {
		std::cout << "Map#" << i << ":\n";
		std::cout << "MIDI input: ";
		for (int j = 0; j < 3; ++j) {
			if (setting.map[i].midiIgnore[j] == 0) {
				std::cout << "NL	";
			}
			else {
				std::cout << setting.map[i].midi[j] << "	";
			}
		}
		std::cout << "\n";
		std::cout << "Keys: ";
		for (int j = 0; j < setting.map[i].numKey; ++j) {
			std::cout << setting.map[i].key[j] << "	";
		}
		std::cout << "\n\n";
	}

	return;
}

void MIDIMap::WriteSetting() {
	qDebug() << "Writing Files...";
	freopen("config.cfg", "w", stdout);
	//printf("%s\n%d\n", qPrintable(*setting.nameMidiIn),setting.numKeyMap);
	printf("%s\n", qPrintable(setting.nameMidiIn));
	std::cout << "\n" << setting.numKeyMap << "\n";
	for (int i = 0; i < setting.numKeyMap; ++i) {
		std::cout << "\n";
		for (int j = 0; j < 3; ++j) {
			std::cout << setting.map[i].midiIgnore[j] << " ";
		}
		for (int j = 0; j < 3; ++j) {
			if (setting.map[i].midiIgnore[j]) {
				std::cout << setting.map[i].midi[j] << " ";
			}
			else {
				std::cout << 0 << "	";
			}
		}
		std::cout << "\n";
		std::cout << setting.map[i].numKey << "	";
		for (int j = 0; j < setting.map[i].numKey; ++j) {
			std::cout << setting.map[i].key[j] << " ";
		}
		std::cout << "\n";
	}
	freopen("CON", "w", stdout);
	return;
}

void MIDIMap::FreshLineEditState() {

	qDebug() << "Fresh Line Edit State\n";
	//refresh lineedit state when the checkboxes are clicked
	bool a = ui.cbChannel->isChecked();
	bool b = ui.cbMKey->isChecked();
	bool c = ui.cbDynamic->isChecked();

	ui.leChannel->setEnabled(a);
	ui.leMKey->setEnabled(b);
	ui.leDynamic->setEnabled(c);

	return;
}

void MIDIMap::AddMap() {
	bool flag = true;
	QString qstr;
	

	//check if there is a checkbox being checked
	if (ui.cbChannel->isChecked() == false && ui.cbMKey->isChecked() == false && ui.cbDynamic->isChecked() == false) {
		MessageBox(NULL, L"At least 1 limitation is needed.", L"Error", NULL);
		flag = false;
	}

	//check if there is a midi input device selected
	if (ui.combSelectDevice->currentIndex() < 1) {
		MessageBox(NULL, L"Please select a midi input device.", L"Error", NULL);
		flag = false;
	}
	else {
		//set midi device name
		
		setting.nameMidiIn = ui.combSelectDevice->currentText();
		//std::string *str = &ui.combSelectDevice->currentText().toStdString();
		//setting.nameMidiIn = &ui.combSelectDevice->currentText().toStdString();

	}
	qDebug() << "Device selected.";

	//check midi message setting
	if (ui.cbChannel->isChecked()) {
		
		if (ui.leChannel->text().isEmpty()) {
			MessageBox(NULL, L"Lack for the channel value.", L"Error", NULL);
			flag = false;
		}
		else {
			setting.map[ui.listWidget->count()].midiIgnore[0] = true;
			setting.map[ui.listWidget->count()].midi[0] = ui.leChannel->text().toInt();
			qstr += ui.leChannel->text();
			qstr += "	";
		}
	}
	else {
		setting.map[ui.listWidget->count()].midiIgnore[0] = false;
		qstr += "NL	";
	}
	if (ui.cbMKey->isChecked()) {
		
		if (ui.leMKey->text().isEmpty()) {
			MessageBox(NULL, L"Lack for the MIDI Key value.", L"Error", NULL);
			flag = false;
		}
		else {
			setting.map[ui.listWidget->count()].midiIgnore[1] = true;
			setting.map[ui.listWidget->count()].midi[1] = ui.leMKey->text().toInt();
			qstr += ui.leMKey->text();
			qstr += "	";
		}
	}
	else {
		setting.map[ui.listWidget->count()].midiIgnore[1] = false;
		qstr += "NL	";
	}
	if (ui.cbDynamic->isChecked()) {
		
		if (ui.leDynamic->text().isEmpty()) {
			MessageBox(NULL, L"Lack for the Dynamic value.", L"Error", NULL);
			flag = false;
		}
		else {
			setting.map[ui.listWidget->count()].midiIgnore[2] = true;
			setting.map[ui.listWidget->count()].midi[2] = ui.leDynamic->text().toInt();
			qstr += ui.leDynamic->text();
			qstr += "	";
		}
	}
	else {
		setting.map[ui.listWidget->count()].midiIgnore[2] = false;
		qstr += "NL	";
	}

	if (ui.leKeyCode->text().isEmpty()) {
		MessageBox(NULL, L"Lack for a key code.", L"Error", NULL);
		flag = false;
	}
	else {
		setting.map[ui.listWidget->count()].numKey = 1;
		setting.map[ui.listWidget->count()].key[0] = ui.leKeyCode->text().toInt();
		qstr += "Key=";
		qstr += ui.leKeyCode->text();
	}

	if (flag == true) {
		ui.listWidget->addItem(qstr);
		setting.numKeyMap = ui.listWidget->count();
		//PrintSetting();
		WriteSetting();
	}
	else {
		qDebug() << "Fail to add a new map.";
	}

	

}

void MIDIMap::GetMidiDevice() {

	std::unique_ptr<RtMidiIn> RtmIn(new RtMidiIn());
	
	//get midi input ports counted
	unsigned int numMidiIn = RtmIn->getPortCount();
	
	//get midi input ports' names and update the contents in the combobox
	std::string namePort;
	nameListMidiIn.clear();
	if (numMidiIn > 0) {
		for (unsigned int i = 0; i<numMidiIn; i++) {
			try {
				namePort = RtmIn->getPortName(i);
				namePort.pop_back();
				namePort.pop_back();
				nameListMidiIn << QString::fromStdString(namePort);
			}
			catch (RtMidiError &error) {
				error.printMessage();
				std::cout << "Error getting input ports." << std::endl;
			}
			std::cout << "Input Port #" << i + 1 << ": " << namePort << '\n';
		}
	}
	else {
		std::cout << "No Input Port Available." << std::endl;
		MessageBox(NULL, L"No Input Port Available.", L"Error", NULL);
	}
	ui.combSelectDevice->clear();
	ui.combSelectDevice->addItem("Select a MIDI input device");
	ui.combSelectDevice->addItems(nameListMidiIn);

	return;
}

void MIDIMap::RunMIDIMap() {

	//WriteSetting();
	qDebug() << "Copying File...";
	QFile file(":/MIDIMap/MIDIMap.exe");
	

	file.copy(":/exe/MIDIMap.exe", "MIDIMap.exe");

	//ShellExecute(NULL, L"Open", L"MIDIMap.exe", NULL, L"", SW_HIDE);
	
	lpExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	lpExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	lpExecInfo.lpVerb = L"Open";
	lpExecInfo.hwnd = NULL;
	lpExecInfo.lpFile = L"MIDIMap.exe";
	lpExecInfo.lpDirectory = L"";
	lpExecInfo.nShow = SW_HIDE;
	lpExecInfo.lpParameters = NULL;
	lpExecInfo.hInstApp = NULL;
	ShellExecuteEx(&lpExecInfo);

	//Change the Run botton to Stop, and link it to the Stop function
	ui.pbRun->setText("Stop");
	QObject::disconnect(ui.pbRun, SIGNAL(clicked()), this, SLOT(RunMIDIMap()));
	QObject::connect(ui.pbRun, SIGNAL(clicked()), this, SLOT(StopMIDIMap()));

	return;
}

void MIDIMap::StopMIDIMap() {

	//Terminate MIDIMap process
	TerminateProcess(lpExecInfo.hProcess, 0);

	//Change the Stop button to Run, and link it to the Run function
	ui.pbRun->setText("Run");
	QObject::disconnect(ui.pbRun, SIGNAL(clicked()), this, SLOT(StopMIDIMap()));
	QObject::connect(ui.pbRun, SIGNAL(clicked()), this, SLOT(RunMIDIMap()));

	return;
}

void MIDIMap::RemoveMap() {

	if (ui.listWidget->count() == 0) {
		MessageBox(NULL, L"There is nothing to remove.", L"Error", NULL);
	}
	else {
		int seqRemoveMap = ui.listWidget->currentRow();
		ui.listWidget->takeItem(seqRemoveMap);
		setting.numKeyMap--;
		for (int i = seqRemoveMap; i < setting.numKeyMap; ++i) {
			setting.map[i] = setting.map[i + 1];
		}
		WriteSetting();
	}


	return;
}