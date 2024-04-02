from time import sleep
import serial
import mido
import jack

BAD_DATA = 0
TIMEOUT = 30

COM_PORT = '/dev/ttyACM0'
BAUD_RATE = 9600

NUMBER_OF_CONTROLS = 3
CONTROLS = [control for control in range(1, NUMBER_OF_CONTROLS + 1)]

CARDINAL_AUDIO_OUT_LEFT = 'Cardinal:audio_out_7'
CARDINAL_AUDIO_OUT_RIGHT = 'Cardinal:audio_out_8'

AUDIO_PLAYBACK_LEFT = 'Built-in Audio Analog Stereo:playback_FL'
AUDIO_PLAYBACK_RIGHT = 'Built-in Audio Analog Stereo:playback_FR'

INTERFACE = 'Arduino-Cardinal-MIDI-Interface'
MIDO_PORT = 'Arduino-Cardinal-MIDI-Interface:midi_out'

CARDINAL_INPORT = 'Cardinal:events-in'
INTERFACE_OUTPORT = 'Midi-Bridge:RtMidiOut Client:(capture_0) Arduino-Cardinal-MIDI-Interface:midi_out'

arduino_serial = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)

jack_client = jack.Client(INTERFACE)

mido_outport = mido.open_output(MIDO_PORT, virtual=True)
sleep(0.5)

cardinal_midi_inport = jack_client.get_port_by_name(CARDINAL_INPORT)
interface_midi_outport = jack_client.get_port_by_name(INTERFACE_OUTPORT)

cardinal_audio_out_left = jack_client.get_port_by_name(CARDINAL_AUDIO_OUT_LEFT)
cardinal_audio_out_right = jack_client.get_port_by_name(CARDINAL_AUDIO_OUT_RIGHT)

playback_left = jack_client.get_port_by_name(AUDIO_PLAYBACK_LEFT)
playback_right = jack_client.get_port_by_name(AUDIO_PLAYBACK_RIGHT)

def get_control(control):
    return CONTROLS[control - 1]

def check_connection(input_port, output_port):
    """Checks if a connection is already established between two ports
    :param input_port: The input port
    :param output_port: The output port
    :return True if the connection is already established, False otherwise"""

    connectionAlreadyBound = False
    for connection in jack_client.get_all_connections(input_port):
        if connection.name == output_port.name:
            connectionAlreadyBound = True
            break

    return connectionAlreadyBound

def connect_interface_to_cardinal():
    if not check_connection(interface_midi_outport, cardinal_midi_inport):
        jack_client.connect(interface_midi_outport, cardinal_midi_inport)

    if not check_connection(cardinal_audio_out_left, playback_left) and not check_connection(playback_left, cardinal_audio_out_left):
        jack_client.connect(cardinal_audio_out_left, playback_left)

    if not check_connection(cardinal_audio_out_right, playback_right) and not check_connection(playback_right, cardinal_audio_out_right):
        jack_client.connect(cardinal_audio_out_right, playback_right)

    print("Connection established")

def stop_interface():
    print("\rResetting all Cardinal control change outputs")
    for control in CONTROLS:
        reset_message = mido.Message('control_change', control=control, value=0)
        print(str(control) + "/" + str(len(CONTROLS)), end='\r')
        sleep(0.1)
        mido_outport.send(reset_message)
    print("Reset done")

    print("Closing ports", end='\n\r')
    if check_connection(interface_midi_outport, cardinal_midi_inport):
        jack_client.disconnect(interface_midi_outport, cardinal_midi_inport)

    if check_connection(cardinal_audio_out_left, playback_left):
        jack_client.disconnect(cardinal_audio_out_left, playback_left)

    if check_connection(cardinal_audio_out_right, playback_right):
        jack_client.disconnect(cardinal_audio_out_right, playback_right)

    sleep(0.5)

    jack_client.close()
    mido_outport.close()
    print("Ports closed")
    
    print("Exiting...")
    exit(0)

def reconnect_serial():
    print("Serial port disconnected")
    print("Trying to reconnect", end='', flush=True)
    global arduino_serial
    arduino_serial.close()
    timeout = 0
    reconnected = False
    while timeout < TIMEOUT and not reconnected:
        try:
            arduino_serial = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
            print("\nSerial port reconnected")
            reconnected = True
        except serial.SerialException:
            timeout += 1
            print(".", end='', flush=True)
            if timeout % 4 == 0:
                print("\r\033[K\033[F")
                print("Trying to reconnect", end='', flush=True)
            sleep(1)
    
    if (timeout == TIMEOUT and reconnected == False):
        print("\nTimeout exceeded, exiting...")
        exit(0)

def reconnect_cardinal():
    print("Connection with Cardinal lost, trying to reconnect...")
    print("Trying to reconnect", end='', flush=True)
    timeout = 0
    reconnected = False
    while timeout < TIMEOUT and not reconnected:
        try:
            connect_interface_to_cardinal()
            print("\rReconnected to Cardinal")
            reconnected = True
        except jack.JackErrorCode:
            timeout += 1
            print(".", end='', flush=True)
            if timeout % 4 == 0:
                print("\r\033[K\033[F")
                print("Trying to reconnect", end='', flush=True)
            sleep(1)
    
    if (timeout == TIMEOUT and reconnected == False):
        print("\nTimeout exceeded, exiting...")
        exit(0)


def read_serial_data():
    data = arduino_serial.readline().decode('utf8', errors='ignore').strip()
    arduino_serial.flushInput()
    return data

def process_data(data):
    serial_output = data.split(',')
    if len(serial_output) == 2 and serial_output[0] != '' and serial_output[1] != '' and serial_output[1].isdigit():
        cut_data = int(serial_output[1]) - 950
        scaled_data = cut_data * (1023 / (1023 - 950))
        previous_data = scaled_data
        voltage = (scaled_data / 1023) * 5

        if voltage >= 4.9:
            voltage = 4.9
        if voltage <= 0.1:
            voltage = 0.1

        processed_data = {}
        processed_data['pin'] = serial_output[0]
        processed_data['voltage'] = voltage

        return processed_data
    else:
        return BAD_DATA

def convert_to_MIDI_and_send(data):
    data['voltage'] = int(data['voltage'] * 127 / 5)

    isMessageDataValid = True

    match data['pin']:
        case '0':
            MIDI_message = mido.Message('control_change', control=get_control(1), value=data['voltage'])
        case '1':
            MIDI_message = mido.Message('control_change', control=get_control(2), value=data['voltage'])
        case '2':
            MIDI_message = mido.Message('control_change', control=get_control(3), value=data['voltage'])
        case other:
            isMessageDataValid = False
            print("No match found")
        
    sleep(0.01)

    if isMessageDataValid:
        mido_outport.send(MIDI_message)

def process_midi():
    serial_output = read_serial_data()
    processed_data = process_data(serial_output)
    if processed_data:
        convert_to_MIDI_and_send(processed_data)


if __name__ == "__main__":
    connect_interface_to_cardinal()
    while True:
        try:
            if not check_connection(interface_midi_outport, cardinal_midi_inport):
                raise jack.JackErrorCode("Connection lost with Cardinal", 22)
            process_midi()
        except KeyboardInterrupt:
            stop_interface()
        except serial.SerialException: 
            reconnect_serial()
        except jack.JackErrorCode:
            reconnect_cardinal()
        except Exception as e:
            print("An error occurred: " + str(e))

        # TODO do a bash launching all the necessary services so it avoids certains cases where the interface tries to connect while Cardinal is not started, and it starts Cardinal with the right configuration