import dash
import dash_daq as daq
import dash_core_components as dcc
import dash_html_components as html
import paho.mqtt.client as mqtt
import json
import time
import threading
from flask import Flask
import logging

# DASH/PLOTLY based web frontend to test MQTT access/control to a DEMESH

# DISCLAIMER 1: this is for development purposes only. There are a number
# of reasons, for which dash/plotly does not really our usecase here.
# Formost, we want to control (in contrast to inspect) the target nodes and
# this effectively amounts to a global state ... which by concept dash/plotly
# does not support.

# DISCLAIMER 2: this file is obsolete and will be removed in due course.



# Run with redirected output --- we have some logging issues
# python dmdash.py > /dev/null

# HTTP server runs on http://localhost:8050/ about 10sec after startup



# MQTT broker
MQTTHOST="192.168.2.108"
MQTTPORT=1884

# turn of server logging
app = Flask(__name__)
log = logging.getLogger('werkzeug')
log.disabled = True

# dash default
external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']

# keep track of devices
devdict={}
devdict_lock=threading.Lock()


# class to manage one target uC
class Device:
    # minimal init
    def __init__(self, devid):
        print("instantiate device object for {}".format(devid))
        self.id = devid
        self.state = {}
        self.lastseen = time.time()
 
        
# list devices
def print_devices():
    now=time.time()
    print("=====")
    devdict_lock.acquire()
    for key, device in devdict.items():
        age=now-device.lastseen
        print("Device {0:s} -- Age {1:02.2f} -- State {2}".format(key,age,device.state))
    devdict_lock.release()
    print("=====")


# monitor target uCs via their heartbeat
def parse_heartbeat(hbmsg):
    id=hbmsg['dev']
    print("parsing heartbeat from device {0} (lock {1})".format(id,devdict_lock.locked()))
    devdict_lock.acquire()
    # discover new devices
    if id not in devdict:
        print("found new device {}".format(id))
        devdict[id]=Device(id)
    # update state and timestamp    
    device=devdict[id]
    now=time.time()
    device.lastseen=now
    hbmsg.pop('dev')
    device.state=hbmsg
    # check for timeout
    for key, device in list(devdict.items()): 
        if now-device.lastseen > 20:
            print("removing stale device {}".format(device.id))            
            devdict.pop(key)
    # report        
    devdict_lock.release()
    print("parsing heartbeat --- lock released")
    #print_devices()
 
mqttclient=mqtt.Client("dmdash",clean_session=False,protocol=mqtt.MQTTv311)
def on_message(client, userdata, message):
    #print("message received " ,str(message.payload.decode("utf-8")))
    #print("message topic=",message.topic)
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)
    hbmsg=json.loads(str(message.payload.decode("utf-8")))
    parse_heartbeat(hbmsg)
mqttclient.on_message=on_message  
def on_connect(client, userdata, flags, rc):
    if rc==0:
        mqttclient.subscribe("/DEMESH/+/heartbeat")
        print("mqtt connected/subscribed")
    else:
        print("connection failed Returned code=",rc)
mqttclient.on_connect=on_connect      
mqttclient.connect(MQTTHOST, port=MQTTPORT, keepalive=60)
mqttclient.loop_start() 

print("wait for heartbeat")    
time.sleep(10)
print_devices()





app = dash.Dash(__name__, external_stylesheets=external_stylesheets,update_title = None)
app.title = 'DmDash'


def device_layout(id):
    return  html.Div(className='dmDev1', children= [
        html.H1('Device {}'.format(id)),
        html.Div(className='dmDev2', style={"display":"flex", "align-items":"center"}, children= [
            html.Div(className='dmDev3', style={"display":"vertical", "align-items":"center"}, children= [
                daq.GraduatedBar(
                    id="cur1-{}".format(id),
                    label="I1",
                    min=0, max=32, step=1, value=0,
                    vertical=True,
                    size=100
                ),
                html.Div(className='dmSpace', style={"padding-top":"20px"}),       
                daq.BooleanSwitch(
                    id="phase1-{}".format(id),
                    on=False,
                    label="",
                    vertical=True
                ),
            ]),   
            html.Div(className='dmDev3', style={"display":"vertical", "align-items":"center"}, children= [
                daq.GraduatedBar(
                    id="cur2-{}".format(id),
                    label="I1",
                    min=0, max=32, step=1, value=0,
                    vertical=True,
                    size=100
                ),
                html.Div(className='dmSpace', style={"padding-top":"20px"}),       
                daq.BooleanSwitch(
                    id="phase2-{}".format(id),
                    on=False,
                    label="",
                    vertical=True
                ),
            ]),   
            html.Div(className='dmDev3', style={"display":"vertical", "align-items":"center"}, children= [
                daq.GraduatedBar(
                    id="cur3-{}".format(id),
                    label="I1",
                    min=0, max=32, step=1, value=0,
                    vertical=True,
                    size=100
                ),
                html.Div(className='dmSpace', style={"padding-top":"20px"}),       
                daq.BooleanSwitch(
                    id="phase3-{}".format(id),
                    on=False,
                    label="",
                    vertical=True
                ),
            ]),   
            html.Div(className='dmSpace', style={"padding-left":"100px"}),       
            daq.LEDDisplay(
                id="power-tx-{}".format(id),
                label="Power",
                size=80,
                value=0
            ),
            html.Div(className='dmSpace', style={"padding-left":"100px"}),       
            dcc.Slider(
                id="maxcur-{}".format(id),
                min=0, max=32, step=1, value=0,
                marks={'0': '0', '10': '10','20': '20', '30': '30'},
                vertical=True,
                verticalHeight=160
            ),
            html.Div(className='dmSpace', style={"padding-left":"100px"}),       
            daq.LEDDisplay(
                id="maxpow-tx-{}".format(id),
                label="Max Power",
                size=80,
                value=0
            ),
        ])
     ])




