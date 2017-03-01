#include "SoundDriverFactory.h"
#if defined(_WIN32)
#include "DirectSoundDriver.h"
#include "XAudio2SoundDriver.h"
#include "DirectSoundDriverLegacy.h"
#include "XAudio2SoundDriverLegacy.h"
#else
#include "SDLSoundDriver.h"
#endif
#include "NoSoundDriver.h"

SoundDriverInterface* SoundDriverFactory::CreateSoundDriver(SoundDriverType DriverID)
{
	SoundDriverInterface *result = NULL;
	switch (DriverID)
	{
		case SND_DRIVER_DS8L :
			result = new DirectSoundDriverLegacy();
			break;
		case SND_DRIVER_DS8:
			result = new DirectSoundDriver();
			break;
		case SND_DRIVER_XA2L:
			result = new XAudio2SoundDriverLegacy();
			break;
		case SND_DRIVER_XA2:
			result = new XAudio2SoundDriver();
			break;
		case SND_DRIVER_NOSOUND:
			result = new NoSoundDriver();
			break;
		default:
			result = new NoSoundDriver();
	}

	return result;
}