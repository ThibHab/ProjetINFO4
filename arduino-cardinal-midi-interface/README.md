# Arduino-Cardinal interface for Cardinal project

Python interface that reads all data sent from the Arduino (Uno) card, processes it and sends it to the Cardinal synthetiser application using MIDI messages.

# How to use the interface

- The controller used to send the MIDI messages can be modified, it can be retrieved on Cardinal by plugging the Cardinal MIDI Host CC output with the corresponding number.

# Common configuration errors and how to fix

- Do not open the serial terminal on the Arduino card or you will not be able to read data because the data will already be read on the Arduino terminal and nothing will remain for the system terminal.
- Always use the same baud rate while configuring the port.
- Be sure to put end of line marks while writing data on the Arduino card or issues may occur.
- It is impossible to upload the code on the card while this program is started, each must be done separately while the other is not running.

# Documentation

> get_control(control)

Returns channel corresponding to the MIDI Control Change (CC) gate on which to sent the MIDI message

**param**: control: The control change corresponding to the channel input to retrieve

> check_connection(input_port, output_port)

Checks if a connection is already established between two ports

**param**: input_port: The input port  
**param**: output_port: The output port  
**return** True if the connection is already established, False otherwise

> connect_interface_to_cardinal()

Checks if all connections are already made and does them if not. This includes:

- The MIDI connection between the interface MIDI output and Cardinal MIDI input
- The left and right audio connections between Cardinal audio outputs and the System left and right audio playback inputs


> stop_interface()

Stops the inteface when a shutting down signal is sent (e.i when a KeyboardInterruption such as CTRL-C is triggered)

It resets all Cardinal CC outputs to 0 by sending a 0 value to them, so the configuration is reset when the interface is shutting down

It also closes the MIDI and audio ports connections between the interface and Cardinal and closes the MIDI ports before exiting

> reconnect_serial()

Tries to reconnect the interface to the Arduino board when a deconnection occurs

The interface will regularly try to reconnect itself to the serial port the Arduino board was using, until a timeout is reached

A message will be displayed to inform the user whether the reconnection has succeeded or not

> reconnect_cardinal()

Tries to reconnect the interface to Cardinal when a deconnection occurs

The interface will regularly try to reconnect its MIDI output port to the Arduino MIDI input port, until a timeout is reached

A message will be displayed to inform the user whether the reconnection has succeeded or not 

> read_serial_data()

Reads the data that is sent on the serial port on which the Arduino board and the interface are connected

The data contains the device pin that has sent it and it's value

**return** the raw data read on the serial port in the following format : device_id, value

> process_data(data)

Processes the data read on the serial port. It actually converts the value received from the serial port which is coded on 10 bits to a value corresponding on 7 bits (because Cardinal MIDI ports only accepts 7 bit data)

Also applies a scale factor to the values because of the portentiometers small range of changing values used during the development

The data is checked to see if it fits the defined format, which is : device_id, value. If it does the data is processed and if not the data is skipped

**param**: data: The raw data to be processed  
**return** the processed data if its format is valid or a BAD_DATA value if not

> convert_to_MIDI_and_send(data)

Converts the processed value to a MIDI message and sends it to a defined MIDI output depending on the pin of the device

The signal will be received on the MIDI input corresponding to the MIDI output on which the data was sent

If the device pin corresponds to a valid MIDI output port the data will be sent on it. If not a message will be displayed indicating that no port matches the device pin

**param**: data: the processed data to be converted and sent as MIDI

> process_midi()

Groups the methods that handle the data from the reading to the sending