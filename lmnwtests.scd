(s.options.outDevice = "BlackHole 2ch";
s.options.sampleRate = 48000;
s.options.outDevice = "MacBook Pro-Lautsprecher";
s.reboot;
)
s.meter.window.alwaysOnTop_(true);

~encoder.(2000, ~textToBits.("Hello Network!") , 50, 200)

~encoder.(2000, ~textToBits.("This is a test me!") , 5, 200)

(
{SinOsc.ar(2200)}.play;
)


(

~textToBits = { |text|
	text.ascii.collect{ |char|
		~byteToBits.(char)
	}.flat;
};

~byteToBits = {|byte|

	Array.fill(8, { |i|
		(byte >> (7-i)) & 1
	})
};

~checkSumBits = { |bits|

	var checksum = 0;

	bits.clump(8).do {|group|

		var padded = group ++ Array.fill(8-group.size, 0);
		var byte = 0;

		padded.do {|bit|
			byte = (byte << 1) | bit;
		};

		checksum = checksum.bitXor(byte);
	};
	~byteToBits.(checksum);

};

~makePacket = {|payload|

	var preamble = [1,0,1,0,1,0,1,0,
		1,0,1,0,1,0,1,0];
	var sync = [1,1,0,1,0,0,1,0];

	var lengthBits;
	var checksum;

	if(payload.size > 255) {
		Error("Max Payload sieze is 255 Bits").throw;
	};

	lengthBits = ~byteToBits.(payload.size);
	checksum = ~checkSumBits.(payload);
	"Length:".postln;
	payload.size.postln;

	preamble ++ sync ++ lengthBits ++ payload ++ checksum;

};


~encoder = { |freq = 2000, code, binLength = 50, freqDelta = 200|
	var packet = ~makePacket.(code);
    var buf;

	var totalTime = packet.size * (binLength / 1000);
	SynthDef(\player, {
		|out = 0, buf = 0, freq = 2000, freqDelta = 200,
		binLength = 50, numBits = 5|

		var trig, index, bit, seq, sig,env;

		trig = Impulse.kr(1000 / binLength);

		index = Demand.kr(
			trig,
			0,
			Dseries(0, 1, numBits)
		);

		bit = Index.kr(buf, index);

		seq = bit * (freqDelta * 2) + freq - freqDelta;

		env = EnvGen.kr(
			Env.linen(0.005, totalTime - 0.01, 0.005),
			doneAction: 2
		);
		sig = SinOsc.ar(seq);

		Out.ar(out, sig ! 2);
	}).add;

    buf = Buffer.loadCollection(s, packet);

	"Sending Packet:".postln;
	packet.postln;

    buf.loadToFloatArray(action: { |data|
        Synth(\player, [
            \buf, buf,
            \freq, freq,
            \freqDelta, freqDelta,
            \binLength, binLength,
            \numBits, packet.size
        ]);
    });
};
)
