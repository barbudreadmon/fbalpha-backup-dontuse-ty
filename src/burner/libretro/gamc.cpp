// Burner Game Control
#include "burner.h"

static char szPlay[4][4]={"p1 ", "p2 ", "p3 ", "p4 "};

#define KEY(x) { pgi->nInput = GIT_SWITCH; pgi->Input.Switch.nCode = (UINT16)(x); }
#define MACRO(x) { pgi->Macro.nMode = 1; pgi->Macro.Switch.nCode = (UINT16)(x); }

// Configure the misc game controls
INT32 GamcMisc(struct GameInp* pgi, char* szi, INT32 nPlayer)
{
	switch (nPlayer) {
		case 0:
			// Set general controls according to Player 1 settings
			if (strcmp(szi, "diag") == 0) {
				KEY(FBK_F2);
				return 0;
			}
			if (strcmp(szi, "reset") == 0) {
				KEY(FBK_F3);
				return 0;
			}
			if (strcmp(szi, "service" ) == 0) {
				KEY(FBK_9);
				return 0;
			}
			if (strcmp(szi, "service2" ) == 0) {
				KEY(FBK_0);
				return 0;
			}
			if (strcmp(szi, "service3" ) == 0) {
				KEY(FBK_MINUS);
				return 0;
			}
			if (strcmp(szi, "service4" ) == 0) {
				KEY(FBK_EQUALS);
				return 0;
			}
			if (strcmp(szi, "tilt") == 0) {
				KEY(FBK_T);
				return 0;
			}
			
			if (strcmp(szi, "op menu" ) == 0) {
				KEY(FBK_F1);
				return 0;
			}
			
			if (strcmp(szi, "clear credit" ) == 0) {
				KEY(FBK_G);
				return 0;
			}
			
			if (strcmp(szi, "hopper" ) == 0) {
				KEY(FBK_H);
				return 0;
			}
			
			// Mahjong controls
			if (strcmp(szi, "mah a") == 0) {
				KEY(FBK_A);
				return 0;
			}
			
			if (strcmp(szi, "mah b") == 0) {
				KEY(FBK_B);
				return 0;
			}
			
			if (strcmp(szi, "mah c") == 0) {
				KEY(FBK_C);
				return 0;
			}
			
			if (strcmp(szi, "mah d") == 0) {
				KEY(FBK_D);
				return 0;
			}
			
			if (strcmp(szi, "mah e") == 0) {
				KEY(FBK_E);
				return 0;
			}
			
			if (strcmp(szi, "mah f") == 0) {
				KEY(FBK_F);
				return 0;
			}
			
			if (strcmp(szi, "mah g") == 0) {
				KEY(FBK_G);
				return 0;
			}
			
			if (strcmp(szi, "mah h") == 0) {
				KEY(FBK_H);
				return 0;
			}
			
			if (strcmp(szi, "mah i") == 0) {
				KEY(FBK_I);
				return 0;
			}
			
			if (strcmp(szi, "mah j") == 0) {
				KEY(FBK_J);
				return 0;
			}
			
			if (strcmp(szi, "mah k") == 0) {
				KEY(FBK_K);
				return 0;
			}
			
			if (strcmp(szi, "mah l") == 0) {
				KEY(FBK_L);
				return 0;
			}
			
			if (strcmp(szi, "mah m") == 0) {
				KEY(FBK_M);
				return 0;
			}
			
			if (strcmp(szi, "mah n") == 0) {
				KEY(FBK_N);
				return 0;
			}
			
			if (strcmp(szi, "mah kan") == 0) {
				KEY(FBK_LCONTROL);
				return 0;
			}
			
			if (strcmp(szi, "mah pon") == 0) {
				KEY(FBK_LALT);
				return 0;
			}
			
			if (strcmp(szi, "mah chi") == 0) {
				KEY(FBK_SPACE);
				return 0;
			}
			
			if (strcmp(szi, "mah reach") == 0) {
				KEY(FBK_LSHIFT);
				return 0;
			}
			
			if (strcmp(szi, "mah ron") == 0) {
				KEY(FBK_Z);
				return 0;
			}
			
			if (strcmp(szi, "mah ff") == 0) {
				KEY(FBK_Y);
				return 0;
			}
			
			if (strcmp(szi, "mah lc") == 0) {
				KEY(FBK_RALT);
				return 0;
			}
			
			if (strcmp(szi, "mah bet") == 0) {
				KEY(FBK_2);
				return 0;
			}
			
			if (strcmp(szi, "mah score") == 0) {
				KEY(FBK_RCONTROL);
				return 0;
			}

			// Player 1 controls
			if (strcmp(szi, "p1 start") == 0) {
				KEY(FBK_1);
				return 0;
			}
			if (strcmp(szi, "p1 select" ) == 0) {
				KEY(FBK_3);
				return 0;
			}
			if (strcmp(szi, "p1 coin" ) == 0) {
				KEY(FBK_5);
				return 0;
			}
			break;
		case 1:
			if (strcmp(szi, "p2 start") == 0) {
				KEY(FBK_2);
				return 0;
			}
			if (strcmp(szi, "p2 select" ) == 0) {
				KEY(FBK_4);
				return 0;
			}
			if (strcmp(szi, "p2 coin" ) == 0) {
				KEY(FBK_6);
				return 0;
			}
			break;
		case 2:
			if (strcmp(szi, "p3 coin" ) == 0) {
				KEY(FBK_7);
				return 0;
			}
			if (strcmp(szi, "p3 start") == 0) {
				KEY(FBK_3);
				return 0;
			}
			break;
		case 3:
			if (strcmp(szi, "p4 start") == 0) {
				KEY(FBK_4);
				return 0;
			}
			if (strcmp(szi, "p4 coin" ) == 0) {
				KEY(FBK_8);
				return 0;
			}
			break;
	}

	return 0;
}

