#include "MDM.h"

using namespace std;

#define MAX_KEYMAP 128

#define KeyDown(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define TryCatch(SOMETHING) try{SOMETHING;}catch(RtMidiError &error){error.printMessage();}

struct KEYMAP {
	bool midiIgnore[3];
	int midi[3];
	int numKey;
	int key[4];
};

struct SETTING {
	bool available = true;
	string nameMidiIn;
	int numKeyMap;
	KEYMAP map[MAX_KEYMAP];
};

SETTING setting;

bool stateKey[MAX_KEYMAP];

//temp values
int type, tmp;
int channel, key, dynamic;
bool flag;


void HandleMessage(vector< unsigned char > *message) {

	for (unsigned int j = 0; j < (message->size()) / 3; ++j) {
		//channel = (int)message->at(j * 3);
		//key = (int)message->at(j * 3 + 1);
		dynamic = (int)message->at(j * 3 + 2);
		for (int i = 0; i < setting.numKeyMap; ++i) {
			flag = true;
			for (int k = 0; k < 2; ++k) {
				if (setting.map[i].midiIgnore[k]) {
					if (setting.map[i].midi[k] != (int)message->at(j * 3 + k)) {
						flag = false;
						break;
					}
				}
			}
			if (setting.map[i].midiIgnore[j * 3 + 2]) {
				if (setting.map[i].midi[j * 3 + 2] > (int)message->at(j * 3 + 2)) {
					flag = false;
				}
			}
			if (flag) {
				if (dynamic == 0) {
					for (int k = 0; k < setting.map[i].numKey; ++k) {
						keybd_event(setting.map[i].key[k], MapVirtualKey(setting.map[i].key[k], 0), KEYEVENTF_KEYUP, 0);
						cout << "\nKey Up: " << setting.map[i].key[k] << "\n";
					}
				}
				else {
					for (int k = 0; k < setting.map[i].numKey; ++k) {
						keybd_event(setting.map[i].key[k], MapVirtualKey(setting.map[i].key[k], 0), 0, 0);
						cout << "\nKey Down: " << setting.map[i].key[k] << "\n";
					}
				}
				
			}
		}
	}


	return;
}

void RtmCallBack(double timeStamp, vector< unsigned char > *message, void *userData) {
	unsigned int nBytes = message->size();
	cout << "\nMIDI Message\n";
	for (unsigned int i = 0; i < nBytes; ++i) {
		type = i % 3;
		switch (type) {
		case 0:
			std::cout << "Channel = ";
			break;

		case 1:
			std::cout << "Key = ";
			break;

		case 2:
			std::cout << "Dynamic = ";
			break;

		default:
			std::cout << "Unknown value = ";
		}
		std::cout << (int)message->at(i) << "  ";
	}
	cout << "\nTime Stamp= " << timeStamp << "	User Data= " << userData << "\n";

	HandleMessage(message);
}

void ListenMidi(SETTING set) {

	string::size_type idx;
	int seqRtmIn;
	unique_ptr<RtMidiIn> RtmIn(new RtMidiIn());
	
	

	unsigned int numMidiIn = RtmIn->getPortCount();
	string namePort;
	flag = false;
	for (unsigned int i = 0; i < numMidiIn; ++i) {
		TryCatch(namePort = RtmIn->getPortName(i));
		namePort.pop_back();
		namePort.pop_back();
		
		idx = set.nameMidiIn.find(namePort);
		//idx = namePort.find(set.nameMidiIn);
		cout << "Comparing names: " << namePort << "	" << set.nameMidiIn << "\n";
		if (idx != string::npos) {
			flag = true;
			seqRtmIn = i;
			break;
		}
	}
	if (!flag) {
		cout << "The specified device could not be found.\n";
		return;
	}
	else {
		cout << "MIDI input device: " << namePort << "\n";
	}

	TryCatch(RtmIn->openPort(seqRtmIn));
	RtmIn->setCallback(RtmCallBack);

	while (1) {
		if (KeyDown(VK_ESCAPE)) {
			cout << "\nQuiting" << endl;
			break;
		}
		Sleep(1);
	}

	return;
}

SETTING ReadSetting(char *nameFile) {

	SETTING set;
	ifstream inFile;

	inFile.open(nameFile, ios::in);
	if (!inFile) {
		cout << "Fail to read config." << endl;
		set.available = false;
		return set;
	}
	

	inFile >> set.nameMidiIn >> set.numKeyMap;

	set.numKeyMap = min(set.numKeyMap, MAX_KEYMAP);

	for (int i = 0; i < set.numKeyMap; ++i) {
		for (int j = 0; j < 3; ++j) {
			inFile >> set.map[i].midiIgnore[j];
		}
		for (int j = 0; j < 3; ++j) {
			inFile >> set.map[i].midi[j];
		}
		inFile >> set.map[i].numKey;
		for (int j = 0; j < set.map[i].numKey; ++j) {
			inFile >> set.map[i].key[j];
		}
	}

	inFile.close();

	return set;

}

void PrintSetting(SETTING set) {

	cout << "\nMIDI input port name: " << set.nameMidiIn << "\n";
	cout << "Number of key maps: " << set.numKeyMap << "\n\n";
	for (int i = 0; i < set.numKeyMap; ++i) {
		cout << "Map#" << i << ":\n";
		cout << "MIDI input: ";
		for (int j = 0; j < 3; ++j) {
			if (set.map[i].midiIgnore[j] == 0) {
				cout << "NL	";
			}
			else {
				cout << set.map[i].midi[j] << "	";
			}
		}
		cout << "\n";
		cout << "Keys: ";
		for (int j = 0; j < set.map[i].numKey; ++j) {
			cout << set.map[i].key[j] << "	";
		}
		cout << "\n\n";
	}

	return;
}



int main() {

	
	setting = ReadSetting("config.cfg");
	PrintSetting(setting);

	if (setting.available) {
		ListenMidi(setting);
	}
	else {
		return 0;
	}
	
	_getch();

	return 0;
}