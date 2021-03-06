// Created on 2015-08-04 @author: Admir Resulaj
// Modified on 2016-06-24 @author: Pei-Ching Chang

// * This is an Arduino protocol implementing a passive odorant exposure
// * paradigm for an olfactory experiment


// include the library code:
#include <SPI.h>
#include <C:\Users\Gottfried_Lab\PycharmProjects\PyOlfa\src\arduino_controller\voyeur_timer_lib.pde>
#include <C:\Users\Gottfried_Lab\PycharmProjects\PyOlfa\src\arduino_controller\IoFunctions_external_timers.pde>
#include <C:\Users\Gottfried_Lab\PycharmProjects\PyOlfa\src\arduino_controller\Voyeur_serial_stream_tools.pde>

#define LED_PIN   13

#define TRIGGER1   4
#define TRIGGER2   5
#define TRIGGER3   6

#define SOLENOID1 29
#define SOLENOID2 28
#define SOLENOID3 27
#define SOLENOID4 26
#define SOLENOID5 25
#define SOLENOID6 24
#define SOLENOID7 23
#define SOLENOID8 22

#define BEAM1     37
#define BEAM2     36
#define BEAM3     35
#define BEAM4     34
#define BEAM5     33
#define BEAM6     32
#define BEAM7     31
#define BEAM8     30

#define CUE1      45
#define CUE2      44
#define CUE3      43
#define CUE4      42
#define CUE5      41
#define CUE6      40
#define CUE7      39
#define CUE8      38

#define ADC_PIN   49
#define DAC1_PIN  53
#define DAC2_PIN  48

#define DIGITAL1  62
#define DIGITAL2  63
#define DIGITAL3  64
#define DIGITAL4  65
#define DIGITAL5  66
#define DIGITAL6  67
#define DIGITAL7  68
#define DIGITAL8  69
#define DIGITAL9  54
#define DIGITAL10 55
#define DIGITAL11 56
#define DIGITAL12 57
#define DIGITAL13 58
#define DIGITAL14 59
#define DIGITAL15 60
#define DIGITAL16 61

// Valves attached to the behaviour box and their solenoid channels.
#define WATERVALVE1 SOLENOID2
#define WATERVALVE2 SOLENOID7
#define FINALVALVE SOLENOID1
#define FINALVALVE2 SOLENOID8
#define FV_T DIGITAL8
#define FAKE_MRI DIGITAL6
#define MRI_TRIGGER DIGITAL7
#define LICK1_SENSOR DIGITAL15
#define LICK2_SENSOR DIGITAL16

// Phases of the sniff signal and the codes for each phases.
#define INHALATION 0
#define EXHALATION 1
#define PHASE_INDEPENDENT 2

// Trial categary labels and codes
#define RIGHT 0
#define LEFT 1

//=======================
// Set the protocol name.
char protocolName[] = "Passive_exposure_2AFC"; // should be less than 20 characters
//=======================

//==============================================================================
// Trial input parameters.
//==============================================================================
unsigned long final_valve_duration = 0;
unsigned long final_valve_duration2 = 0;
unsigned long response_window = 0;
unsigned long inter_trial_interval = 0;
// Sniff phase for starting the final valve onset time.
unsigned long odorant_trigger_phase = 0;// 0 = inhalation, 1 = exhalation, 2 = phase_independent
unsigned long trial_type = 0;
unsigned long lick_grace_period = 0;
unsigned long tr = 0;
unsigned long reward_delay = 0;
unsigned long licking_training = 0;
boolean left_free_water = false;
boolean right_free_water = false;

// Water reward durations in milliseconds. Not used in this particular paradigm.
unsigned long water_duration1 = 0;
unsigned long water_duration2 = 0;

//==============================================================================
// Event to be transmitted back. These are the trial result variables.
//==============================================================================
unsigned long parameters_received_time = 0;
unsigned long trial_start = 0;
unsigned long trial_end = 0;
unsigned long final_valve_onset = 0;
unsigned long response = 0;
unsigned long first_lick = 0;