INT32 GamcAnalogJoy(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nJoy, INT32 nSlide)
{
	INT32 nAxis = 0;

	char* szSearch = szPlay[nPlayer & 3];
	if (_strnicmp(szSearch, szi, 3) != 0)	{	// Not our player
		return 1;
	}
	szi += 3;

	if (szi[0] == 0) {
		return 1;
	}
	if (strncmp(szi + 1, "-axis", 5) != 0) {
		return 1;
	}

	if (strncmp(szi, "x", 1) == 0) {
		nAxis = 0;
	}
	if (strncmp(szi, "y", 1) == 0) {
		nAxis = 1;
	}
	if (strncmp(szi, "z", 1) == 0) {
		nAxis = 2;
	}

	if (strlen(szi) > 6) {
		if (strcmp(&szi[6], "-neg") == 0) {
			nSlide = 3;
		}
		if (strcmp(&szi[6], "-pos") == 0) {
			nSlide = 4;
		}
	}

	switch (nSlide) {
		case 2:								// Sliding
			pgi->nInput = GIT_JOYSLIDER;
			pgi->Input.Slider.nSliderValue = 0x8000;		// Put slider in the middle
			pgi->Input.Slider.nSliderSpeed = 0x0700;
			pgi->Input.Slider.nSliderCenter = 0;
			pgi->Input.Slider.JoyAxis.nAxis = nAxis;
			pgi->Input.Slider.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 1:								// Sliding (centering)
			pgi->nInput = GIT_JOYSLIDER;
			pgi->Input.Slider.nSliderValue = 0x8000;		// Put slider in the middle
			pgi->Input.Slider.nSliderSpeed = 0x0E00;
			pgi->Input.Slider.nSliderCenter = 10;
			pgi->Input.Slider.JoyAxis.nAxis = nAxis;
			pgi->Input.Slider.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 3:								// Absolute, axis-neg
			pgi->nInput = GIT_JOYAXIS_NEG;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		case 4:								// Absolute, axis-pos
			pgi->nInput = GIT_JOYAXIS_POS;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
			break;
		default:							// Absolute, entire axis
			pgi->nInput = GIT_JOYAXIS_FULL;
			pgi->Input.JoyAxis.nAxis = nAxis;
			pgi->Input.JoyAxis.nJoy = (UINT8)nJoy;
	}

	return 0;
}

INT32 GamcPlayer(struct GameInp* pgi, char* szi, INT32 nPlayer, INT32 nDevice)
{
	char* szSearch = szPlay[nPlayer & 3];
	INT32 nJoyBase = 0x4000;
	
	if (_strnicmp(szSearch, szi, 3) != 0) {	// Not our player
		return 1;
	}
	szi += 3;

	// Joystick
	nJoyBase |= nDevice << 8;

	if (strcmp(szi, "up") == 0)	{
		KEY(nJoyBase + 0x02)
	}
	if (strcmp(szi, "down") == 0) {
		KEY(nJoyBase + 0x03)
	}
	if (strcmp(szi, "left") == 0)	{
		KEY(nJoyBase + 0x00)
	}
	if (strcmp(szi, "right") == 0) {
		KEY(nJoyBase + 0x01)
	}

	if (strcmp(szi, "up 2") == 0)	{
		KEY(nJoyBase + 0x06)
	}
	if (strcmp(szi, "down 2") == 0) {
		KEY(nJoyBase + 0x07)
	}
	if (strcmp(szi, "left 2") == 0)	{
		KEY(nJoyBase + 0x04)
	}
	if (strcmp(szi, "right 2") == 0) {
		KEY(nJoyBase + 0x05)
	}

	if (strncmp(szi, "fire ", 5) == 0) {
		char *szb = szi + 5;
		INT32 nButton = strtol(szb, NULL, 0);
		if (nButton >= 1) {
			nButton--;
		}
		KEY(nJoyBase + 0x80 + nButton);
	}

	return 0;
}

#undef MACRO
#undef KEY

