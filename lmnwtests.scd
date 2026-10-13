(
s.options.outDevice = "BlackHole 2ch"
s.reboot;
)
s.meter.window.alwaysOnTop_(true);

~encoder.(2000, [1,0,1,0,1], 100, 100)
(


~encoder = { |freq = 2000, code, binLength = 50, freqDelta = 200|
    var buf;

	var totalTime = code.size * (binLength / 1000);
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

    buf = Buffer.loadCollection(s, code);

    buf.loadToFloatArray(action: { |data|
        Synth(\player, [
            \buf, buf,
            \freq, freq,
            \freqDelta, freqDelta,
            \binLength, binLength,
            \numBits, code.size
        ]);
    });
};
)

(
{
    SinOsc.ar(600, 0, 0.2) ! 2
}.play;
)