//===================
// Internal variables
//===================
// Counter for received parameters at the start of a trial.
short received_parameters = 0;
// Flags used for internal states.
boolean trial_done_flag = false, send_last_packet = false;
unsigned long mri_trigger_state = false;

// For generating a period pulse
unsigned long last_pulse = 0;   // time of last pulse change
boolean pulse_on = false;  // change this to true to test with fake signal
int pulse_state = false;

// State of the state machine.
int state = 0;
// The command code sent from the master (python).
int code = 0;

// Indices to keep track when transmitting the sniffing data in the sniff buffer
int last_sent_sniff_data_index = -1, current_sniff_data_index = 0;

// Buffer to use for receiving special user-defined ASCII commands.
char user_command_buffer[128];
// Number of words (argument variables) in a user command line.
char *argument_words[8];
// Index to use when filling the user command buffer.
uint8_t user_command_buffer_index = 0;

// temporary variables used to keep track of time and time windows.
unsigned long trial_now = 0;
unsigned long inter_water_interval = 2000;

// The chance of free water is given to mice during licking training
unsigned long random_chance = 0;

// Wait for 5 TR to start the experiment
int TR_counts = 0;
boolean on_hold = true;
boolean TR_status = false;

int trial = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT); // Green LED on the front
  pinMode(TRIGGER1, OUTPUT); // Pulse generator ch. 1 trigger
  pinMode(TRIGGER2, OUTPUT); // Pulse generator ch. 2 trigger
  pinMode(SOLENOID1, OUTPUT);
  pinMode(SOLENOID2, OUTPUT);
  pinMode(SOLENOID3, OUTPUT);
  pinMode(SOLENOID4, OUTPUT);
  pinMode(SOLENOID5, OUTPUT);
  pinMode(SOLENOID6, OUTPUT);
  pinMode(SOLENOID7, OUTPUT);
  pinMode(SOLENOID8, OUTPUT);
  pinMode(DIGITAL1, OUTPUT);
  pinMode(DIGITAL2, OUTPUT);
  pinMode(DIGITAL3, OUTPUT);
  pinMode(DIGITAL4, OUTPUT);
  pinMode(DIGITAL5, OUTPUT);
  pinMode(FAKE_MRI, OUTPUT);
  pinMode(MRI_TRIGGER, INPUT);
  pinMode(FV_T, OUTPUT);
  pinMode(DIGITAL9, OUTPUT);
  pinMode(DIGITAL10, OUTPUT);
  pinMode(DIGITAL11, OUTPUT);
  pinMode(DIGITAL12, OUTPUT);
  pinMode(DIGITAL13, OUTPUT);
  pinMode(DIGITAL14, OUTPUT);
  pinMode(LICK1_SENSOR, INPUT);
  pinMode(LICK2_SENSOR, INPUT);
  pinMode(CUE1, OUTPUT);
  pinMode(CUE2, OUTPUT);
  pinMode(CUE3, OUTPUT);
  pinMode(CUE4, OUTPUT);
  pinMode(CUE5, OUTPUT);
  pinMode(CUE6, OUTPUT);
  pinMode(CUE7, OUTPUT);
  pinMode(CUE8, OUTPUT);
  pinMode(ADC_PIN, OUTPUT);
  pinMode(DAC1_PIN, OUTPUT);
  pinMode(DAC2_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(TRIGGER1, LOW);
  digitalWrite(TRIGGER2, LOW);
  digitalWrite(SOLENOID1, LOW);
  digitalWrite(SOLENOID2, LOW);
  digitalWrite(SOLENOID3, LOW);
  digitalWrite(SOLENOID4, LOW);
  digitalWrite(SOLENOID5, LOW);
  digitalWrite(SOLENOID6, LOW);
  digitalWrite(SOLENOID7, LOW);
  digitalWrite(SOLENOID8, LOW);
  digitalWrite(DIGITAL1, LOW);
  digitalWrite(DIGITAL2, LOW);
  digitalWrite(DIGITAL3, LOW);
  digitalWrite(DIGITAL4, LOW);
	digitalWrite(DIGITAL5,LOW);
//	digitalWrite(DIGITAL6,LOW);
//  digitalWrite(DIGITAL7,LOW);
//  digitalWrite(DIGITAL8, LOW);
  digitalWrite(DIGITAL9, LOW);
  digitalWrite(DIGITAL10, LOW);
  digitalWrite(DIGITAL11, LOW);
  digitalWrite(DIGITAL12, LOW);
  digitalWrite(DIGITAL13, LOW);
  digitalWrite(DIGITAL14, LOW);
//  digitalWrite(DIGITAL15,LOW);
//  digitalWrite(DIGITAL16,LOW);
  digitalWrite(CUE1, LOW);
  digitalWrite(CUE2, LOW);
  digitalWrite(CUE3, LOW);
  digitalWrite(CUE4, LOW);
  digitalWrite(CUE5, LOW);
  digitalWrite(CUE6, LOW);
  digitalWrite(CUE7, LOW);
  digitalWrite(CUE8, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(ADC_PIN, HIGH);
  digitalWrite(DAC1_PIN, HIGH);
  digitalWrite(DAC2_PIN, HIGH);

  //=======================
  // prep ANALOG inputs
  analogReference(DEFAULT);
  //=======================

  //=======================
  // prep SPI for AD control
  startSPI();
  //=======================

  //=======================
  // initialize the SERIAL communication
  Serial.begin(115200);
  //  Serial.println(F("* Monitor System ready *"));
  //=======================

  //=======================
  // initialize SERIAL for LCD
  Serial1.begin(19200);
  Serial1.write(0x0c); // clear the display
  delay(10);
  Serial1.write(0x11); // Back-light on
  Serial1.write(0x80); // col 0, row 0
  Serial1.print(F("Passive exposure"));

  //=======================

  // Pulse generator communication
  Serial2.begin(115200);
  // Setup and start ms timer from: voyeur_timer_lib.pde .
  // This defines totalms.
  setupVoyeurTimer();

  // setup buffer sizes
  //(unsigned unsigned int sniffbuff, unsigned int mribuff, unsigned int lickbuff)
  setupBuffers(128, 32, 4, 4);

  // Recording of sniff
  // The first two arguments define the two analog pins that we are recording from: ex. 0 = AIN1, 1=AIN2
  // The third argument indicates which channel to actually record data from: 0 = no channel, 1 = first channel
  setupAnalog(0, 1);

  // start analog acquisition. Every millisecond go and fetch a value from the sniff sensor into the sniff buffer.
  start_analog_timer(); //(ioFunctions).
  // start digital recording timer. Creates a buffer to store the event timestamps and defines which pin to sample from.
  // Argument 1 is channel 1 (first lick port), argument 2 is channel 2.
  startDigital(LICK1_SENSOR, LICK2_SENSOR, MRI_TRIGGER);

  // first argument is how many digital channels to look for licking. 1 = only first channel. 2 = only second channel, 3 = both.
  // function is found at ioFunctions.pde . Second argument is the inter sample interval 1 = every ms
  digitalOn(3, 1);
  recordsniffttl = true;

  // Init done
  cueOff(LED_PIN);
}

//==============================================================================
void loop() {

  //==========================================================================
  //                       State machine goes here
  //==========================================================================
  
  digitalWrite(FAKE_MRI, HIGH);
  if (pulse_on) {
    digitalWrite(FAKE_MRI, HIGH);
    if (totalms - last_pulse > tr) {
      digitalWrite(FAKE_MRI, LOW);
      last_pulse = totalms ;
      delay(1);
    }
  }
 
  if (on_hold) {
     TR_status = hasDigitaled(3);
     if (TR_status = !hasDigitaled(3)) {
      on_hold = false;
     }
  }
 
  switch (state) {

    case 0: //waiting for initialization of trial
        break;

    case 1:
      // Python has uploaded the trial parameters to start a trial.
      // Wait for ITI to be over and wait for trigger before starting the trial.
      if (((totalms - trial_end) > (inter_trial_interval)) && (!on_hold)) {
        trial_start = totalms;
        // Final valve trigger state
        if (odorant_trigger_phase == EXHALATION) {
          state = 2;
        }
        else if (odorant_trigger_phase == INHALATION) {
          state = 4;
        }
        else if (odorant_trigger_phase == PHASE_INDEPENDENT) { 
          state = 6;
        }
      }
      break;

    case 2: // Wait for TR to be over and then exhalation state.
      if (hasDigitaled(3)) {
        state = 3;
      }
      
      break;

    case 3: // Find exhalation state so that the stimulus can be triggered.
      if (sniff_trigger) {
        state = 7;
      }
      break;

    case 4: // Wait for TR to be over and then inhalation state.
      if (hasDigitaled(3)) {
        state = 5;
      }
      break;

    case 5: // Find inhalation state so that the stimulus can be triggered.
      if (!sniff_trigger) {
        state = 7;
      }
      break;

    case 6: // Wait for TR to be over regardless of inhaleation/exhaleatio state
      if (hasDigitaled(3)) {
        state = 7;
      }
      break;
      
    case 7: // Trigger stimulus state.
      // open final valve
      final_valve_onset = totalms;
      valveOnTimer(FINALVALVE, final_valve_duration);
      valveOnTimer(FINALVALVE2, final_valve_duration2);
      valveOnTimer(FV_T, final_valve_duration);
      state = 8;
      break;

    case 8: // Grace period.
      if (totalms >= (final_valve_onset + lick_grace_period)) {
        trial = trial + 1;
        if (trial_type == LEFT) {
          if ((left_free_water)) {
              valveOnTimer(WATERVALVE1, water_duration1);
          }
          state = 9;
        }
        else if (trial_type == RIGHT) {
          if ((right_free_water)) {
              valveOnTimer(WATERVALVE2, water_duration2);
          }
          state = 10;
        }
      }
      break;

    case 9:  // LEFT Trial
      trial_now = totalms;
      if (hasDigitaled(1) || hasDigitaled(2)) { // lick has occured
        first_lick = totalms;
        if (hasDigitaled(1)) { // Left lick
          if ((first_lick - final_valve_onset) < response_window) {
            response = 1; // A LEFT HIT
            for (int x = 0; x < 1; x = x + 1) {
              valveOnTimer(WATERVALVE1, water_duration1);
            }
          }
          state = 11;
        }
        else if (hasDigitaled(2)) {
          response = 3; // A LEFT MISS
          state = 11;
        }
      }
      else if ((trial_now - final_valve_onset) > response_window) {
        response = 5; // A Left no response
        state = 11;
      }

      break;

    case 10:  //RIGHT Trial
      trial_now = totalms;
      if (hasDigitaled(1) || hasDigitaled(2)) { // lick has occured
        first_lick = totalms;
        if (hasDigitaled(2)) { // Right lick
          if ((first_lick - final_valve_onset) < response_window) {
            response = 2; // A RIGHT HIT
            for (int x = 0; x < 1; x = x + 1) {
              valveOnTimer(WATERVALVE2, water_duration2);
            }
          }
          state = 11;
        }
        else if (hasDigitaled(1)) {
          response = 4; // A RIGHT MISS
          state = 11;
        }
      }
      else if ((trial_now - final_valve_onset) > response_window) {
        response = 6; // A right no response
        state = 11;
      }

      break;

    case 11: // shut everything down.
      trial_end = totalms;
      trial_done_flag = true;
      state = 0;
      break;
  }


  //==========================================================================
  // CHECK SERIAL PORT
  //==========================================================================


  if (Serial.available() > 0) {

    code = Serial.read();

    switch (code) {
      case 86: // user defined command (i.e. lick)
        RunSerialCom(code);
        break;

      case 87:  // Streaming packet was requested. Send all the streaming data.
        RunSerialCom(code);
        break;

      case 88: // trail ended and the trial details were requested
        RunSerialCom(code);
        break;

      case 89: // stop execution. 89 = 'Y';
        state = 0;
        Serial.print(3);
        Serial.print(",");
        Serial.print("*");
        break;

      case 90: // Start trial state (i.e. need to read from the serial port)
        RunSerialCom(code);
        parameters_received_time = totalms;
        first_lick = 0;
        final_valve_onset = 0;
        state = 1;
        break;

      case 91: // Protocol name
        RunSerialCom(code);
        break;

    }
  }
}

