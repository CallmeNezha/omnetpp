import sys
import os
import time

# setting up flushing of the output after every print by default
import functools
print = functools.partial(print, flush=True)

import PythonEntryPoint

from py4j.java_gateway import DEFAULT_PORT
from py4j.clientserver import ClientServer, JavaParameters, PythonParameters

java_port = int(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PORT
# print("initiating Python ClientServer, connecting to port " + str(java_port))

entry_point = PythonEntryPoint.PythonEntryPoint()

properties_provider = None # will be set later
results_provider = None # will be set later
widget_provider = None # will be set later
chart_plotter = None # will be set later

gateway = ClientServer(
    java_parameters=JavaParameters(port=java_port, auto_field=True, auto_convert=True, auto_close=True),
    python_parameters=PythonParameters(port=0, daemonize=True, daemonize_connections=True),
    python_server_entry_point=entry_point)

python_port = gateway.get_callback_server().get_listening_port()

# telling Java which port we listen on
gateway.java_gateway_server.resetCallbackClient(
    gateway.java_gateway_server.getCallbackClient().getAddress(), python_port)

# print("Python ClientServer done, listening on port " + str(python_port))

