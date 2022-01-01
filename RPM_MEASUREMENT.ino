/*
 * Author : VAIBHAV.
 * DATE : 1 JAN 2022.
 * PURPOSE : 1. SPEED CONTROL USING PWM.
 *           2. MEASURE A SPEED IN "Hz" and "RPM"
 */
#define PULSE_IN_PIN 8  // Sensor PIN
#define PWM_PIN 11      // Motor Pin
#define TIME_TO_ONE_TICK  64e-6  // (16e16/1024)^-1  Time Required to increment counter by one
#include<avr/interrupt.h>


long unsigned int t_p1 = 0;  // Variable to store a capture timestamp of pulse_1
long unsigned int t_p2 = 0;  // Varible to store a capture timestamp of pulse_2
long unsigned int t_diff = 0; // Varible to store a difference between two timestamp
double t_ms = 0.00; // varible to store a final value
double freq = 0.00; // varible to store frequency
float RPM = 0.00; // varible to store RPM

bool pulse_1_occured = false;
bool pulse_2_occured = false;

unsigned int no_of_cycle = 0;
unsigned char i =0;



void setup()
{
 Serial.begin(9600);
  // Initialization of Timer/Counter1 in "Normal Mode" 
  cli();
  TCCR1A= 0x00; // Initialize timer1 in normal mode operation free running mode 0x0000 to 0xffff
  //TCCR1B = 0x05; // prescalar to 1024
  TCCR1B = 0x85; // Enable Noice Cancel on ICP1, Falling  EDGE Triggering, Prescalar = 1024
  TCCR1C = 0x00;
  TCNT1 = 0x0000; // initilize timer1 with 0x0000
  TIMSK1 = 0x21;  // Unmask timer1 overflow interrupt and ICP event interrupt
  pinMode(PULSE_IN_PIN,INPUT_PULLUP);
  pinMode(PULSE_IN_PIN,INPUT);
 
   // Initialization of Timer/Counter2 in "Phase Correct PWM" Mode  
   TCCR2A = 0x81; 
   TCCR2B = 0x02;
   TCNT2 = 0x00;
   pinMode(PWM_PIN,OUTPUT);
   OCR2A = 142;
  sei();
}

void loop()
{
  // if both pulse occurs then procede for calculation
  if( pulse_1_occured && pulse_2_occured )
    {
      cli(); // clear global interrupt flag while computation of time period.
      
      // If difference between consequtive pulse greater than "4 Sec" (0xFFFF*64e-6). 
      if( no_of_cycle > 0)
        t_diff = ( (0xFFFF - t_p1) + ( 0xFFFF*(no_of_cycle - 1) )  + t_p2  );
      else
        t_diff = (t_p2 - t_p1);

      // Convert Seconds to milliSeconds
      t_ms = (t_diff * TIME_TO_ONE_TICK) * 1000;
      Serial.print("Difference is:");
      Serial.print(t_ms);
      Serial.print(" msec");

      // Calculate Frequency
      freq = 1/(t_ms/1000);
      Serial.print(" | Frequency :");
      Serial.print(freq);
      Serial.print(" Hz");

      // Calculate RPM
      RPM = freq*60;
      Serial.print(" | RPM :");
      Serial.println(RPM);
      
      // Reset Values
      no_of_cycle = 0;
      t_diff = 0;
      t_ms = 0;
      pulse_1_occured = false;
      pulse_2_occured = false;
      sei();
    }

  rpm_control(); // to set duty cycle
}


void rpm_control()
{
 OCR2A = 255;
}

// ISR for counting number of overflow
ISR(TIMER1_OVF_vect)
{
  no_of_cycle++;
}

// ISR to capture a Timer/Counter1 value at the instance of event.
ISR(TIMER1_CAPT_vect)
{
  
  if( ! pulse_1_occured )
   {
    pulse_1_occured = true;
    t_p1 = ICR1;
    no_of_cycle=0;
   }
  else 
   {
    pulse_2_occured = true;
    t_p2 = ICR1;
   }
}
