#include "MDM.h"

using namespace std;

#define MAX_KEYMAP 128

#define MOUSERIGHTSTOP 0
#define MOUSEUPSTOP 1
#define MOUSELEFTSTOP 2
#define MOUSEDOWNSTOP 3

#define KeyDown(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)
#define TryCatch(SOMETHING) try{SOMETHING;}catch(RtMidiError &error){error.printMessage();}

struct KEYMAP {
	bool midiIgnore[3];
	int midi[3];
	int numKey;
	int key[4];
};

struct MOUSEMAP {
	bool midiIgnore[3];
	int midi[3];
	int direct;
	int sensitivity;//better in a resonable range, meaning how far(ScreenHeightOrWidth/65536) to move when reciving a single signal.
};

struct SETTING {
	bool available = true;
	string nameMidiIn;
	int numKeyMap;
	KEYMAP map[MAX_KEYMAP];
	MOUSEMAP mouse[4];
	int mouseMapAvailable;
};



SETTING setting;

bool stateKey[MAX_KEYMAP];
bool stateMouseMove[4] = {0,0,0,0};
bool signalMouseMoveThreadExit = false;


//temp values
int type, tmp;
int channel, key, dynamic;
bool flag;

void HandleMouseMove(RECT screenRect) {
	double x, y;
	double sx = (double)1024 / (screenRect.right - screenRect.left);
	double sy = (double)1024 / (screenRect.bottom - screenRect.top);
	POINT mousePoint;
	while (1) {

		GetCursorPos(&mousePoint);
		x = mousePoint.x;
		y = mousePoint.y;

		if (stateMouseMove[MOUSELEFTSTOP]) {
			x -= (double)setting.mouse[MOUSELEFTSTOP].sensitivity*sx;
			x--;
		}
		if (stateMouseMove[MOUSEUPSTOP]) {
			y -= (double)setting.mouse[MOUSEUPSTOP].sensitivity*sy;
			y--;
		}
		if (stateMouseMove[MOUSERIGHTSTOP]) {
			x += (double)setting.mouse[MOUSERIGHTSTOP].sensitivity*sx;
			x++;
		}
		if (stateMouseMove[MOUSEDOWNSTOP]) {
			y += (double)setting.mouse[MOUSEDOWNSTOP].sensitivity*sy;
			y++;
		}
		SetCursorPos(x, y);

		if (signalMouseMoveThreadExit) {
			break;
		}
		Sleep(10);
	}

	return;
}


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
			if (setting.map[i].midiIgnore[2]) {
				if (setting.map[i].midi[2] > (int)message->at(j * 3 + 2)) {
					flag = false;
				}
			}
			if (flag) {
				if (dynamic == 0) {
					for (int k = 0; k < setting.map[i].numKey; ++k) {
						if (setting.map[i].key[k] == 1) {
							mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
						}
						else if (setting.map[i].key[k] == 2) {
							mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
						}
						else {
							keybd_event(setting.map[i].key[k], MapVirtualKey(setting.map[i].key[k], 0), KEYEVENTF_KEYUP, 0);
						}
						cout << "\nKey Up: " << setting.map[i].key[k] << "\n";
					}
				}
				else {
					for (int k = 0; k < setting.map[i].numKey; ++k) {
						if (setting.map[i].key[k] == 1) {
							mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
						}
						else if (setting.map[i].key[k] == 2) {
							mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
						}
						else {
							keybd_event(setting.map[i].key[k], MapVirtualKey(setting.map[i].key[k], 0), 0, 0);
						}
						cout << "\nKey Down: " << setting.map[i].key[k] << "\n";
					}
				}
				
			}
		}

		if (setting.mouseMapAvailable) {
			
			for (int i = 0; i < 4; ++i) {
				flag = true;
				for (int k = 0; k < 2; ++k) {
					if (setting.mouse[i].midiIgnore) {
						if (setting.mouse[i].midi[k] != (int)message->at(j * 3 + k)) {
							flag = false;
							break;
						}
					}
				}
				if (setting.mouse[i].midiIgnore[2]) {
					if (setting.mouse[i].midi[2] > dynamic) {
						flag = false;
					}
				}
				if (flag) {
					if (dynamic == 0) {
						stateMouseMove[i] = false;
						cout << "\nMouse Stop: " << i << endl;

					}
					else {
						stateMouseMove[i] = true;
						cout << "\nMouse Move: " << i << endl;
					}
				}
			}
		}
	}


	return;
}

//RtMIDI CallBack function, to display messages and send them to HandleMessage
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

// main MIDI listening loop
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
		
		if (KeyDown(VK_ESCAPE) && KeyDown(VK_RETURN)) {
			cout << "\nQuiting" << endl;
			break;
		}
		
		Sleep(1);
	}
	

	return;
}

bool Cmp(MOUSEMAP a, MOUSEMAP b) {
	return a.direct < b.direct;
}

//read a setting from config.cfg
SETTING ReadSetting(char *nameFile) {

	SETTING set;
	ifstream inFile;

	inFile.open(nameFile, ios::in);
	if (!inFile) {
		cout << "Fail to read config." << endl;
		set.available = false;
		return set;
	}
	
	getline(inFile, set.nameMidiIn);
	inFile >> set.numKeyMap;

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

	inFile >> set.mouseMapAvailable;
	if (set.mouseMapAvailable == 1) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 3; ++j) {
				inFile >> set.mouse[i].midiIgnore[j];
			}
			for (int j = 0; j < 3; ++j) {
				inFile >> set.mouse[i].midi[j];
			}
			inFile >> set.mouse[i].direct;
			inFile >> set.mouse[i].sensitivity;
		}

		sort(set.mouse, set.mouse + 4, Cmp);
	}
	else {
		set.mouseMapAvailable = 0;
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
	if (setting.mouseMapAvailable == 1) {
		cout << "Mouse Move Available.\n";
		for (int i = 0; i < 4; ++i) {
			cout << "Direct #" << i << ":\n";
			cout << "Map#" << i << ":\n";
			cout << "MIDI input: ";
			for (int j = 0; j < 3; ++j) {
				if (set.mouse[i].midiIgnore[j] == 0) {
					cout << "NL	";
				}
				else {
					cout << set.mouse[i].midi[j] << "	";
				}
			}
			cout << "\n";
			cout << "Sensitivity = " << set.mouse[i].sensitivity << "\n";
		}
	}

	return;
}



int main() {

	
	setting = ReadSetting("config.cfg");
	PrintSetting(setting);
	HWND desktopHwnd = GetDesktopWindow();
	RECT scrRect;
	GetWindowRect(desktopHwnd,&scrRect);
	if (setting.available) {
		thread mouseThread(HandleMouseMove, scrRect);
		ListenMidi(setting);
	}
	else {
		cout << "Error. Exiting..." << endl;
		return 0;
	}
	
	
	signalMouseMoveThreadExit = true;
	Sleep(20);

	return 0;
}