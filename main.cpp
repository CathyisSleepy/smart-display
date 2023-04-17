
#include "ClearCore.h"
#include "EthernetTcp.h"
#include "EthernetTcpServer.h"


// Specifies which motor to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0

// Define the acceleration limit to be used for each move
int32_t accelerationLimit = 3500; // pulses per sec^2
int32_t velocityLimit = 5000;

// Declares our user-defined helper functions
bool MoveAtVelocity(int32_t velocity);
void InitializeMotor();
void UpdateTags();
void Estopped();

//define the pins we are using for buttons
#define fwdButton ConnectorIO0
#define revButton ConnectorIO1
#define stopButton ConnectorIO2

//define the pins we are using for lights
#define fwdLight ConnectorIO3
#define revLight ConnectorIO4
#define stopLight ConnectorIO5

//define pins for the emergency stops
#define estopSignal ConnectorDI6
#define pullcordSignal ConnectorDI7

//forward/backward bit
bool fwd = true;

//initialize the state bits
bool running = false;
bool faulted = false;
bool in_auto = true;
bool estop_pressed = false;
bool was_estop = false;
bool is_jogging = false;
bool paused = false;

//initialize motor speed int
int motor_speed;

//initialize ALL of the timer variables
uint32_t wait_time  = 10000;
uint32_t press_hold_time = 200;
uint32_t stop_hold_time = 5000;
uint32_t timer_start;
uint32_t press_timer_start;
uint32_t stop_press_timer_start;
uint32_t blink_timer_start;
uint32_t tag_timer_start = Milliseconds();
uint32_t jog_timer_start;
uint32_t estop_timer_start;

//initialize button state holding bits
bool hmi_fwd_state;
bool hmi_rev_state;
bool hmi_stop_state;
bool fwdState;
bool revState;
bool stopState;
bool fwdState_past;
bool revState_past;
bool stopState_past;

//define server on port 8888
EthernetTcpServer server = EthernetTcpServer(8888);

