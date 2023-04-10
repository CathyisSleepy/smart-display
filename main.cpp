
#include "ClearCore.h"
#include "EthernetTcp.h"
#include "EthernetTcpServer.h"


// Specifies which motor to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0

// Define the acceleration limit to be used for each move
int32_t accelerationLimit = 3500; // pulses per sec^2
int32_t velocityLimit = 5000;

// Declares our user-defined helper function
bool MoveAtVelocity(int32_t velocity);
void InitializeMotor();
void UpdateTags();
void Estopped();

#define fwdButton ConnectorIO0
#define revButton ConnectorIO1
#define stopButton ConnectorIO2

#define fwdLight ConnectorIO3
#define revLight ConnectorIO4
#define stopLight ConnectorIO5

#define estopSignal ConnectorDI6
#define pullcordSignal ConnectorDI7

bool fwd = true;

bool running = false;
bool faulted = false;
bool in_auto = true;
bool estop_pressed = false;
bool was_estop = false;
bool is_jogging = false;
bool paused = false;
int motor_speed;
int motor_pos;
uint32_t wait_time  = 9000;
uint32_t press_hold_time = 200;
uint32_t stop_hold_time = 5000;
uint32_t timer_start;
uint32_t press_timer_start;
uint32_t stop_press_timer_start;
uint32_t blink_timer_start;
uint32_t tag_timer_start = Milliseconds();
uint32_t jog_timer_start;
uint32_t estop_timer_start;
bool hmi_fwd_state;
bool hmi_rev_state;
bool hmi_stop_state;
bool fwdState;
bool revState;
bool stopState;
bool fwdState_past;
bool revState_past;
bool stopState_past;

EthernetTcpServer server = EthernetTcpServer(8888);

