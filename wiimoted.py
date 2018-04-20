#!/usr/bin/python
#
# Demonstrates the use of the wiimote using 5 status leds.
#

import cwiid
import time
import sys
import subprocess

import RPi.GPIO as GPIO

import sysv_ipc

GPIO.setmode(GPIO.BCM)
GPIO.setup([21,20,16,12,1], GPIO.OUT, initial=GPIO.LOW)
GPIO.output(21, 1)

# Our wiimote address.
addr = "00:1c:BE:22:32:A0"

# Set bluetooth timeout in ms
BLUETOOTH_TIMEOUT = 100

def main():
        count = 599
	led = 0
	rpt_mode = 0
	rumble = 0
	mesg = False

	#Connect to address given on command-line, if present
	#print 'Put Wiimote in discoverable mode now (press 1+2)...'
	global wiimote
        try:
            wiimote = cwiid.Wiimote(addr)
        except RuntimeError:
            #print "Could not connect wiimote"
            # Stop moving motors here or switch to autopilot.
            return
        
        # Switch to manual.
        Q.send(chr(18), type=1)

        GPIO.output(21, 0)
        
        wiimote.rumble = 1
        time.sleep(.2)
        wiimote.rumble = 0

        # Set the timeout of the wiimote. 
        # This is important so that the device immediatley knows when to go to auto-pilot.
        subprocess.call(("hcitool","lst",addr,str(BLUETOOTH_TIMEOUT*16/10)))


	# This function will handle messages received from wiimote.
        wiimote.mesg_callback = callback
        # Tell the wiimote to report button pushes. 
        wiimote.rpt_mode = cwiid.RPT_BTN
        # This enables the messages to be sent.
        wiimote.enable(cwiid.FLAG_MESG_IFC);
        
        while wiimote:
            # Constantly poll wiimote to make sure it is connected
            # Show battery life using LEDs, update every minute.
            time.sleep(.1)
            count += 1
            try:
                wiimote.request_status()
                
                if count == 600:
                    count = 0
                    led = 0
                    batt  = int(100.0 * wiimote.state['battery'] / cwiid.BATTERY_MAX)
                    #print 'Battery:', batt
                    if batt > 0:
                        led ^= cwiid.LED1_ON
                    if batt > 25:
                        led ^= cwiid.LED2_ON
                    if batt > 50:
                        led ^= cwiid.LED3_ON
                    if batt > 75:
                        led ^= cwiid.LED4_ON
                    wiimote.led = led

            except ValueError:
                # Wiimote object closed on its own.
                GPIO.output(20, 0)
                GPIO.output(16, 0)
                GPIO.output(12, 0)
                GPIO.output(1,  0)
                return
            except RuntimeError:
                #print "Disconnected suddenly"
                Q.send(chr(10), type=1)
                Q.send(chr(19), type=1)
                GPIO.output(20, 0)
                GPIO.output(16, 0)
                GPIO.output(12, 0)
                GPIO.output(1,  0)
                # Stop moving motors here or switch to autopilot.
                return
	wiimote.close()


def callback(mesg_list, t):
	#print 'time: %f' % t
	for mesg in mesg_list:
		if mesg[0] == cwiid.MESG_BTN:
			#print 'Button Report: %.4X' % mesg[1]
                        #print 
                        if mesg[1] == cwiid.BTN_UP:
                            Q.send(chr(11), type=1)

                            GPIO.output(20, 1)
                            GPIO.output(16, 0)
                            GPIO.output(12, 0)
                            GPIO.output(1,  0)
                        elif mesg[1] == cwiid.BTN_DOWN:
                            Q.send(chr(12), type=1)

                            GPIO.output(20, 0)
                            GPIO.output(16, 1)
                            GPIO.output(12, 0)
                            GPIO.output(1,  0)
                        elif mesg[1] == cwiid.BTN_RIGHT:
                            Q.send(chr(13), type=1)
                            
                            GPIO.output(20, 0)
                            GPIO.output(16, 0)
                            GPIO.output(12, 0)
                            GPIO.output(1,  1)
                        elif mesg[1] == cwiid.BTN_LEFT:
                            Q.send(chr(14), type=1)
                            
                            GPIO.output(20, 0)
                            GPIO.output(16, 0)
                            GPIO.output(12, 1)
                            GPIO.output(1,  0)
                        elif mesg[1] == (cwiid.BTN_PLUS + cwiid.BTN_MINUS):
                            global wiimote
                            global disconnect
                            disconnect = True
                            wiimote.rumble = 1
                            time.sleep(.2)
                            wiimote.rumble = 0
                            #print "Disconnecting"
                            wiimote.close()
                            # Switch to autopilot
                            GPIO.output(21, 1)
                        
                        else:
                            Q.send(chr(10), type=1)
                            GPIO.output(20, 0)
                            GPIO.output(16, 0)
                            GPIO.output(12, 0)
                            GPIO.output(1,  0)
                            continue

		elif mesg[0] ==  cwiid.MESG_ERROR:
			#print "Error message received"
                        # Switch to autopilot
                        GPIO.output(21, 1)
		else:
			print 'Unknown Report'

# Global to check if graceful disconnect requires sleep.
disconnect = False

Q = sysv_ipc.MessageQueue(0x7dd)

while __name__ == '__main__':
    main()
    if disconnect:
        Q.send(chr(19), type=1)
        disconnect = False
    time.sleep(5)
