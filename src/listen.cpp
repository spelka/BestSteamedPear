//Listening Thread
while listening
    Wait for serial event          // Listening To Serial Event state
    if received ENQ or RVI
        send ACK

        while !EOT	          //loop in the thread that receives packets
            set timer to TO1 with timeout callback
            wait for serial event // Waiting for data packet state
            if !interrupted
                 set SetTimer() to 0 with a null callback
                 validate received data// Packet Validation function
                 if received data is valid
			   packet accepted
                     if RVI
                         send RVI
                     else
                         send ACK
                 else
                     send NAK

//timeout callback
set interrupted flag to true
interrupt “wait for serial event”

//validate packet data
if packet syn equals prev_syn
    discard packet
    return
apply CRC on data of the packet
if CRC result matches received CRC
    update received buffer’s cursor to signify available data
    dispatch data received event