int main(void) 
{
	//initialize Usb serial connection
	ConnectorUsb.Mode(Connector::USB_CDC);
	ConnectorUsb.Speed(9600);
	ConnectorUsb.PortOpen();
	uint32_t timeout = 5000;
	uint32_t startTime = Milliseconds();
	
	while (!ConnectorUsb && Milliseconds() - startTime < timeout)
	{
		continue;
	}
	
	//=======================================
	
	InitializeMotor();
	
	//=====================================
	
	//initialize the Ethernet manager
	while (!EthernetMgr.PhyLinkActive()) 
	{
		  ConnectorUsb.SendLine("The Ethernet cable is unplugged...");
		  Delay_ms(10);
	}

	// Specify the desired network values:
	// ClearCore IP address
	IpAddress ip = IpAddress(169, 254, 207, 250);
	// Router/switch IP address
	IpAddress gateway = IpAddress(169, 254, 207, 1);
	// Mask to separate network vs. host addresses (required for TCP)
	IpAddress netmask = IpAddress(255, 255, 0, 0);
	// Set the network values through the EthernetManager.
	EthernetMgr.Setup();
	EthernetMgr.LocalIp(ip);
	EthernetMgr.GatewayIp(gateway);
	EthernetMgr.NetmaskIp(netmask);

	EthernetMgr.Setup();

	//set the led output pin modes
	fwdLight.Mode(Connector::OUTPUT_DIGITAL);
	revLight.Mode(Connector::OUTPUT_DIGITAL);
	stopLight.Mode(Connector::OUTPUT_DIGITAL);
	
	//for debug
	ConnectorUsb.SendLine("Ethernet Started");
	
	ConnectorUsb.SendLine(EthernetMgr.LocalIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.GatewayIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.NetmaskIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.DnsIp().StringValue());
	EthernetMgr.Refresh();
	///////////////
	
	//EthernetTcpServer server = EthernetTcpServer(8888);
    //Start listening for TCP connections on port 8888.
    server.Begin();
	
	//attach a system interrupt to e stop and pull cord pins to the estopped routine
	pullcordSignal.InterruptHandlerSet(&Estopped, InputManager::LOW);
	estopSignal.InterruptHandlerSet(&Estopped, InputManager::LOW);
  
  //main loop goes forever
   while (1)
   {
	   //refresh the ethernet manager
	   EthernetMgr.Refresh();
	   
	   //update button states
	   fwdState = fwdButton.State();
	   revState = revButton.State();
	   stopState = stopButton.State();
	   
	   //update timers
	   uint32_t current_time = Milliseconds();
	   uint32_t timer_count = current_time - timer_start;
	   uint32_t blink_timer_count = current_time - blink_timer_start;
	   uint32_t press_timer_count = current_time - press_timer_start;
	   uint32_t tag_timer_count = current_time - tag_timer_start;
	   uint32_t estop_timer_count = current_time - estop_timer_start;
	   uint32_t jog_timer_count = current_time - jog_timer_start;
	   // Obtain a reference to a connected client with incoming data available.
	   EthernetTcpClient client = server.Available();
	   
	   //if the machine is faulted stop and update tags
	   if(faulted)
	   {
		   running = false;
		   UpdateTags();
	   }
	   
	   //if the machine is running and has been reset after estop
	   if (running && !was_estop)
	   {
		   //if the forward button has been pressed physically or on the hmi
		   if (fwdState || hmi_fwd_state)
		   {
			   //update light states with only forward light on
			  	stopLight.State(false);
			  	revLight.State(false);
			  	fwdLight.State(true);
				  
				//the machine is running forward
			  	fwd = true;
				//set to not paused
				paused = false;
				  
				//if the motor is reversed set it forward
				if (motor_speed < 0)
				{
					motor_speed = motor_speed * -1;
				}
			  	   
			  	//if it is in automatic mode then do rapid if held or just go forward if pressed
				if (in_auto)
				{
					if (fwdState_past && press_hold_time <= press_timer_count)
					{
						MoveAtVelocity(motor_speed + 1000);
						//this delay is necessary for the rapid function to work
						Delay_ms(1);
					}
					else
					{
						MoveAtVelocity(motor_speed);
						
						if (!fwdState_past)
						{
							press_timer_start = Milliseconds();
						}
					}
				}
			  	
				//if it is in manual then start jog timer to jog foward
				else
				{
					jog_timer_start = Milliseconds();
					blink_timer_start = Milliseconds();
					
					if(!is_jogging)
					{
						is_jogging = true;
					}
				}
		  	} 
			
			//if the reverse button has been pressed physically or on the hmi
			else if (revState || hmi_rev_state)
			{
				//update lights to indicate reverse
				stopLight.State(false);
				revLight.State(true);
				fwdLight.State(false);
				
				//set forward false and pause false
				fwd = false;
				paused = false;
				
				//if the motor is moving forward reverse it
				if (motor_speed > 0)
				{
					motor_speed = motor_speed * -1;
				}
				
				//if it is in automatic mode then do rapid if held or just go forward if pressed
				if (in_auto)
				{	
					if (revState_past && press_hold_time <= press_timer_count)
					{
						MoveAtVelocity(motor_speed - 1000);
						//this delay is necessary because for some reason the
						//move at velocity does not execute properly
						//without it
						Delay_ms(1);
					}
					else
					{
						MoveAtVelocity(motor_speed);
						
						if (!revState_past)
						{
							press_timer_start = Milliseconds();
						}
					}
					
					//start timer to return to default forward state
					timer_start = Milliseconds();
				}
				
				//start jog timer
				else
				{
					jog_timer_start = Milliseconds();
					blink_timer_start = Milliseconds();
					
					if(!is_jogging)
					{
						is_jogging = true;
					}
				}
			}
			
			//if stop button is physically pressed or pressed on the hmi then update state based on manual or auto mode
			else if (stopState || hmi_stop_state)
			{
				//update lights to only stop light lit
				stopLight.State(true);
				revLight.State(false);
				fwdLight.State(false);
				
				//stop motor and set pause state true
				MoveAtVelocity(0);
				paused = true;
				
				//start timer to go back to default forward state
				timer_start = Milliseconds();
			}
			
			//if forward or backward button (this is so the rapid works) was not pressed and not paused
			//run the motor at speed
			if ((!fwdState_past||!revState_past) && !paused)
			{
				MoveAtVelocity(motor_speed);
			}
			
			//if the auto state and it was not estopped without reset
			if (in_auto && !was_estop)
			{
				//if timer has gone past set wait time then return to default forward state
				if (timer_count >= wait_time && (!fwdState || !fwdState_past))
				{
					fwd = true;
					paused = false;
					
					//if the speed is negative (reversed) set it positive (forward)
					if (motor_speed < 0)
					{
						motor_speed = motor_speed * -1;
					}
					
					//update lights to reflect forward state
					stopLight.State(false);
					revLight.State(false);
					fwdLight.State(true);
					MoveAtVelocity(motor_speed);
				}
			}
			
			//if in manual mode then jog if jogging timer is active otherwise set motor
			//to stop moving
			else if (!in_auto)
			{
				//if jog timer is active
				if (is_jogging)
				{
					//move at motor speed
					MoveAtVelocity(motor_speed);
					//for 310 ms (the limit of the timer)
					//this was the sweet spot for responsiveness
					if(jog_timer_count >= 310)
					{
						//once the timer expires jog timer is inactive
						//and motor stops
						is_jogging = false;
						MoveAtVelocity(0);
					}
				} 
				//if jog is not active
				else
				{
					//don't move motor
					MoveAtVelocity(0);
				}
				
				//this sets all lights active again after a second
				//after any button press
				//this was to allow directional buttons to light up
				if (blink_timer_count >= 1000)
				{
					stopLight.State(true);
					revLight.State(true);
					fwdLight.State(true);
				}
			}
	   }
	   
	   //if not running
	   else
	   {
		   //if the clearcore/motor is faulted
		   if (faulted)
		   {
			   //turn off froward light
			   fwdLight.State(false);
			   
			   //blink the stop and reverse lights every second to
			   //indicate the fault
			   if(!stopLight.State() && 1000 <= blink_timer_count)
			   {
				   blink_timer_start = Milliseconds();
				   stopLight.State(true);
				   revLight.State(true);
			   }
			   else if(stopLight.State() && 1000 <= blink_timer_count)
			   {
				   stopLight.State(false);
				   revLight.State(false);
				   blink_timer_start = Milliseconds();
			   }
		   }
		   
		   //if it is not faulted but still not running
		   else
		   {
			    //set reverse and forward lights low
		        revLight.State(false);
				fwdLight.State(false);
				
				//blink the stop light only to indicate that the machine is stopped
				if(!stopLight.State() && 1000 <= blink_timer_count)
				{
					blink_timer_start = Milliseconds();
					stopLight.State(true);
				}
				else if(stopLight.State() && 1000 <= blink_timer_count)
				{
					stopLight.State(false);
					blink_timer_start = Milliseconds();
				}  
		   }
	   }
	   
	   //reset hmi button inputs
	   hmi_fwd_state = false;
	   hmi_rev_state = false;
	   hmi_stop_state = false;
	   
	   //save past states of forward and reverse buttons
	   //for the press and hold functions
	   fwdState_past = fwdState;
	   revState_past = revState;
	   stopState_past = stopState;
	   
	   if (client.Connected()) 
	   {
			//The server has returned a connected client with incoming data available.
			//initialize the variables we need for our tag parser
			bool recv_in_prog = false;
			int idex = 0;
			uint16_t bin_select;
			uint16_t data_in;
			
			//any additional incoming tags are going to be added here
			//format is "<[asci character as tag][value]>" both the tag and value need
			//to be one byte in order to be compatible
			while (client.BytesAvailable() > 0)
			{
				//read incoming byte and store it in data in
				data_in = client.Read();
				//look for the tag start character "<"
				if (data_in == 60 && !recv_in_prog)
				{
					//start the parser
					recv_in_prog = true;
					//set the index to one to let it know we are looking at the
					//tag byte now
					idex = 1;
					ConnectorUsb.SendLine("Parse start");
				}
				
				//if we get a closing byte ">" at any point close the tag parse			   
				else if (data_in == 62 && recv_in_prog && idex > 2)
				{
					recv_in_prog = false;
					ConnectorUsb.SendLine("Parse stop");
				}
				
				//with the start byte received index is 1 and we are looking at the tag			   
				else if (idex == 1 && recv_in_prog)
				{
					//read the tag and select the right bin to put the attached value (if any) into
					switch (data_in)
					{
						case 109: //m -> motor speed set
						bin_select = 1; //bin = motor_speed
						break;
									   
						case 108: //l -> main loop on/off
						bin_select = 2; //bin = on/off
						break;
							   
						case 119: //w -> wait timer set
						bin_select = 3; //bin = timer set
					    break;
						
						case 114: //r -> reset
						bin_select = 4; //bin = reset
						ConnectorUsb.SendLine("reset received");
						break;
						
						case 97: //a -> automatic/manual toggle
						bin_select = 5;//bin = auto toggle
						break;
						
						case 98: //b -> hmi button input
						bin_select = 6;//bin = button input
						break;
								   
						default:
						//invalid tag so exit and look for next
						bin_select = 0;
						idex = 0;
						recv_in_prog = 0;
						break;
					}
					//increment the index
					idex++;
				}
				
				//now that we have a tag and a bin selected		   
				else if (idex == 2 && recv_in_prog && bin_select != 0)
				{
					//look at selected bin and do appropriate actions
					switch (bin_select)
					{
						ConnectorUsb.Send("data from tag = ");
						ConnectorUsb.SendLine(data_in);
						case 1: //motor speed
						{
							//take the motor speed input from the hmi and map it to min and max
							//speed values
							if (0 <= data_in && data_in <= 255)
							{
								float x = static_cast<float>(data_in) / 255;
								uint32_t speedmin = 0; //pulses per second
								uint32_t speedmax = 3200;
								float result = speedmin + (speedmax - speedmin) * x;
								if (result <= speedmax && result >= speedmin)
								{
									motor_speed = static_cast<int>(result);
								}
							}
							
							//map the inputed speed to the current machine direction
							if (!fwd)
							{
								motor_speed = motor_speed * -1;
							}
							
							//if the machine is in auto, not faulted, running,
							//and not estopped without reset move the motor at the
							//updated speed
							if(running && !faulted && in_auto && !was_estop && !paused)
							{
								MoveAtVelocity(motor_speed);
							}
						}
						break;
									   
						case 2: //start/stop machine
						{
							//if the input is valid and the machine is not faulted or
							//estopped without reset then update running to the input
							//and notify the hmi
							if ((data_in == 1 || data_in == 0) && !faulted && !was_estop)
							{
								running = data_in;
								UpdateTags();
							}
							
							//if in auto and running then set the lights to the appropriate
							//direction based on fwd bit
							if (running && in_auto && !paused)
							{
								if (fwd)
								{
									stopLight.State(false);
									revLight.State(false);
									fwdLight.State(true);
								}
								else
								{
									stopLight.State(false);
									revLight.State(true);
									fwdLight.State(false);
								}
								MoveAtVelocity(motor_speed);
							}
							else
							{
								//stop the motor and light up the stop button
								MoveAtVelocity(0);
								stopLight.State(true);
								revLight.State(false);
								fwdLight.State(false);
							}
						}
						break;
							   
						case 3: //set wait_time
						{
							//map 1 byte on 2s to 30s (2000ms to 3000ms)
							float y  = data_in / 255;
							uint32_t waitmin = 5000; //ms
							uint32_t waitmax = 60000; //ms
							uint32_t resulty = waitmin + (waitmax - waitmin) * y;
							//if the result is valid
							if (resulty >= waitmin && resulty <= waitmax)
							{
								//set the time until the machine reverts back to default forward state
								wait_time = resulty;
							}
						}	
						break;
						
						case 4: //reset estop and faults
						{
							//reset estop to allow the machine to run
							was_estop = false;
							
							//if there was a faulted then reinitialize the motor and
							//clear all faults notifying the hmi when done
							if (faulted)
							{
								motor.ClearAlerts();
								motor.ClearFaults(timeout);
								InitializeMotor();
								motor.EnableRequest(true);
								faulted = false;
								ConnectorUsb.SendLine("reset complete");
								UpdateTags();
							}
							//otherwise go to next tag
							else
							{
								continue;
							}
						}
						break;
						
						case 5: //auto/manual set
						{
							//if the the input is valid
							if (data_in == 1 || data_in == 0)
							{
								//update mode
								in_auto = data_in;
								
								//if in manual then stop motor
								if(!in_auto)
								{
									MoveAtVelocity(0);
								}
							}
						}
						break;
						
						case 6: //hmi direction buttons
						{
							//check which button was pressed on the hmi
							switch(data_in)
							{
								case 0: //stop button pressed
								hmi_stop_state = true;
								break;
								
								case 1: //backward button pressed
								hmi_rev_state = true;
								break;
								
								case 2: //forward button pressed
								hmi_fwd_state = true;
								break;
								
								default: //otherwise exit
								break;
							}
						}
						break;
							   	   
						default:
						{
							//invalid tag so exit and look for next tag
							idex = 0;
							recv_in_prog = false;
						}
						break;
					}
					idex++;//increment index
				}
				EthernetMgr.Refresh();	// refresh Ethernet				   
			}
	   }
	   	   
	   //if the tag timer has triggered then update tags to the hmi
	   if (tag_timer_count >= 500)
	   {
		   UpdateTags();
		   tag_timer_start = Milliseconds();
	   }
	   
   }
}


