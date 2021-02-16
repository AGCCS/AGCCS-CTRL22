#!/usr/bin/python3
#
# simple python script to control/test/demo demesh, tmoor 2020-2021.

# imports
import sys
import time
import socket
import socketserver
import threading
import http.server
import time
import base64
import zlib
import json


# interface/port to listen for demesh root to connect
HOST, PORT = "0.0.0.0", 8070

# interface/port to run http server for firmware updates
FHOST, FPORT = "0.0.0.0", 8071

# command to run
COMMAND=""
AVRIMAGE=""
NODE=""


# usage
def usage():
    print("usage:")
    print("  dmctrl                                // broadcast status request")
    print("  dmctrl <CMD>                          // broadcast <CMD>, i.e., send {\"dst\":\"*\",\"cmd\":\"<CMD>\"}") 
    print("  dmctrl <JSON>                         // specify verbatim JSON message to be sent")
    print("  dmctrl upgrade <VER> <BRD>            // disribute ESP firmware version/board as specified")
    print("  dmctrl avrflash <FILE> <NODE>         // flash avr image for target uC (obmit NODE to broadcast)")
    print("  dmctrl avrgetpar <PAR> <NODE>         // get parameter <PAR> in target uC")
    print("  dmctrl avrsetpar <PAR> <VALUE> <NODE> // set parameter <PAR> in client uC to specified value")
    print("  dmctrl monitor                        // monitor heartbeat of target uCs")
    


# shutdown server event
shutdown_evt = threading.Event()

# monitor target uCs via their heartbeat
TARGETDICT={}

def print_targets():
    now=time.time()
    print("=====")
    for k, v in TARGETDICT.items():
        age=now-v['date']
        print("Node {0:s} -- Age {1:02.2f} -- State {2}".format(k,age,v['state']))
    print("=====")

def parse_heartbeat(hbmsg):
    src=hbmsg.pop('src')
    hbmsg.pop('mtype')
    # print("received heartbeat from {}".format(src))
    now=time.time()
    TARGETDICT[src]={'date':now, 'state':hbmsg}
    for k,v in list(TARGETDICT.items()): # learn python ...y
        age=now-v['date']
        if age>20:  TARGETDICT.pop(k)
    print_targets()
    

# tcp request handle to monitor target uCs
class MonitorTCPHandler(socketserver.BaseRequestHandler):
    # called on connection established
    def handle(self):
        print("{} connected, reading heartbeat".format(self.client_address[0]))
        self.request.settimeout(10)
        try:
            while True:
                reply = self.request.recv(1024).strip(b'\0x0 \n\r')
                jreply=json.loads(reply)
                if jreply['mtype'] == "heartbeat":
                    parse_heartbeat(jreply)
        except socket.error:
            print('timeout')
        print('shutting down demesh tcp server')
        shutdown_evt.set()
        

# tcp request handle for simple command, e.g. broadcast status
class CommandTCPHandler(socketserver.BaseRequestHandler):
    # called on connection established
    def handle(self):
        print("{} connected, sending command".format(self.client_address[0]))
        # say hello to dmesh
        self.request.sendall(COMMAND.encode(encoding="utf-8", errors="strict"))
        # read reply
        print("await for reply from {}".format(self.client_address[0]))
        self.request.settimeout(3)
        now=time.time()
        try:
            while True:
                reply = self.request.recv(1024).strip(b'\0x0 \n\r')
                jreply=json.loads(reply)
                if jreply['mtype'] != "heartbeat":
                    print(reply.decode("utf-8"))
                    now=time.time()
                if time.time()-now > 3:
                    break
        except socket.error:
            print('timeout')
        print('shutting down demesh tcp server')
        shutdown_evt.set()


