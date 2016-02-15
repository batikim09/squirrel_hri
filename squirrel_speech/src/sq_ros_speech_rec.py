#!/usr/bin/env python

from __future__ import print_function

import rospy
import speech_recognition as sr
import std_msgs.msg

import sys
import pyaudio

from subprocess import call
call(["pulseaudio", "--kill"])
call(["jack_control", "start"])

from squirrel_speech_msgs.msg import RecognizedSpeech


default_lang = "de"

arg_num = len(sys.argv)
arg_list = str(sys.argv)

if arg_num > 1:
    arg_lang = str(sys.argv[1])
else:
    arg_lang = default_lang

if arg_num > 2:
    dev_id = int(sys.argv[2])
else:
    dev_id = int(0)
 
if arg_num > 3:
    print("Too many Arguments!")

r = sr.Recognizer()
#m = sr.Microphone(0)
m = sr.Microphone(int(dev_id))


print("SQUIRREL SPEECH RECOGNITION -----------------------------------------------")
if arg_num == 1: 
	arg_lang = "de"

if arg_lang != "de" and arg_lang != "de-DE":
    if arg_lang != "en" and arg_lang != "en-GB" and arg_lang != "en-US": 
        if arg_lang != "nl" and arg_lang != "nl-NL":
            print("Language not supported... (*might work anyway - see google speech language list)")
	
print("Used Language: " + arg_lang) 
print("Google API") 
print(" ") 
print("---------------------------------------------------------------------------") 

def recognizer():
    pub = rospy.Publisher('squirrel_speech_recognized_speech', RecognizedSpeech, queue_size=5) 
    msg = RecognizedSpeech()
   
    rospy.init_node('squirrel_speech_recognizer', anonymous=True)

    with m as source:
        r.adjust_for_ambient_noise(source)
        print("Set minimum energy threshold to {}".format(r.energy_threshold))

        print("---------------------------------------------------------------------------") 

        while not rospy.is_shutdown():
            # LOOP -----------------------------------------------------------------------
            try:

            
                print("Ready!")
                audio = r.listen(source)
                print("Recognition...")
            
                #value_str = r.recognize_google(audio,None,"de")
                value = r.recognize_google(audio,None, arg_lang)
                value = value.encode("utf-8")
                #value = unicode(value_str,"utf-8")
                #value = value.encode("utf-8")
                value = value.lower()

                msg.recognized_speech = value
                msg.is_recognized = True
                msg.speaker_ID = 0
            
                he = std_msgs.msg.Header()
                he.stamp = rospy.Time.now()
                msg.header = he



                print("\033[0;32m", end="")  #green
                rospy.loginfo(msg)
                print("\033[0;39m", end="")    #default

                pub.publish(msg)


            except sr.UnknownValueError:
                msg.recognized_speech = ""
                msg.is_recognized = False
                msg.speaker_ID = 0
            
                he = std_msgs.msg.Header()
                he.stamp = rospy.Time.now()
                msg.header = he

                print("\033[0;31m", end="")  #red
                print("ERROR: Unrecognizable", end="")
                rospy.loginfo(msg)
                print("\033[0;39m")   #default

                pub.publish(msg)
                


            except sr.RequestError as e:
                print("\033[0;31m", end="")  #red
                print("ERROR: Couldn't request results from Google Speech")
                print("Recognition service - {0}".format(e))
                print("\033[0;39m")   #default



if __name__ == '__main__':
    try:
        recognizer()
    except rospy.ROSInterruptException:
        pass



