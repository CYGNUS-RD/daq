import argparse
import sys
from xmlrpc.client import ServerProxy
import time
import utilities as ut  # Assuming this contains the log function

log_path = '/home/cygno01/Desktop/ArduinoServerManager/log.txt'

def log(message, path):
    with open(path, 'a') as log_file:
        log_file.write(f"{message}\n")

def main():
    parser = argparse.ArgumentParser(description='Control and monitor HV board.')
    parser.add_argument("-p", "--port", type=int, default=8000, help="Port of the XML-RPC server, default 8000")
    parser.add_argument("-v", "--voltage", type=int, help="Set voltage for the HV board")
    parser.add_argument("-i", "--current", type=int, help="Set current for the HV board")
    parser.add_argument("-c", "--channel", type=int, default=0, help="Select channel on the HV board")
    parser.add_argument("-on", "--turn_on", action='store_true', help="Turn on the selected channel")
    parser.add_argument("-off", "--turn_off", action='store_true', help="Turn off the selected channel")
    args = parser.parse_args()

    if args.channel >= 2:
        log("ERROR: Invalid channel number.", log_path)
        sys.exit(1)
    if args.turn_on and args.turn_off:
        log("ERROR: Conflicting commands to turn on and off simultaneously.", log_path)
        sys.exit(1)

    server = ServerProxy(f"http://localhost:{args.port}")
    print(f"Connected to the server on port {args.port}")

    try:
        # Handle power status changes
        if args.turn_on:
            server.hv_set(args.channel, "Pw", 1)
            log(f"Channel {args.channel} turned on.", log_path)
        elif args.turn_off:
            server.hv_set(args.channel, "Pw", 0)
            log(f"Channel {args.channel} turned off.", log_path)

        # Adjust voltage and current if specified
        if args.voltage is not None:
            server.hv_set(args.channel, "VSet", args.voltage)
            log(f"Voltage set to {args.voltage} for channel {args.channel}.", log_path)
        if args.current is not None:
            server.hv_set(args.channel, "ISet", args.current)
            log(f"Current set to {args.current} for channel {args.channel}.", log_path)

    except Exception as e:
        log(f"ERROR: {e}", log_path)
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()