int main(void) 
{
	
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
/*
	bool dhcpSuccess = EthernetMgr.DhcpBegin();
	if (dhcpSuccess) {
		// DHCP successfully assigned an IP address.
	}
*/
	fwdLight.Mode(Connector::OUTPUT_DIGITAL);
	revLight.Mode(Connector::OUTPUT_DIGITAL);
	stopLight.Mode(Connector::OUTPUT_DIGITAL);
	
	ConnectorUsb.SendLine("Ethernet Started");
	
	ConnectorUsb.SendLine(EthernetMgr.LocalIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.GatewayIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.NetmaskIp().StringValue());
	ConnectorUsb.SendLine(EthernetMgr.DnsIp().StringValue());
	EthernetMgr.Refresh();
	
	//EthernetTcpServer server = EthernetTcpServer(8888);
    //Start listening for TCP connections on port 8888.
    server.Begin();
	
	pullcordSignal.InterruptHandlerSet(&Estopped, InputManager::LOW);
	estopSignal.InterruptHandlerSet(&Estopped, InputManager::LOW);
  
   while (1)
   {
	   EthernetMgr.Refresh();
	   fwdState = fwdButton.State();
	   revState = revButton.State();
	   stopState = stopButton.State();
	   uint32_t current_time = Milliseconds();
	   uint32_t timer_count = current_time - timer_start;
	   uint32_t blink_timer_count = current_time - blink_timer_start;
	   uint32_t press_timer_count = current_time - press_timer_start;
	   uint32_t tag_timer_count = current_time - tag_timer_start;
	   uint32_t estop_timer_count = current_time - estop_timer_start;
	   uint32_t jog_timer_count = current_time - jog_timer_start;
	   // Obtain a reference to a connected client with incoming data available.
	   EthernetTcpClient client = server.Available();
	   
	   if(faulted)
	   {
		   running = false;
		   UpdateTags();
	   }
	   
	   /*if (!EthernetMgr.PhyLinkActive())
	   {
		   faulted = true;
		   running = false;
	   }
	   */
	   
	   if (running)
	   {
		   if (fwdState || hmi_fwd_state)
		   {
			  	stopLight.State(false);
			  	revLight.State(false);
			  	fwdLight.State(true);
			  	fwd = true;
				paused = false;
				  
				 if (motor_speed < 0)
				 {
					 motor_speed = motor_speed * -1;
				 }
			  	   
			  	//if the motor is moving backward move the motor forward
				if (in_auto)
				{
					if (fwdState_past && press_hold_time <= press_timer_count)
					{
						MoveAtVelocity(motor_speed + 1000);
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
			  
			else if (revState || hmi_rev_state)
			{
				stopLight.State(false);
				revLight.State(true);
				fwdLight.State(false);
				
				fwd = false;
				paused = false;
				
				if (motor_speed > 0)
				{
					motor_speed = motor_speed * -1;
				}
				
				if (in_auto)
				{	
					if (revState_past && press_hold_time <= press_timer_count)
					{
						MoveAtVelocity(motor_speed - 1000);
						//ConnectorUsb.SendLine("rapid started");
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
					
					timer_start = Milliseconds();
				}
				
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
			
			else if (stopState || hmi_stop_state)
			{
				stopLight.State(true);
				revLight.State(false);
				fwdLight.State(false);
				MoveAtVelocity(0);
				paused = true;
				   
				if (stopState_past && stop_hold_time <= press_timer_count && !in_auto)
				{
					MoveAtVelocity(0);
					running = 0;
				}
				   
				if (!stopState_past)
				{
					press_timer_start = Milliseconds();
				}
				
				timer_start = Milliseconds();
			}
			
			if ((!fwdState_past||!revState_past) && !paused)
			{
				MoveAtVelocity(motor_speed);
			}
			
			if (in_auto)
			{
				if (timer_count >= wait_time && (!fwdState || !fwdState_past))
				{
					fwd = true;
					paused = false;
					if (motor_speed < 0)
					{
						motor_speed = motor_speed * -1;
					}
					
					stopLight.State(false);
					revLight.State(false);
					fwdLight.State(true);
					MoveAtVelocity(motor_speed);
				}
			}
			
			else if (!in_auto)
			{
				if (is_jogging)
				{
					MoveAtVelocity(motor_speed);
					if(jog_timer_count >= 500)
					{
						is_jogging = false;
						MoveAtVelocity(0);
					}
				} 
				else
				{
					MoveAtVelocity(0);
				}
				
				if (blink_timer_count >= 1000)
				{
					stopLight.State(true);
					revLight.State(true);
					fwdLight.State(true);
				}
			}
	   }
	   
	   else
	   {
		   if (faulted)
		   {
			   fwdLight.State(false);
			   
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
		   
		   else
		   {
		        revLight.State(false);
				fwdLight.State(false);
			 
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
	   
	   hmi_fwd_state = false;
	   hmi_rev_state = false;
	   hmi_stop_state = false;
	   
	   fwdState_past = fwdState;
	   revState_past = revState;
	   //stopState_past = stopState;
	   
	   if (client.Connected()) 
	   {
			//ConnectorUsb.SendLine("a client connected");
			// The server has returned a connected client with incoming data available.
			bool recv_in_prog = false;
			int idex = 0;
			uint16_t bin_select;
			uint16_t data_in;
						   
			while (client.BytesAvailable() > 0)
			{
				// Send the data received from the client over a serial port.
				data_in = client.Read();
				//ConnectorUsb.SendLine(data_in);
				if (data_in == 60 && !recv_in_prog)
				{
					recv_in_prog = true;
					idex = 1;
					ConnectorUsb.SendLine("Parse start");
				}
							   
				else if (data_in == 62 && recv_in_prog && idex > 2)
				{
					recv_in_prog = false;
					ConnectorUsb.SendLine("Parse stop");
				}
							   
				else if (idex == 1 && recv_in_prog)
				{
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
						bin_select = 5;
						break;
						
						case 98: //b -> hmi button input
						bin_select = 6;
						break;
								   
						default:
						//invalid tag so exit and look for next
						bin_select = 0;
						idex = 0;
						recv_in_prog = 0;
						break;
					}
					idex++;
				}
						   
				else if (idex == 2 && recv_in_prog && bin_select != 0)
				{
					switch (bin_select)
					{
						ConnectorUsb.Send("data from tag = ");
						ConnectorUsb.SendLine(data_in);
						case 1: //motor speed
						{
							if (0 <= data_in && data_in <= 255)
							{
								float x = static_cast<float>(data_in) / 255;
								uint32_t speedmin = 0; //pulses per second
								uint32_t speedmax = 3200;
								float result = speedmin + (speedmax - speedmin) * x;
								if (result <= speedmax && result >= speedmin)
								{
									motor_speed = static_cast<int>(result);
									motor.VelMax(motor_speed);
									ConnectorUsb.Send("motor_speed after conversion = ");
									ConnectorUsb.SendLine(result);
								}
							}
							
							if (!fwd)
							{
								motor_speed = motor_speed * -1;
							}
							ConnectorUsb.SendLine(motor_speed);
							if(running && !faulted && in_auto)
							{
								MoveAtVelocity(motor_speed);
							}
						}
						break;
									   
						case 2: //start/stop machine
						{
							if ((data_in == 1 || data_in == 0) && !faulted && !was_estop)
							{
								running = data_in;
								UpdateTags();
							}
							
							if (running && in_auto)
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
								//set the time until the machine reverts back to default foward state
								wait_time = resulty;
							}
						}	
						break;
						
						case 4: //reset faults
						{
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
							else
							{
								continue;
							}
						}
						break;
						
						case 5:
						{
							if (data_in == 1 || data_in == 0)
							{
								in_auto = data_in;
								
								if(!in_auto)
								{
									MoveAtVelocity(0);
								}
							}
						}
						break;
						
						case 6:
						{
							switch(data_in)
							{
								case 0:
								hmi_stop_state = true;
								break;
								
								case 1:
								hmi_rev_state = true;
								break;
								
								case 2:
								hmi_fwd_state = true;
								break;
								
								default:
								break;
							}
						}
						break;
							   	   
						default:
						{
							//invalid tag so exit and look for next
							idex = 0;
							recv_in_prog = false;
						}
						break;
					}
					idex++;
				}
				EthernetMgr.Refresh();					   
			}
	   }
	   	   
	   //ConnectorUsb.Send("tag timer count is: ");
	   //ConnectorUsb.SendLine(tag_timer_count);
	   if (tag_timer_count >= 500)
	   {
		   UpdateTags();
		   tag_timer_start = Milliseconds();
	   }
	   if (was_estop && estop_timer_count >= 1000)
	   {
			was_estop = false;
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

	// Waits for the step command to ramp up/down to the commanded velocity.
	// This time will depend on your Acceleration Limit.
	//ConnectorUsb.SendLine("Ramping to speed...");
	//while (!motor.StatusReg().bit.AtTargetVelocity) 
	//{
		//continue;
	//}

	//ConnectorUsb.SendLine("At Speed");
	return true;
}

void UpdateTags()
{
	EthernetTcpClient cli = server.Available();
	
	uint8_t current_speed_percent = static_cast<int>(motor.HlfbState());
	cli.Send("|speed,");
	cli.Send(current_speed_percent);
	cli.Send("|");
	
	cli.Send("|estop,");
	if (estop_pressed)
	{
		cli.Send("1");
	}
	else if (!estop_pressed)
	{
		cli.Send("0");
	}
		   
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
}

void Estopped()
{
	motor.MoveStopAbrupt();
	Delay_ms(100);
	UpdateTags();
	while(motor.StatusReg().bit.InEStopSensor || !estopSignal.State() || !pullcordSignal.State())
	{
		EthernetMgr.Refresh();
		running = false;
		uint32_t current_time = Milliseconds();
		uint32_t timer_count = current_time - blink_timer_start;
		uint32_t tag_timer = current_time - tag_timer_start;
		estop_pressed = true;
			   
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
		
		if (tag_timer >= 200)
		{
			UpdateTags();
		}
	}
	estop_pressed = false;
	was_estop = true;
	estop_timer_start = Milliseconds();
	UpdateTags();
}