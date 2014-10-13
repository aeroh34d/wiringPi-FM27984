

#include "FM27984.h"

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

const uint16_t FM27984::Config0[CONFIG_REGS] = {0xD001, 0, 0x0400, 0x86D3};

FM27984::FM27984() {
	init(975,1,6);
}

FM27984::FM27984(int _chan, int _vol, int _sst) {
	init(_chan,_vol,_sst);
}

FM27984::~FM27984() {
	close(fd);
}

void FM27984::init(int _chan, int _vol, int _sst) {
	fd = wiringPiI2CSetup(SLAVE_ID);
	if (fd < 0) {
		printf("Failed to open \n");
	}
	chan0 = _chan;
	vol0 = _vol;
	sst0 = _sst;
}

void FM27984::reset() {
	vol = vol0;
	chan = chan0;
	sst = sst0;
	memcpy(Config,Config0,CONFIG_REGS * sizeof(uint16_t));
	Config[1] &= ~CHAN_MASK;
	Config[1] |= (chan - MIN_FREQ) << CHAN;
	Config[3] &= ~VOLUME_MASK;
	Config[3] |= vol  << VOLUME;   
	tune_enable();
	setConfig();
	tune_disable();
}

void FM27984::tune_enable() {
	Config[1] |= (1 << TUNE);
}

void FM27984::tune_disable() {
	Config[1] &= ~(1 << TUNE);
}

bool FM27984::tune_done() {
	// see if seek/tune is complete  assumes getStatus() called recently
    if (stc && !(Config[0] & SEEK_MASK)) return true;
	if (stc) {
		seek_off();
		chan = seekchan + MIN_FREQ;
		setConfig();
		return true;
	}
	return false;
}

void FM27984::seek_off() {
	Config[0] &= ~SEEK_MASK;   // turn off seek
}

void FM27984::seek_up() {
	Config[0] &= ~SEEK_MASK;
	Config[0] |= SEEK_UP;
}

void FM27984::seek_down() {
	Config[0] &= ~SEEK_MASK;
	Config[0] |= SEEK_DOWN;
}

bool FM27984::vol_update(int val) {
	int newval = vol + val;
	if ( newval < 0 ||  newval > MAX_VOL) return false;
	vol =  newval;
	Config[3] &= ~VOLUME_MASK;
	Config[3] |= vol  << VOLUME;  
	return true;
}

bool FM27984::chan_update(int val) {
	return chan_set(chan + val);
}

bool FM27984::chan_set(int val) {
	if (val < MIN_FREQ || val > MAX_FREQ) return false;
	chan = val;
	Config[1] &= ~CHAN_MASK;
	Config[1] |= (chan - MIN_FREQ) << CHAN;
	tune_enable();
	return true;
}

int FM27984::chan_get() {
	return chan;
}

bool FM27984::sst_update(int val) {
	int newval = sst + val;
	if ( newval < 0 ||  newval > MAX_SST) return false;
	sst =  newval;
	Config[3] &= ~SEEKTH_MASK;
	Config[3] |= sst  << SEEKTH;  
	return true;
}

// Set configuration register

void FM27984::setConfig()
{
	int i;

	for(i=0; i< CONFIG_REGS;i++) {
		wiringPiI2CWriteReg16(fd, CONFIG_REG_START + i, (int)Config[i]);
	}
}



void FM27984::getStatus()
{
	int i;
	for(i=0; i< STATUS_REGS;i++) {
		Status[i] = wiringPiI2CReadReg16(fd, STATUS_REG_START + i);
	}
    seekchan = Status[0] & READCHAN_MASK;
    stc =  Status[0] >> STC  & 1;
    sf =  Status[0] >> SF & 1;
    st =  Status[0] >> ST & 1;
    rss = (Status[1] & RSS_MASK) >> RSS;
    fmtrue = Status[1] >> FM_TRUE & 1;
    fmready = Status[1] >> FM_READY & 1;
    if (stc) chan = seekchan + MIN_FREQ;
}

//  these shouldn't really by in  class file

void FM27984::printConfig() {
    int config_chan;
	char str[128];
	sprintf(str,"config %04x %04x %04x %04x",Config[0],Config[1],Config[2],Config[3]);
	Serial.println(str);
	config_chan = ((Config[1] & CHAN_MASK) >> CHAN ) + MIN_FREQ;
	sprintf(str," fm channel %d.%d  volume %d  ss %d  sst %d   %d.%d",
          chan/10,chan%10,vol,rss,sst,config_chan/10,config_chan%10);
	Serial.println(str);
}

void FM27984::printStatus() {
	int chan;
	char str[128];
	sprintf(str,"status %04x %04x",Status[0],Status[1]);
	Serial.println(str);
	chan = seekchan + MIN_FREQ;
	sprintf(str," seek channel %d.%d  ss %d complete %d fail %d %d %d",
		chan/10,chan%10,rss,stc,sf,fmtrue,fmready);
	Serial.println(str);
}
