/*
  SRMachineControl.h - Library for controlling a SR machine with an Arduino Due.
  Requires the Encoder Library
 
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

//#ifndef SRM_h_
//#define SRM_h_

#include "Encoder.h"

class PhysicalSwitch
{
   /* PhysicalSwitch:
    * The abstraction of a switch in the bridge circuit.
    * Can be activated and deactivated.
    * It saves state to prevent reactivation of a pin (thus introducing unknow behaviour)
    */

   public:
     PhysicalSwitch(int number, int pin);
     void Activate();
     void Deactivate();
     int _switchnumber;
   private:
     int _pin;
     bool _state;
};

class SwitchState
{
   /* SwitchState:
    * The abstract notion of state for the switches/bridge.
    * Keeps track of which switches should be activated to obtain the state.
    *
    * This is organized in a double looped ring construct which carries sequence information.
    * 
    * Only for storing information
    */

   public:
     SwitchState(PhysicalSwitch** activeSwitches, int nSwitches);
     void InsertSwitchState(SwitchState* insertedState);
     SwitchState* Clone();
		// Setters
     void SetNext(SwitchState* insertedState);
     void SetPrevious(SwitchState* insertedState);
     void SetInterval(int start, int stop);
		// Getters
     int GetNumberOfSwitches();
     PhysicalSwitch** GetSwitches();
     SwitchState* GetNext();
     SwitchState* GetPrevious();
     int* GetInterval();
     int _statenumber;
     bool _exception;
   private:
     PhysicalSwitch** _activeSwitches;
     int _nSwitches;
     int _interval[2];
     SwitchState* _next;
     SwitchState* _previous;
};

class InverterStage
{
  /* InverterStage:
   * The abstract layer of the inverter circuit used to drive the SRM.
   * It contains the switches and all of the possible states of the drive.
   * 
   * The InverterStage keeps track of the state and changes the state of the switches.
   */

  public:
    InverterStage(SwitchState* topState);
    void TurnOff();
    void ActivateNextState();
    void ActivatePreviousState();
    void ActivateCurrentState();
    SwitchState* GetCurrentState();
    void Expand(int eRevPerMRev, int nStates);
  private:
    SwitchState* _startState;
    SwitchState* _currentState;
    void ActivateState(SwitchState* activatedState);
    void DeactivateState(SwitchState* deactivatedState);
};

class Controller
{
   /* Controller:
    * Contains the switching logic and all of the components.
    */

   public:
     Controller(SwitchState* topState, Encoder* theEncoder, int pulsesPerRev, int eRevPerMRev, int nStates, int offset, int direction, int switchingdelay);
     void Logic();
     bool CalculateNext();
     void Step();
     void StepperLogic();
     void PositionLogic();
   private:
     // Function
     int Change();
     int gcd(int x, int y);
     void ControllerDelay(int secondsDelay);
     // Variables
     Encoder* _encoder;
     InverterStage* _inverterStage;
     int _increase;
     int _switchingCounter;
     int _correctionCondition;
     int _error;
     int _direction; // values 1 or -1 accepted [Maybe change to a define?]
     int _switchingDelay;
     unsigned int _switchingTime;
     bool _startup;
     int* _interval;
};