void InitializeMotor()
{
		// Sets the input clocking rate. This normal rate is ideal for ClearPath
		// step and direction applications.
		MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_NORMAL);

		// Sets all motor connectors into step and direction mode.
		MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL,
		Connector::CPM_MODE_STEP_AND_DIR);

		// Set the motor's HLFB mode to bipolar PWM
		motor.HlfbMode(MotorDriver::HLFB_MODE_HAS_BIPOLAR_PWM);
		// Set the HFLB carrier frequency to 482 Hz
		motor.HlfbCarrier(MotorDriver::HLFB_CARRIER_482_HZ);

		// Set the maximum acceleration for each move
		motor.AccelMax(accelerationLimit);

		// Enables the motor; homing will begin automatically if enabled
		motor.EnableRequest(true);
		ConnectorUsb.SendLine("Motor Enabled");

		// Waits for HLFB to assert (waits for homing to complete if applicable)
		ConnectorUsb.SendLine("Waiting for HLFB...");
		int timeout_start = Milliseconds();
		int timeout;
		while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED)
		{
			timeout = Milliseconds();
			if (timeout - timeout_start >= 20000)
			{
				faulted = 1;
				break;
			}
			continue;
		}
		//connect the estop to the motor wait until complete
		while (!motor.EStopConnector(CLEARCORE_PIN_DI6))
		{
			continue;
		}
		ConnectorUsb.SendLine("Motor Ready");
		
		return;
}