# tcp request handle for avr firmware update (broadcast not yet implemented)
class AvrflashTCPHandler(socketserver.BaseRequestHandler):

    # send a JSON message and await JSON reply
    def rwnode(self,msg,rtype,tout):
        #print(">> {}".format(msg));
        # write to node
        self.request.sendall(msg.encode(encoding="utf-8", errors="strict"))
        # await reply        
        now=time.time()
        jreply=None
        try:
            while True:
                nout=tout-(time.time()-now)
                if nout < 0:
                    break
                self.request.settimeout(tout-(time.time()-now))
                reply = self.request.recv(1024).strip(b'\0x0 \n\r')
                jreply=json.loads(reply)
                if jreply['mtype'] == rtype:
                    break
                jreply=None
        except socket.error:
            print('timeout')
        #print("<< {}".format(reply))    
        return jreply    

    # called on connection established
    def handle(self):
        print("{} connected".format(self.client_address[0]))

        # set avrstate to receive firmware packages
        print("setting avrstate to receive firmware")
        cmd="{{\"dst\":\"{0:s}\",\"cmd\":\"avrota\",\"state\":\"recimg\"}}".format(NODE)
        jreply=self.rwnode(cmd,"avrota",3)
        jreply=json.loads(reply)
        ok = jreply is not None
        if ok:
          ok = jreply['state'] == "recimg"
        if not ok:
          print("setting otastate FAILED")
          shutdown_evt.set()
          return            

        # loop until all file read
        print("{} connected, starting avr firmware upload".format(self.client_address[0]))
        avrimgcnt=0;
        avraddr=0;
        while True:
            # read and encode block
            data = AVRIMAGE.read(128)
            if not data:
                break
            avrimgcnt=avrimgcnt+len(data)
            data64=base64.b64encode(data).decode()
            avrcrc = zlib.crc32(data)
            print("dmctrl: downloading packet: addr 0x{0:04x}".format(avraddr))
            # prepare command (note: ESPs JSON uses signed 32bit for integers)
            if avrcrc >= 2**31:
                avrcrc = avrcrc -2**32
            cmd="{{\"dst\":\"{0:s}\",\"cmd\":\"avrimg\",\"avraddr\":{1:d},\"avrdata\":\"{2:s}\",\"avrcrc\":{3:d}}}".format(NODE,avraddr,data64,avrcrc)
            # transmit to esp
            ok=False;
            retries=10;
            while not ok and retries>0:
                # write to node
                jreply=self.rwnode(cmd,"avrimg",10)
                if jreply is None:
                    break
                ok = jreply['avrcrc'] == "ok"
                retries=retries-1
            # report and proceed
            if not ok:
                print("dmctrl: downloading packet: addr 0x{0:04x}: FAILED (!)".format(avraddr))
                break;
            else:                    
                avraddr=avraddr+128
        # end upload loop

        # set avrota state to trigger flash procedure
        if ok:
            print("setting avrstate to trigger flash process (#{0:d} bytes)".format(avrimgcnt))
            cmd="{{\"dst\":\"{0:s}\",\"cmd\":\"avrota\",\"state\":\"flash\",\"avrimgcnt\":{1:d}}}".format(NODE,avrimgcnt)
            jreply=self.rwnode(cmd,"avrota",30)
            if jreply is not None:
                ok = jreply['state'] == "running"
                if not ok:
                    print("writing firmware to AVR flash FAILED")
                else:
                    print("writing firmware to target AVR succeeded")

        # done        
        print('shutting down demesh server')
        shutdown_evt.set()


# Run the server, binding to localhost
def run( ):

    print("demesh: starting demesh server on {0:s}:{1:d}".format(HOST,PORT))

    # run server in extra thread
    socketserver.ThreadingTCPServer.allow_reuse_address = True;
    if COMMAND=="avrflash":
        demesh_server = socketserver.ThreadingTCPServer((HOST, PORT), AvrflashTCPHandler)
    elif COMMAND=="monitor":
        demesh_server = socketserver.ThreadingTCPServer((HOST, PORT), MonitorTCPHandler)
    else:            
        demesh_server = socketserver.ThreadingTCPServer((HOST, PORT), CommandTCPHandler)       
    demesh_thread = threading.Thread(target=demesh_server.serve_forever)
    demesh_thread.daemon = True
    demesh_thread.start()
    shutdown_evt.wait()
    demesh_server.shutdown()

