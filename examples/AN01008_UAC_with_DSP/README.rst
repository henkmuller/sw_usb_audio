XMOS xCORE-AI USB Audio
=======================

This is a sample program on how to combine USB Audio with DSP. THe
documenation is in doc/rst; or you can search for XMOS App note AN01008 in
order to get a PDF.

The documentation describes four methods for integrating USB. These can be
built as follows:

* ``xmake CONFIG=USB_THREAD``, most source code in ``src/dsp_code_usb_thread.c``

* ``xmake CONFIG=SINGLE_THREAD``, most source code in ``src/dsp_code_single_thread.c``

* ``xmake CONFIG=MULTI_THREAD``, most source code in ``src/dsp_code_multi_thread.c``

* ``xmake CONFIG=PIPELINE``, most source code in ``src/dsp_code_pipeline.c``

``xmake`` without arguments builds all four.

