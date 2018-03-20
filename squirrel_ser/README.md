<a id="top"/> 
# squirrel_ser

This folder has source codes for deep convolutional neural network-based speech emotion recognition. Note that this module relies on many machine learning packages and platforms such as Google Tensorflow and Keras, which is comptutationally expensive without GPU supports. Hence, it may not be operationable on the robot, rather deployment on an external machine is recommended. This module requires two trained models: keras-model and elm-model. Keras-model is either convolutional DNN or convolutional LSTM, which output frame-level prediction. If elm-model is given, it proceeds the outputs of keras-model to predict utterance-level prediction. (The latter gives better results so far). Performance varies on speakers and environment.

Maintainer: [**batikim09**](https://github.com/**github-user**/) (**batikim09**) - **j.kim@utwente.nl**

##Contents
1. <a href="#1--installation-requirements">Installation Requirements</a>

2. <a href="#2--build">Build</a>

3. <a href="#3--device">Device setup</a>

4. <a href="#4--usage">Usage</a>

4. <a href="#5--references">References</a>

## 1. Installation Requirements <a id="1--installation-requirements"/>
####Debian packages

Please run the following steps BEFORE you run catkin_make.

`sudo apt-get install python-pip python-dev libhdf5-dev portaudio19-dev'

Next, using pip, install all pre-required modules.
(pip version >= 8.1 is required.)

http://askubuntu.com/questions/712339/how-to-upgrade-pip-to-latest

If you have old numpy (<1.12) please remove it.
https://github.com/tensorflow/tensorflow/issues/559

Then,
sudo pip install -r requirements.txt

If you have numpy already, it must be higher than 1.12.0
try to install tensorflow using pip, then it will install the proper version of numpy.

## 2. Build <a id="2--build"/>

Please use catkin_make to build this.

## 3. Device setup <a id="3--device"/>
Currently, using pulse audio as the input device is the best stable way. If you do not specify device ID, a pulse audio device will be chosen as an input. However, you must make sure if pulseaudio server is running. (if not, type "pulseaudio --start").

If you want and know pulse & alsa works, you can choose your own input device as a pulse audio and use the pulse as the input device for emotion recognition as follows:

0. turn on pulseaudio server if it's off

pulseaudio --start

To run a demon, instead,

pulseaudio -D

1. find your device by:
See: https://wiki.archlinux.org/index.php/PulseAudio/Examples

pacmd list-sources | grep -e device.string -e 'name:'

Depending on os and devices, it gives you various names. You need to choose a right input device among them.

2. set a default device for "pulse" by typing in a terminal for example:

pacmd "set-default-source alsa_input.usb-Andrea_Electronics_Corp._Andrea_Stereo_USB_Mic-00-Mic.analog-stereo"

3. check it works:

pacmd stat

4. check your "pulse" device's ID in pulseaudio:

rosrun squirrel_ser ser.py

This will give you a list of audio devices and you need to identify index of "pulse".
Note that this index changes depending on usb devices being used. Hence, it's safe to check before it runs.

5. set the ID of "pulse" in the launch file: ser.launch
by the argument: -d_id 

## 4. Usage <a id="4--usage"/>

For a quick start, run in the terminal:

roslunach squirrel_ser ser.launch

For a visualisation,

rosluanch squirrel_ser viz.launch

To get information of parameters, 

rosrun squirrel_ser ser.py

usage: --default [-h] [-sr SAMPLE_RATE] [-fd FRAME_DURATION] [-vm VAD_MODE]
                 [-vd VAD_DURATION] [-me MIN_ENERGY] [-d_id DEVICE_ID]
                 [-g_min G_MIN] [-g_max G_MAX] [-fp FEAT_PATH]
                 [-md MODEL_FILE] [-elm_md ELM_MODEL_FILE]
                 [-c_len CONTEXT_LEN] [-m_t_step MAX_TIME_STEPS]
                 [-tasks TASKS] [--stl] [--reg] [-save SAVE] [--default]
                 [--name]

optional arguments:
  
  -h, --help            show this help message and exit
  
  -sr SAMPLE_RATE, --sample_rate SAMPLE_RATE
                        number of samples per sec, only accept
                        [8000|16000|32000]
  
  -fd FRAME_DURATION, --frame_duration FRAME_DURATION
                        a duration of a frame msec, only accept [10|20|30]
  
  -vm VAD_MODE, --vad_mode VAD_MODE
                        vad mode, only accept [0|1|2|3], 0 more quiet 3 more
                        noisy
  
  -vd VAD_DURATION, --vad_duration VAD_DURATION
                        minimum length(ms) of speech for emotion detection
  
  -me MIN_ENERGY, --min_energy MIN_ENERGY
                        minimum energy of speech for emotion detection
  
  -d_id DEVICE_ID, --device_id DEVICE_ID
                        device id for microphone:
                        PLEASE use "pulse" and set your device as a default sink for "pulse", see 2. device setup:

  -g_min G_MIN, --gain_min G_MIN
                        min value of automatic gain normalisation
  
  -g_max G_MAX, --gain_max G_MAX
                        max value of automatic gain normalisation
  
  -fp FEAT_PATH, --feat_path FEAT_PATH
                        temporay feat path
  
  -md MODEL_FILE, --model_file MODEL_FILE
                        keras model path
  
  -elm_md ELM_MODEL_FILE, --elm_model_file ELM_MODEL_FILE
                        elm model_file
  
  -c_len CONTEXT_LEN, --context_len CONTEXT_LEN
                        context window's length
  
  -m_t_step MAX_TIME_STEPS, --max_time_steps MAX_TIME_STEPS
                        maximum time steps for DNN
  
  -tasks TASKS, --tasks TASKS
                        tasks (arousal:2,valence:2)
  
  --reg                 regression mode
  
  --save SAVE, --save SAVE
                        save directory
  
  --default             default
  
  --name                name
  
  
To run voice detection with default parameters,

rosrun squirrel_ser ser.py --default

(in this case, it performs only voice detection but not emotion detection since models are not specified.)

To see a published topic,

rostopic echo /arousal

rostopic echo /valence

ROS Messages are defined in :

squirrel_common/squirrel_vad_msgs/RecognisedResult.msg

To see visualisation (MarkerArray), set fixed frame as 'sound', first and add the marker by topic name.

<a href="#top">top</a>
