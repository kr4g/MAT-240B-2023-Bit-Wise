# Assignment 2

Generate quasi-bandlimited (virtual) analog waveforms using frequency modulation as explained in Schoffhauzer's paper. Start with the source code from the paper. Copy and paste, but beware of the copy/paste errors that happen with .pdf files. Next, try to compile this code *without JUCE*. It would be easy to use our UNIX-like system from the previous homework. Adjust the code so that it compiles, changing as little as possible. Next, add to the code so that it prints the sample data for about a half a second of a sawtooth. Now try the pulse wave. Only use one or the other; Do not try to compute them both at the same time. Why? Look at the output data. Listen to the output. Do these seem reasonable¹? Once you know the algorithm is working, abstract these allgorithms into one or more C++ classes². Allow a user to set the various parameters (e.g., frequency, pulse width) and get the next sample (`.getNextSample()`, `.next()`, `float operator()() {}`). Now test this class-based code. Does it produce the same reasonable output as before?

Once you believe the algorithms work, add your class to the JUCE starter code. Make parameters for gain (controls loudness), MIDI note (controls frequency), virtual filter³, mix (how much sawtooth versus how much pulse), and pulse width.

¹ This paper comes with a semantic error; Try to find it by comparing the pulse and saw algorithms. Nerd points to those who fix the error. Otherwise, show me where the error is and I will show you how to fix it.
² The paper's code may be encapsulated into a single class, but it could also be separated into two classes. Or, both the Pulse and Saw classes might be siblings of a parent class. Don't go too far to start. You could do these algorithms only using functions.
³ You must introduce this parameter to the algorithm. Useful values are on (0, 1.4) in my implementation. This controls how much the algorithms feedsback which determines how fast the harmonics "roll off".


## Set up

Start by getting JUCE in this folder:

    git clone https://github.com/juce-framework/JUCE

Also, say `brew install clang-format` on the terminal to get a great code formatting tool.

Use the Visual Studio Code editor unless you know what you are doing. Open this folder in VScode so that this README is in the root of the workspace.

Install these extensions:

- Microsoft's C/C++ (IntelliSense)
- Microsoft's CMake Tools
- Xaver Hellauer's Clang-Format

Once you have these extensions, close and reopen VScode.
# MAT-240B-2023-Bit-Wise
