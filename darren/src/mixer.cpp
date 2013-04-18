#include <IAudio.h>
using namespace chip;

class Mixer {
	IAudio[] audioList; // each of the IAudio objects that will be added together
	int size; //size of IAudio array
	
	public:
		Mixer(IAudio[], int); //constructor
		float[] advance(int); //the 0th elements are all added together, the 1st elements, 2nd, all the way to the 
							  //nth elements and the result is returned -- aka move along the sound wave
	};
	
Mixer(IAudio[] elements, int size){
	//constructor
	audioList = (*IAudio)malloc(sizeOf(IAudio)*size);
}

float[] advance(int numSamples){
	//the 0th elements are all added together, the 1st elements, 2nd, all the way to the 
	//nth elements and the result is returned (aka - move along the sound wave)
	float[] mixedFinal = (float)malloc(sizeOf(float)*numSamples);
	
	//todo
	
}
	