bool MoveAtVelocity(int32_t velocity) 
{
	// Check if an alert is currently preventing motion
	if (motor.StatusReg().bit.AlertsPresent || !motor.StatusReg().bit.Enabled || motor.StatusReg().bit.InEStopSensor) 
	{
		ConnectorUsb.SendLine("Motor status: 'In Alert'. Move Canceled.");
		running = false;
		faulted = true;
		UpdateTags();
		return false;
	}

	//ConnectorUsb.Send("Commanding velocity: ");
	//ConnectorUsb.SendLine(velocity);

	// Command the velocity move
	motor.MoveVelocity(velocity);

	//ConnectorUsb.SendLine("At Speed");
	return true;
}

void UpdateTags()
{
	//grab available client
	EthernetTcpClient cli = server.Available();
	
	//this code does not work
	/*
	uint8_t current_speed_percent = static_cast<int>(motor.HlfbState());
	cli.Send("|speed,");
	cli.Send(current_speed_percent);
	cli.Send("|");
	*/
	
	//update estop state to the hmi 
	cli.Send("|estop,");
	if (estop_pressed)
	{
		cli.Send("1");
	}
	else if (!estop_pressed)
	{
		cli.Send("0");
	}
	
	//update run state to the hmi   
	cli.Send("|run,");
	if (running)
	{
		cli.Send("1");
		ConnectorUsb.SendLine("|run,1|");
	}
	else if(!running)
	{
		cli.Send("0");
	}
		
	//update fault state to the hmi    
	cli.Send("|fault,");
	if (faulted)
	{
		cli.Send("1");
	}
	else if(!faulted)
	{
		cli.Send("0");
	}
	cli.Send("|");
	
	//any additional tags added will go here
}

