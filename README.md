# Look Mom No Wifi

This project involves building a fully functional network protocol where audio is the shared medium.
Right now everything is tested digitally with BlackHole audio bridge as the shared medium, a SuperCollider Patch as the sender / encoder and a python script as Listener / Decoder.
Ascii encoding and decoding is already supported.

## Theoretical Baseline

Networks usually represent a blackbox for most people. You connect to the wifi and (in the best case) everything just works. Different protocols, syncing, DNS etc. all work in the background. Apart from network engineers most people probably will never be confronted with the actual workings of a network. I personally believe that there is beauty hidden in the workings of a network. It may just be a fascination for the ingenuity of the people that developed the logic of our modern networks but i still think there is a great aesthetic potential hidden away in the network blackbox.
**Look Mom No Wifi** (working title) aims to bring the shadowed dynamics of network communication into the light. Audio is chosen as the shared medium, as it is perceptually available in contrast to RF based networks.
The auditive domain as the playing ground for communication of course comes with some physical and probably social challenges. Sound is rather slow at $343 \frac{m}{s}$ at 20°C. This means that the whole system is already prone to a lot of latency. Two clients places 10 meters apart already have a message delay of

$$
\Delta t = \frac{10 m}{343 \frac{m}{s}} \approx 0,029 s
$$

On top of that using the sound as a shared medium means the system is exposed to overtones via distortion of the speakers, room reflections, interference and especially external noise. As only the nodes themselves are incorporated into the network itself, everything else will "disturb". The network therefore does not *supply* any functionality to users, but *expects* them to stay silent.

## Signal Characteristics

### Signal Channels

The payload is encoded via [FSK (Frequency-Shift Keying)](https://en.wikipedia.org/wiki/Frequency-shift_keying) and is sent over 5 different channels, defined by their frequency, according to logical function. The channel functions are described in the following table:

| Channel | Logic     |
|---------|-----------|
| 0       | BEACON    |
| 1       | REQUEST   |
| 2       | SYNC      |
| 3       | DATA      |
| 4       | EMERGENCY |

The exact frequencies will be a choice of sound composition keeping potential for errors (i.e. avoiding overtone overlaps) in mind.

### Message Scheduling

The scheduling is inspired by a [TDM (Time-Division Multiplexing) network](https://en.wikipedia.org/wiki/Time-division_multiplexing), dividing the timeline into several fixed slots for each client of the network to use. A client can send on as many logical channels as needed simultaneously.

### Message Structure

Every message follows this structure:


| Bit 0 – 15 | Bit 16 – 23 | Bit 24 – 31    | 32 – *X* | *X + 1* – end |
|------------|-------------|----------------|----------|---------------|
| Preambel   | Sync        | Payload Length | Payload  | Checksum      |

After the payload the message is concluded with a checksum for obvious reasons.

## Network Characteristics

*under construction*

## Hardware Plans

The individual nodes of the network will be build from an esp32 MCU with a MAX98357A I2S audio amplifier connected to a small speaker, as well as a INMP441 I2S encoding microphone.

## TODO

- [x] Get acoustic transmission working
- [x] Build test node
- [ ] Build Nodes
- [ ] Combine Decoding and Encoding into single ESP32 code
- [ ] Make Sine-Wavetables for data encoding
- [ ] Write autocalibration for rms / filter threshholds 
- [ ] Refactor Packet decoding for virtual packets
- [ ] Write audio sender 
- [ ] Implement simple scheduling
