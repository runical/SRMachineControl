/*
  SRMachineControl.cpp - Library for controlling a SR machine with an Arduino Due.
 
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

#define DEBUG 1

//#include "arduino.h"
#include <stdio.h>
#include "SRM.h"

//////////////////////////////////////////
// 				PhysicalSwitch			//
//////////////////////////////////////////

PhysicalSwitch::PhysicalSwitch(int pin)
{
	// PhysicalSwitch sets the pin mode to output, inits it to low and saves the pin number.
	
	// pinMode(pin, OUTPUT);
	// digitalWrite(pin, LOW);
	this->_state = false;
	this->_pin = pin;
	
	#ifdef DEBUG
		printf("Switch initialized on pin %i \n", _pin);
	#endif
};

void PhysicalSwitch::Activate()
{
	#ifdef DEBUG
	
	printf("Switch on pin %i has state %i\n", this->_pin, this->_state);
	
	if(this->_state)
	{
		printf("Switch on pin %i already activated (state %i)\n", _pin, _state);
	};
	#endif
	
	// Activate a switch
	if(not this->_state)
	{
		// digitalWrite(_pin, HIGH);
		this->_state = true;
		
		#ifdef DEBUG
			printf("Activated switch on pin %i (state %i)\n", _pin, _state);
		#endif
	};
	
	#ifdef DEBUG
		printf("Switch on pin %i has state %i\n", this->_pin, this->_state);
	#endif
};

void PhysicalSwitch::Deactivate()
{
	#ifdef DEBUG
	printf("Switch on pin %i has state %i\n", this->_pin, this->_state);
	
	if(not this->_state)
	{
		printf("Switch on pin %i already deactivated (state %i)\n", _pin, _state);
	};
	#endif
	
	// Deactivate a switch
	if(this->_state)
	{
		// digitalWrite(_pin, LOW);
		this->_state = false;
		
		#ifdef DEBUG
			printf("Deactivated switch on pin %i (state %i)\n", _pin, _state);
		#endif
	};
	
	#ifdef DEBUG
		printf("Switch on pin %i has state %i\n", this->_pin, this->_state);
	#endif
	
};

//////////////////////////////////////////
// 				SwitchState				//
//////////////////////////////////////////

SwitchState::SwitchState(PhysicalSwitch** activeSwitches, int nSwitches)
{
	// Switchstates takes an array of Switches, which can be activated in groups.
	this->_activeSwitches = activeSwitches;
	this->_nSwitches = nSwitches;
	this->_next = this;
	this->_previous = this;
	
	#ifdef DEBUG
		printf("SwitchState initialized with %i switches\n", _nSwitches);
	#endif
};

void SwitchState::InsertSwitchState(SwitchState* insertedState)
{
	#ifdef DEBUG
		printf("Start adding switchstate\n");
	#endif
	insertedState->SetPrevious(this);
	insertedState->SetNext(this->_next);
	_next->SetPrevious(insertedState);
	this->SetNext(insertedState);
	#ifdef DEBUG
		printf("Added SwitchState\n");
	#endif
};

void SwitchState::SetNext(SwitchState* insertedState)
{
	_next = insertedState;
	#ifdef DEBUG
		printf("Set next\n");
	#endif
};

void SwitchState::SetPrevious(SwitchState* insertedState)
{
	_previous = insertedState;
	#ifdef DEBUG
		printf("Set previous\n");
	#endif
};

SwitchState* SwitchState::GetNext()
{
	return _next;
};

SwitchState* SwitchState::GetPrevious()
{
	return _previous;
};

int SwitchState::GetNumberOfSwitches()
{
	return _nSwitches;
};

PhysicalSwitch** SwitchState::GetSwitches()
{
	return _activeSwitches;
};

//////////////////////////////////////////
//				Bridge					//
//////////////////////////////////////////

Bridge::Bridge(int numberOfSwitches, PhysicalSwitch** switches)
{
	_nSwitches = numberOfSwitches;
	_switches = switches;
	
	#ifdef DEBUG
		printf("Bridge initialized with %i switches\n", _nSwitches);
	#endif
};

void Bridge::TurnOff()
{
	#ifdef DEBUG
		printf("Starting with deactivation of switches.\n");
	#endif
	
	int i;
	
	for(i = 0 ; i < _nSwitches ; i++)
	{
		_switches[i]->Deactivate();
	};
	
	#ifdef DEBUG
		printf("Switches deactivated.\n");
	#endif
};

void Bridge::ActivateState(SwitchState* activatedState)
{
	#ifdef DEBUG
		printf("Started with activation of a new state.\n");
	#endif
	
	int limit = activatedState->GetNumberOfSwitches();
	PhysicalSwitch** tempSwitches = activatedState->GetSwitches();
	this->TurnOff();
	
	int i;
	
	for(i = 0 ; i < limit ; i++)
	{
		tempSwitches[i]->Activate();
	};
	
	#ifdef DEBUG
		printf("Activated the new state\n");
	#endif
};

//////////////////////////////////////////
// 				Controller				//
//////////////////////////////////////////

// Needs to be implemented, Work in progress

Controller::Controller(SwitchState* topState, Bridge* theBridge, Encoder* theEncoder)
{
	this->_startState = topState;
	this->_currentState = topState;
	this->_bridge = theBridge;
	this->_encoder = theEncoder;
	#ifdef DEBUG
		printf("Controller initialized\n");
	#endif
};

void Controller::ActivateNextState()
{
	this->_currentState = this->_currentState->GetNext();
	this->_bridge->ActivateState(this->_currentState);
};

void Controller::ActivatePreviousState()
{
	this->_currentState = this->_currentState->GetPrevious();
	this->_bridge->ActivateState(this->_currentState);
};

void Controller::CalculateOffset()
{
	return;
};

void Controller::CalculateTransitions(int numberOfElectricRevPerMechRev, int nStates)
{
	return;
};

void Controller::Logic()
{
	this->ActivatePreviousState();
	return;
};
