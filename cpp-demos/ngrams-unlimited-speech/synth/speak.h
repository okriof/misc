#ifndef SPEAK_H_
#define SPEAK_H_

#include "speak_lib.h"
#include <cstring>

// dependencies: libespeak-dev (-lespeak)
// http://espeak.sourceforge.net/speak_lib.h

class eSpeak
{
public:
	eSpeak()
	{
		espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL,0);
	}


	void lang(const char *lang)
	{
		espeak_SetVoiceByName(lang);
	}


	void speak(char *word)
	{
		espeak_Synth((char*)word, strlen(word)+1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, NULL, NULL);
		espeak_Synchronize();
	}


	void setRate(int value)
	{
		espeak_SetParameter(espeakRATE, value, 0);
	}


	void setVoices(unsigned char gender,unsigned char age)
	{
		espeak_VOICE *voice_spec=espeak_GetCurrentVoice();
		voice_spec->gender=gender;
		voice_spec->age = age;
		espeak_SetVoiceByProperties(voice_spec);
	}


	~eSpeak()
	{
		espeak_Terminate();
	}
};




#endif
