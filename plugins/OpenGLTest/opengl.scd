~samp =Buffer.readChannel(s,"YOUR SAMPLE PATH HERE",channels:[0])
~samp.free

~width=2048; // in this example we use the x dimension to write audio to shader


(a={
	var out;
	var input;
	//var input = PlayBuf.ar(1,~samp,loop:1); //UNCOMMENT IF YOU LOADED A SAMPLE

	//read horizontally
	var phasor = [LFSaw.ar(\freqr1.ar(s.sampleRate /(~width))).linlin(-1.0,1.0,0.0,1.0) * \a1.ar(1.0) + \o1.ar(0.0),
		(\modAmt.ar(0.01)*SinOsc.ar(\mod.ar(0.1)))+ \o2.ar(0.48)]; // Modulate y position read


	input = SinOsc.ar(\freqw.ar(50)); //COMMENT IF YOU LOADED A SAMPLE

	//args are input, gain, x, y, path
	//input to shader
	//x y read out coordinates between 0-1
	//output gain
	//path is path to shader folder
	out=OpenGLTest.ar
	(
		\inputAmp.ar(0.5)*input,
		DC.ar(1),
		phasor[0].scope,
		phasor[1],
		~width,
		512,     // keep height fixed -> only for this example, feel free to use different sizes
		log2(~width/s.options.blockSize), //use parsimoniously. Internally this resolves to pow(2, thisParamValue) * vectorSize >>> so how many vectors will be the backbuffer be made of, in powers of 2
		(Platform.userExtensionDir++"\\OpenGLTest\\").standardizePath   // put your shaders in the Extensions folder if you use this -> naming is fixed, so use shader.vert, shader.frag and post.vert, post.frag

	);

	out.scope;

	((out*\ampOut.ar(0.5))+(input*\inOutAmp.ar(0.2)) ).tanh.dup;

}.play
)



a.set( \freqw,  s.sampleRate /(~width));

a.set( \freqr1, s.sampleRate /(~width) );

//select portion to read
a.set(\a1,1,\o1,0.0,  \o2,0.2, \modAmt,0,\mod,0.01)

a.set(\inputAmp,0.5,\ampOut,0.5, \inOutAmp, 0.2)

a.free







