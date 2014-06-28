#include <stdio.h>
//#include "Encoder.h"
#include "SRM.h"

#define ENCODER_OPTIMIZE_INTERRUPTS

#define MACHINE_TOPOLOGY 0
/* Machinetype can be set to 2 different modes:
    0 -> mechanical speed = 1/2 * electrical speed
    1 -> mechanical speed = 1/4 * electrical speed
    
    If not defined, there will be no action.
*/

int ENCODERPULSES = 20000;

// The offset
//int offset = 600;
//int offset = 834;
int offset = -834;
//int offset = 0;

// Create all of the switches
PhysicalSwitch* switch1 = new PhysicalSwitch(1, 40);
PhysicalSwitch* switch2 = new PhysicalSwitch(2, 41);
PhysicalSwitch* switch3 = new PhysicalSwitch(3, 42);
PhysicalSwitch* switch4 = new PhysicalSwitch(4, 43);
PhysicalSwitch* switch5 = new PhysicalSwitch(5, 44);
PhysicalSwitch* switch6 = new PhysicalSwitch(6, 45);
PhysicalSwitch* switch7 = new PhysicalSwitch(7, 46);
PhysicalSwitch* switch8 = new PhysicalSwitch(8, 47);
PhysicalSwitch* switch9 = new PhysicalSwitch(9, 48);
PhysicalSwitch* switch10 = new PhysicalSwitch(10, 49);
PhysicalSwitch* switch11 = new PhysicalSwitch(11, 50);
PhysicalSwitch* switch12 = new PhysicalSwitch(12, 51);

// Add all into an array for the bridge (in order to make the TurnOff function work)
PhysicalSwitch* AllSwitches[] = {switch1, switch2, switch3, switch4, switch5, switch6, switch7, switch8, switch9, switch10, switch11, switch12};

// The encoder to read the position.
Encoder* myEncoder = new Encoder(0);

// Init the bridge with all of the switches. Responsible for the activation of the states.
Bridge* myBridge = new Bridge(12, AllSwitches);

// The switchstates
#if MACHINE_TOPOLOGY == 0

    int n_States = 6;
    
    PhysicalSwitch* SwitchArray1[] = {switch1, switch8};
    PhysicalSwitch* SwitchArray2[] = {switch11, switch6};
    PhysicalSwitch* SwitchArray3[] = {switch9, switch4};
    PhysicalSwitch* SwitchArray4[] = {switch7, switch2};
    PhysicalSwitch* SwitchArray5[] = {switch5, switch12};
    PhysicalSwitch* SwitchArray6[] = {switch3, switch10};
    
    SwitchState* State1 = new SwitchState(SwitchArray1, 2);
    SwitchState* State2 = new SwitchState(SwitchArray2, 2);
    SwitchState* State3 = new SwitchState(SwitchArray3, 2);
    SwitchState* State4 = new SwitchState(SwitchArray4, 2);
    SwitchState* State5 = new SwitchState(SwitchArray5, 2);
    SwitchState* State6 = new SwitchState(SwitchArray6, 2);
    
    SwitchState* State7 = new SwitchState(SwitchArray1, 2);
    SwitchState* State8 = new SwitchState(SwitchArray2, 2);
    SwitchState* State9 = new SwitchState(SwitchArray3, 2);
    SwitchState* State10 = new SwitchState(SwitchArray4, 2);
    SwitchState* State11 = new SwitchState(SwitchArray5, 2);
    SwitchState* State12 = new SwitchState(SwitchArray6, 2);
    
    int elecToMech = 2;  // one mechanical period is 2 electrical periods
    
#elif MACHINE_TOPOLOGY == 1
    
    int n_States = 3;
    
    PhysicalSwitch* SwitchArray1[] = {switch1, switch8};
    PhysicalSwitch* SwitchArray2[] = {switch5, switch12};
    PhysicalSwitch* SwitchArray3[] = {switch9, switch4};
    
    SwitchState* State1 = new SwitchState(SwitchArray1, 2);
    SwitchState* State2 = new SwitchState(SwitchArray2, 2);
    SwitchState* State3 = new SwitchState(SwitchArray3, 2);
    
    SwitchState* State4 = new SwitchState(SwitchArray1, 2);
    SwitchState* State5 = new SwitchState(SwitchArray2, 2);
    SwitchState* State6 = new SwitchState(SwitchArray3, 2);
    
    SwitchState* State7 = new SwitchState(SwitchArray1, 2);
    SwitchState* State8 = new SwitchState(SwitchArray2, 2);
    SwitchState* State9 = new SwitchState(SwitchArray3, 2);
    
    SwitchState* State10 = new SwitchState(SwitchArray1, 2);
    SwitchState* State11 = new SwitchState(SwitchArray2, 2);
    SwitchState* State12 = new SwitchState(SwitchArray3, 2);
    
    int elecToMech = 4;  // one mechanical period is 4 electrical periods
    
#endif

// Create the controller, which will do most of the heavy lifting.
Controller* myController = new Controller(State1, myBridge, myEncoder);

int Start = 0;

void setup() {
  // put your setup code here, to run once:

  // Organize all switches in a ring in the correct sequence, so all we have to do is say "Next state"
  State1->InsertSwitchState(State2);
  State2->InsertSwitchState(State3);
  State3->InsertSwitchState(State4);
  State4->InsertSwitchState(State5);
  State5->InsertSwitchState(State6);
  State6->InsertSwitchState(State7);
  State7->InsertSwitchState(State8);
  State8->InsertSwitchState(State9);
  State9->InsertSwitchState(State10);
  State10->InsertSwitchState(State11);
  State11->InsertSwitchState(State12);
  
  // Set up the transitions
  myController->CalculateTransitions(ENCODERPULSES, elecToMech, n_States, offset);
  myBridge->ActivateState(myController->GetCurrentState());
  printf("\n");
}

void loop() {    
  myController->Logic();
}

void resetEncoder()
{
  myEncoder->write(0);
}

int main()
{
	setup();
	int i = 0;
	for(i = 0 ; i <= 40001 ; i++)
	{
		//printf("pos: %i ", myEncoder->read());
		loop();
		//printf("\n");
		myEncoder->write( i % 20000 );
	}
}