def core_layout():
    res=[]
    for key, device in devdict.items():
        res.append(device_layout(device.id))
    return html.Div(res)                   



app.layout = html.Div([
    html.H1('DEMESH Dashboard'),

    html.Div('''
        dmdash.py: a dash based web app to control a demesh
    '''),

    html.Div(core_layout()),
    
    dcc.Interval(
        id='interval-500',
        interval=1000, # in milliseconds
        n_intervals=0
    )    
])    



def device_callbacks(id):

    @app.callback(
        dash.dependencies.Output("maxpow-tx-{}".format(id), 'value'),
        [
          dash.dependencies.Input("maxcur-{}".format(id), 'value'),
          dash.dependencies.Input("phase1-{}".format(id), 'on'),
          dash.dependencies.Input("phase2-{}".format(id), 'on'),
          dash.dependencies.Input("phase3-{}".format(id), 'on'),
          dash.dependencies.Input("interval-500", 'n_intervals')
        ]
    )
    def update_maxcur(maxcur,phase1,phase2,phase3,n):
        print("update maxpower-{0} ({1},{2},{3},{4})".format(id,maxcur,phase1,phase2,phase3))
        maxcur=maxcur*10
        phases=0
        phcnt=0
        if phase1:
            phases=10*phases+1
            phcnt=phcnt+1
        if phase2:
            phases=10*phases+2
            phcnt=phcnt+1
        if phase3:
            phases=10*phases+3
            phcnt=phcnt+1
        mqttclient.publish("/DEMESH/{0:s}/control".format(id),'{{"cmd":"avrsetpar","avrpar":"phases","avrval":{0:d}}}'
.format(phases))
        mqttclient.publish("/DEMESH/{0:s}/control".format(id),'{{"cmd":"avrsetpar","avrpar":"maxcur","avrval":{0:d}}}'.format(maxcur))
        maxpower = maxcur/10.0*phcnt*230/1000.0
        print("update maxpower-{0}: {1:02.1f}".format(id,maxpower))
        return "{0:04.1f}".format(maxpower)

   
    @app.callback(
        dash.dependencies.Output("cur1-{}".format(id), 'value'),
        [dash.dependencies.Input("interval-500", 'n_intervals')]
    )
    def update_cur1(n):
        print("update cur1-{0} (lock {1})".format(id,devdict_lock.locked()))
        devdict_lock.acquire()
        device=devdict[id]
        cur=device.state['cur1']/10.0
        devdict_lock.release()
        print("update cur1-{0}: {1} (lock released)".format(id,cur))
        return cur

    @app.callback(
        dash.dependencies.Output("cur2-{}".format(id), 'value'),
        [dash.dependencies.Input("interval-500", 'n_intervals')]
    )
    def update_cur2(n):
        print("update cur2-{0} (lock {1})".format(id,devdict_lock.locked()))
        devdict_lock.acquire()
        device=devdict[id]
        cur=device.state['cur2']/10.0
        devdict_lock.release()
        print("update cur2-{0}: {1} (lock released)".format(id,cur))
        return cur

    @app.callback(
        dash.dependencies.Output("cur3-{}".format(id), 'value'),
        [dash.dependencies.Input("interval-500", 'n_intervals')]
    )
    def update_cur3(n):
        print("update cur3-{0} (lock {1})".format(id,devdict_lock.locked()))
        devdict_lock.acquire()
        device=devdict[id]
        cur=device.state['cur3']/10.0
        devdict_lock.release()
        print("update cur3-{0}: {1} (lock released)".format(id,cur))
        return cur

    @app.callback(
        dash.dependencies.Output("power-tx-{}".format(id), 'value'),
        [dash.dependencies.Input("interval-500", 'n_intervals')]
    )
    def update_powertx(n):
        #print("update power for {}".format(id))
        devdict_lock.acquire()
        device=devdict[id]
        pow=(device.state['cur1']/10.0+device.state['cur2']/10.0+device.state['cur3']/10.0)*230 / 1000.0
        devdict_lock.release()    
        return "{0:04.1f}".format(pow)
     


for key, device in devdict.items():
    device_callbacks(device.id)

    
app.run_server(debug=False)