//this is the estop interupt service routine that will run when the estop is pressed
void Estopped()
{
	//stop the motor as abruptly as possible
	motor.MoveStopAbrupt();
	//wait for a little to make sure the motor is stopped
	Delay_ms(100);
	
	//while the estop remains pressed the progam will remain in this loop
	while(motor.StatusReg().bit.InEStopSensor || !estopSignal.State() || !pullcordSignal.State())
	{
		//refresh ethernet
		EthernetMgr.Refresh();
		
		//set the running bit to false and tell the machine it has been estopped
		running = false;
		was_estop = true;
		
		//update timers
		uint32_t current_time = Milliseconds();
		uint32_t timer_count = current_time - blink_timer_start;
		uint32_t tag_timer = current_time - tag_timer_start;
		
		//set estop pressed bit true
		estop_pressed = true;
		
		//blink all the lights
		if(!stopLight.State() && 1000 <= timer_count)
		{
			blink_timer_start = Milliseconds();
			stopLight.State(true);
			revLight.State(true);
			fwdLight.State(true);
		}
		else if(stopLight.State() && 1000 <= timer_count)
		{
			stopLight.State(false);
			revLight.State(false);
			fwdLight.State(false);
			blink_timer_start = Milliseconds();
		}
		
		//if tag timer is done then restart timer
		if (tag_timer >= 200)
		{
			UpdateTags();
			tag_timer_start = Milliseconds();
		}
	}
	
	//set estop bit false and update tags
	estop_pressed = false;
	UpdateTags();
}