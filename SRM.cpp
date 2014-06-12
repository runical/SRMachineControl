/*
  SRM.cpp - Library for controlling a SR machine with an Arduino Due.
 
  Copyright (c) 2014 Peter van den Hurk

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <stdlib.h>     /* malloc, free, rand */
#include "Arduino.h"
#include "SRM.h"

// Switch

Switch::Switch(int pin)
{
	// Switch sets the pin mode to output, inits it to low and saves the pin.
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	_state = false;
	_pin = pin;
};

void Switch::Activate()
{
	// Activate a switch
	if(not _state)
	{
		digitalWrite(_pin, HIGH);
		_state = true;
	};
};

void Switch::Deactivate()
{
	// Deactivate a switch
	if(_state)
	{
		digitalWrite(_pin, LOW);
		_state = false;
	};
};

int Switch::GetPin()
{
	// Get the pin associated with the switch
	return _pin;
};

// Bridge

// SwitchState

SwitchState::SwitchState(Switch** activeSwitches)
{
	// Switchstates takes an array of Switches, which can be activated in groups.
	_activeSwitches = activeSwitches;
};

void SwitchState::AddSwitch(Switch* AddedSwitch)
{
	// Add a switch to the switch array
	
};

// Controller
