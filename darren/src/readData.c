#include <stdio.h>
#include <portmidi.h>
#include <porttime.h>

int main() {

	int cnt, i, dev; // dev is the device, should always be 3
	PmError retVal;
	const PmDeviceInfo *info;
	PmEvent msg[32];
	PortMidiStream *mstream;
	
	Pm_Initialize();
	cnt = Pm_CountDevices();
	
	if(cnt) {
		for(i=0; i<cnt; i++) {
			info = Pm_GetDeviceInfo(i);
			if(info->input) {
				printf("%d: %s \n", i, info->name);
			}
		}
		dev = 3;
		
		Pt_Start(1, NULL, NULL, NULL);
		retval = Pm_OpenInput(&msstream, dev, NULL, 512L, NULL, NULL);
		
		if(retval != pmNoError) {
			printf("error: %s \n", Pm_GetErrorText(retval));
		}
		else {
			while(1) {
				if(Pm_Poll(mstream)) {
					cnt = Pm_Read(mstream
