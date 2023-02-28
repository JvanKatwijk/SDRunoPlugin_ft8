    FT8 plugin for SDRuno

This directory - or folder if you like - contains the sources for
the SDRuno FT8 plugin.
The plugin is developed as a ".dll" file, using the SDRuno plugin API.

The plugin takes the IQOUT port provided by SDRuno, a port that delivers
I/Q samples (well, actually the samples are Q/I) with a speed of 19200
samples per second.
The first step in the plugin's operation is to filter and decimate to
a stream of samples on 12000 Ss.

The plugin provides only a few controls, read the manual.

![overview](/ft8-widget.png?raw=true)

The code in the plugin uses parts of the code of the FT4FT8 decoding
software of Karlis Goba, in particular the LDPC decoder is (almost)
a copy of his code, and the copyright
to the parts taken or derived from his code are gratefully acknowledged.


