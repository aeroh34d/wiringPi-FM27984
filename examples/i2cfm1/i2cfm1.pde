#include "FM27984.h"
#include <iostream>


#define MAX_PRESETS 10


FM27984 fm(975,1,6);

uint16_t  fm_presets[MAX_PRESETS] = {919,975}, fm_presetnext=2;
bool seeking;

using namespace std;

void setup()
{
	cout << "FM Demo" << endl;

	fm.reset();

	fm.printConfig();
	printPresets();
}

void loop()
{
	uint8_t c;
	bool update;

	while (seeking) {
		fm.getStatus();
		if (fm.tune_done()) {
			fm.printStatus();
			seeking = false;
		}
		usleep(20);
	}
	cout << "Freq +/-  Vol >/<  Seek u/d  Preset 0-9  AddPreset A  Reset R  SST x/y" << endl << endl << ">> ";
	cin >> c;
	update = false;
	switch(c) {
		case '+':
			update = fm.chan_update(1);
			break;
		case '-':
			update = fm.chan_update(-1);
			break;
		case '>':
			update = fm.vol_update(1);
			break;
		case '<':
			update = fm.vol_update(-1);
			break;
		case 'u':
			fm.seek_up();
			seeking = update = true;
			break;
		case 'd':
			fm.seek_down();
			seeking = update = true;
			break;
		case 'x':
			update = fm.sst_update(1);
			break;
		case 'y':
			update = fm.sst_update(-1);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			fm.chan_set(fm_presets[c - '0']);
			update = true;
			break;
		case 'A':
			add_preset();
			break;
		case 'R':
			fm.reset();
			seeking = false;
			update = true;
			break;
        case '?':
            fm.getStatus();
            fm.printConfig();
			//i2cdump(0x02,4); // config on module
            fm.printStatus();
            printPresets();
            break;

		default:
			break;
	}
	if (update || seeking) {
		fm.setConfig();
		fm.getStatus();
		fm.printConfig();
		fm.printStatus();
		if (seeking && fm.tune_done()) {   // ? tune complete check too?
			seeking = false;
		}
		fm.tune_disable();
	}
}

void add_preset() {
    if (fm_presetnext >= MAX_PRESETS) return;
    fm_presets[fm_presetnext++] = fm.chan_get();
    printPresets();
}


void printPresets() {
	int chan,i;
        char str[128];
        
        cout << "Presets: ";
        for (i=0;i<fm_presetnext;i++) {
          chan = fm_presets[i];
          sprintf(str,"%d.%d ",chan/10,chan%10);
          cout << str;
        }
       	cout << endl;
}

#define SID (0x22 >> 1)
void i2cdump(uint8_t addr, int n)
{
 // int i;
 // uint16_t buff[8];
 // char str[128];

 // Serial.print(addr,HEX); Serial.println(" dump");
 // Wire.beginTransmission(SID);
 // Wire.write(addr);  // start addr
 // Wire.endTransmission();
 // Wire.requestFrom(SID, n*2);
 // for(i=0; i< n;i++) {
 //    buff[i] = Wire.read();
 //    buff[i] = buff[i]<<8 | Wire.read();
 //    sprintf(str,"%04x ",buff[i]);
 //    Serial.print(str);
 // }
 // Serial.println();
}

