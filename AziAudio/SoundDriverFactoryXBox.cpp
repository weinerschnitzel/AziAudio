#include "SoundDriverFactory.h"
#include "DirectSoundDriver.h"
#include "DirectSoundDriverLegacy.h"
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
		case SND_DRIVER_NOSOUND:
			result = new NoSoundDriver();
			break;
		default:
			result = new NoSoundDriver();
	}

	return result;
}