# configure  
socketserver.ThreadingTCPServer.allow_reuse_address = True;

# primitive commandline parsing
COMMAND=""
run_http=False
# default: broadcast mesh status request
if (COMMAND=="") and (len(sys.argv) == 1):
    COMMAND="{\"dst\":\"*\",\"cmd\":\"status\"}"
# help
if (COMMAND=="") and (len(sys.argv) == 2):
    if (sys.argv[1] == "?") or (sys.argv[1] == "-?") or (sys.argv[1] == "-h"): 
        usage()
        exit(0)
# monitor heartbeat
if (COMMAND=="") and (len(sys.argv) == 2) and (sys.argv[1] == "monitor"):
    COMMAND="monitor"
# verbatim JSON message or broadcast, e.g. "./dmctrl.py status"
if (COMMAND=="") and (len(sys.argv) == 2):
    COMMAND=sys.argv[1]
    if not COMMAND[0]=='{':
        COMMAND="{{\"dst\":\"*\",\"cmd\":\"{0:s}\"}}".format(COMMAND)
# upgrade ESP32 firmware
if (COMMAND=="") and (len(sys.argv) == 4):        
    if sys.argv[1] == "upgrade":
        ver = sys.argv[2]
        brd = sys.argv[3]
        if ver[0]=='v':
            ver=ver[1::];
        COMMAND="{{\"dst\":\"root\",\"cmd\":\"upgrade\",\"version\":\"{0:s}\",\"board\":\"{1:s}\"}}".format(ver,brd)
        run_http=True
# downstream AVR firmware        
if (COMMAND=="") and (len(sys.argv) >=3) and (len(sys.argv) <=4):
    if sys.argv[1] == "avrflash":
        COMMAND="avrflash";
        try:
            AVRIMAGE=open(sys.argv[2],"rb")
        except IOError:
            print("dmctrl: can not read avrimage from file \"{0:s}\"".format(sys.argv[2]))
            exit(-1)
        NODE="*"
        if len(sys.argv) == 4:
            NODE=sys.argv[3]
        if NODE=="*":
            print("dmctrl: broadcast of arvimage not implemented");
            exit(-1)
# set target AVR uC parmeter            
if (COMMAND=="") and (len(sys.argv) == 5):        
    if sys.argv[1] == "avrsetpar":
        par = sys.argv[2]
        val = sys.argv[3]
        node = sys.argv[4]
        COMMAND="{{\"dst\":\"{0:s}\",\"cmd\":\"avrsetpar\",\"avrpar\":\"{1:s}\",\"avrval\":{2:s}}}".format(node,par,val)
# get target AVR uC parmeter            
if (COMMAND=="") and (len(sys.argv) == 4):        
    if sys.argv[1] == "avrgetpar":
        par = sys.argv[2]
        node = sys.argv[3]
        COMMAND="{{\"dst\":\"{0:s}\",\"cmd\":\"avrgetpar\",\"avrpar\":\"{1:s}\"}}".format(node,par)
# no valid command found        
if not COMMAND:
    usage()
    exit(-1)

    

# report command to be sent
print("dmctrl: command {0:s}".format(COMMAND))

  
# run http server for firmware updates  
if run_http:
    print("dmctrl: starting firmware server on {0:s}:{1:d}".format(FHOST,FPORT))
    http_server = socketserver.ThreadingTCPServer((FHOST, FPORT), http.server.SimpleHTTPRequestHandler)
    http_thread = threading.Thread(target=http_server.serve_forever)
    http_thread.daemon = True
    http_thread.start()

    
# doit
run()

# this was an upgrade command
if run_http:
    print("press ctrl-c to shutdown firmware server")
    while True:
        time.sleep(1)
    http_server.shutdown()
