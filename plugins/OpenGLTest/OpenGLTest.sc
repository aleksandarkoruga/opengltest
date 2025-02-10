OpenGLTest : UGen {
	*ar { |input, gain, x, y, path|
		/* TODO */
		var file_args = Array.with(path.size, *path.asList.collect(_.ascii));
		var input_args = [input, gain, x,y];
		file_args.postln;
		^this.multiNew('audio', *(input_args++file_args));
	}
	checkInputs {
		/* TODO */
		^this.checkValidInputs;
	}
}
