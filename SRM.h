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
    * Can be activated, deactivated and the associated pin can be extracted.
    */

   public:
     PhysicalSwitch(int number, int pin);
     void Activate();
     void Deactivate();
     int getSwitchNumber();
   private:
     int _pin;
     int _switchNumber;
     bool _state;
};

class SwitchState
{
   /* SwitchState:
    * The abstract notion of state for the switches/bridge.
    * Keeps track of which switches should be activated to obtain the state.
    *
    * This is organized in a double looped ring construct which carries sequence information.
    */

   public:
     SwitchState(PhysicalSwitch** activeSwitches, int nSwitches);
     SwitchState* GetNext();
     SwitchState* GetPrevious();
     void InsertSwitchState(SwitchState* insertedState);
     void SetNext(SwitchState* insertedState);
     void SetPrevious(SwitchState* insertedState);
     int GetNumberOfSwitches();
     PhysicalSwitch** GetSwitches();
     int GetTransition();
     void SetTransition(int transition);
   private:
     PhysicalSwitch** _activeSwitches;
     int _nSwitches;
     int _transition;
     SwitchState* _next;
     SwitchState* _previous;
};

class Bridge
{
  /* Bridge:
   * The abstract layer of the (a)symmetric bridge circuit used to drive the SRM.
   * It contains the switches and all of the possible states of the drive.
   * 
   * The Bridge changes the state and resets all of the switches when needed.
   */

  public:
    Bridge(int numberOfSwitches, PhysicalSwitch** switches);
    void ActivateState(SwitchState* activatedState);
    void TurnOff();
  private:
    int _nSwitches;
    PhysicalSwitch** _switches;
};

class Controller
{
   /* Controller:
    * Contains the switching logic and all of the components.
    */

   public:
     Controller(SwitchState* topState, Bridge* theBridge, Encoder* theEncoder);
     void Setup(int pulsesPerRev, int eRevPerMRev, int nStates, int offset);
     void Logic();
     void CalculateTransitions(int pulsesPerRev, int eRevPerMRev, int nStates, int offset);
     SwitchState* GetCurrentState();
   private:
     void ActivateNextState();
     void ActivatePreviousState();
     Encoder* _encoder;
     Bridge* _bridge;
     SwitchState* _startState;
     SwitchState* _currentState;
};
