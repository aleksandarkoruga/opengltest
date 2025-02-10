(
a={
	arg amp=1;
	var out;

	//read horizontally
	var phasor = [SinOsc.ar(\freqr1.ar(10.0)) + \o1.ar(0.5) * \a1.ar(0.5),\a2.ar(0.5)];

	//args are input, gain, x, y, path
	//input to shader
	//x y read out coordinates between 0-1
	//output gain
	//path is path to shader folder
	out=OpenGLTest.ar(amp* SinOsc.ar(\freqw.ar(60)),DC.ar(1),phasor[0],phasor[1],(Platform.userExtensionDir++"\\OpenGLTest\\").standardizePath);
	out.scope;
	(out*0.1).tanh.dup;

}.play
)

a.set( \freqw, 60, \amp, 1);

a.set( \freqr1,50.0);

//select portion to read
a.set(\a1,0.1,\o1,0.1,\a2,0.5)


a.free

