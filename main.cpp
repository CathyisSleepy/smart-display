
#include "ClearCore.h"
#include "EthernetTcp.h"
#include "EthernetTcpServer.h"


// Specifies which motor to move.
// Options are: ConnectorM0, ConnectorM1, ConnectorM2, or ConnectorM3.
#define motor ConnectorM0

// Define the acceleration limit to be used for each move
int32_t accelerationLimit = 100000; // pulses per sec^2

// Declares our user-defined helper function, which is used to command moves to
// the motor. The definition/implementation of this function is at the  bottom
// of the example
bool MoveAtVelocity(int32_t velocity);

#define fwdButton ConnectorIO0
#define revButton ConnectorIO1
#define stopButton ConnectorIO2

#define fwdLight ConnectorIO3
#define revLight ConnectorIO4
#define stopLight ConnectorIO5

bool fwd = true;

bool running = true;
int motor_speed;
int wait_time  = 2000;
int timer_start;
bool fwdState_past;
bool revState_past;

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
	while (motor.HlfbState() != MotorDriver::HLFB_ASSERTED) 
	{
		continue;
	}
	ConnectorUsb.SendLine("Motor Ready");
	
	//=====================================
	
	while (!EthernetMgr.PhyLinkActive()) 
	{
		  ConnectorUsb.SendLine("The Ethernet cable is unplugged...");
		  Delay_ms(1000);
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
	
	EthernetTcpServer server = EthernetTcpServer(8888);
   //Start listening for TCP connections on port 8888.
   server.Begin();
	
	bool fwdState;
	bool revState;
	bool stopState;
  
   while (1)
   {
	   EthernetMgr.Refresh();
	   fwdState = fwdButton.State();
	   revState = revButton.State();
	   stopState = stopButton.State();

	   
	   if (fwdState) 
	   {
          stopLight.State(false);
          revLight.State(false);
          fwdLight.State(true);
		  fwd = true;
		  
		  //if the motor is moving backward move the motor forward
		  if (motor_speed < 0)
		  {
			  motor_speed = motor_speed * -1;
			  MoveAtVelocity(motor_speed);
		  }
		  
		  if (fwdState_past)
		  {
			  MoveAtVelocity(motor_speed * 1.4);
		  }
	   }
	   
	   else if (revState)
	   {
		    stopLight.State(false);
		    revLight.State(true);
		    fwdLight.State(false);
		    fwd = false;
		    if (motor_speed > 0)
		    {
			    motor_speed = motor_speed * -1;
				MoveAtVelocity(motor_speed);
		    }
			
			timer_start = Milliseconds();
	   }
	   
	   else if (stopState)
	   {
		   stopLight.State(true);
		   revLight.State(false);
		   fwdLight.State(false);
		   MoveAtVelocity(0);
		   
		   timer_start = Milliseconds();
	   }
	   
	   uint32_t current_time = Milliseconds();
	   uint32_t timer_count = timer_start - current_time;
	   if (timer_count >= wait_time)
	   {
		   fwd = true;
		   if (motor_speed < 0)
		   {
			   motor_speed = motor_speed * -1;
		   }
		   
		   MoveAtVelocity(motor_speed);
	   }
	   
	   // Obtain a reference to a connected client with incoming data available.
	   EthernetTcpClient client = server.Available();
	   if (client.Connected()) 
	   {
				   ConnectorUsb.SendLine("a client connected");
				   // The server has returned a connected client with incoming data available.
				   bool recv_in_prog = false;
				   int idex = 0;
				   int bin_select;
				   uint16_t data_in;
						   
				   while (client.BytesAvailable() > 0)
				   {
					   // Send the data received from the client over a serial port.
					   data_in = client.Read();
					   ConnectorUsb.SendLine(data_in);
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
							   bin_select = 2;
							   
							   case 119: //w -> wait timer set
							   bin_select = 3;
								   
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
							   case 1: //motor speed
							   motor_speed = data_in * 25;
							   if (!fwd)
							   {
								   motor_speed = motor_speed * -1;
							   }
							   ConnectorUsb.SendLine(motor_speed);
							   break;
									   
							   case 2: //start/stop motor
							   running = data_in;
							   if (data_in)
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
								   MoveAtVelocity(0);
								   stopLight.State(true);
								   revLight.State(false);
								   fwdLight.State(false);
							   }
							   break;
							   
							   case 3: //set wait_time
							   wait_time = data_in * 100;
							   break;
							   
							   
							   default:
							   //invalid tag so exit and look for next
							   idex = 0;
							   recv_in_prog = false;
							   break;
						   }
						   idex++;
					   }
					   EthernetMgr.Refresh();					   
				  }
	   } 
	   fwdState_past = fwdState;
	   revState_past = revState;    
   }
}





bool MoveAtVelocity(int32_t velocity) {
	// Check if an alert is currently preventing motion
	if (motor.StatusReg().bit.AlertsPresent) {
		ConnectorUsb.SendLine("Motor status: 'In Alert'. Move Canceled.");
		return false;
	}

	//ConnectorUsb.Send("Commanding velocity: ");
	//ConnectorUsb.SendLine(velocity);

	// Command the velocity move
	motor.MoveVelocity(velocity);

	// Waits for the step command to ramp up/down to the commanded velocity.
	// This time will depend on your Acceleration Limit.
	//ConnectorUsb.SendLine("Ramping to speed...");
	while (!motor.StatusReg().bit.AtTargetVelocity) {
		continue;
	}

	//ConnectorUsb.SendLine("At Speed");
	return true;
}