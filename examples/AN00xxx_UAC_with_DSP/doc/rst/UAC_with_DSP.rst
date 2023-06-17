Extending USB Audio with Digital Signal Processing
==================================================

In this app note we describe how to extend the XMOS USB Audio stack with
DSP capabilities.

USB Audio is a highly configurable piece of software; in its simplest form
it may just interface a single ADC to USB Audio; but it can deal with a
multitude of I2S, TDM, DSD, SPDIF, ADAT and other interfaces. Often data is
just transported in real-time, but DSP that may be interesting may, for
example, include:

* Equalisation

* Mixing

* Dynamic range compression

* Audio effects

This app note discusses the API that the USB Audio stack offers to enable
you to include DSP algorithms inside the stack.

For reference, we refer to the following repositories that you may want to
use:

* ``git@github.com:xmos/sw_usb_audio.git`` for the USB Audio reference
  design

* ``git@github.com:xmos/lib_xcore_math.git`` for high performance maths
  functions executing on the vector unit with 40-bit accumulators

* ``git@github.com:xmos/lib_dsp.git`` for high-resolution maths functions
  that execute on the CPU often using 64-bit accumulators.

* ``git@github.com:xmos/lib_audio_effects.git`` for audio effects
  functions.

Introduction to USB Audio
-------------------------

The basic structure or USB Audio is shown below:


On the left is a USB interface to the host - this is dealt with by the XUD
and XUA libraries. On the right is a series of interfaces (ADC, DAC,
S/PDIF, ADAT). USB Audio provides a path from the left to the right (USB
host computer to the interfaces), this is called the output path; and a
path from the right to the left (the interfaces to the USB host computer)
that is called the input path. The terms input-path and output-path are
host-centric names, and we use input and output this was as it is
consistent with the USB standard nomenclature.

API available
-------------

The USB Audio stack provides one function that you need to override in
order to add any DSP capability to your system::

  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS]
  );

The function is called at the sample rate of the USB Audio stack
(eg, 48 kHz) and the two arras carry a full multi-channel audio-frame.
The first array carries all the data that shall be shipped to the
interfaes, the second array carries all the data from the interfaces that
shall be shipped to the USB host. You can chose to intercept and overwrite
the sample values.

Suppose that we are just building a single-channel microphone; in this
case, NUM_OUTPUTS=0 and NUM_INPUTS=1. We can run the input_samples through
a cascaded_biquad in order equalise the microphone signal::

  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS]
  ) {
      input_samples[0] = cascaded_biquad(biquad_state, input_samples[0]);
  }

Indeed, by instead applying this to the ``output_samples`` array we can
equalise a USB speaker, and by applying two independent cascaded biquads to
the two channels we can equalise stereo speakers::

  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS]
  ) {
      output_samples[0] = cascaded_biquad(biquad_state0, output_samples[0]);
      output_samples[1] = cascaded_biquad(biquad_state1, output_samples[1]);
  }

By combining input_samples and output_samples one can mix data from
interfaces or USB into USB or the interfaces.

Timing requirements
-------------------

The XMOS USB Audio stack is designed to operate on single samples in order
to minimise latency introduced by the audio stacks. The
``UserBufferManagement()`` function is called from the core of the USB
stack; it is called at the native frame rate of the system (for example 48
kHz), and it should therefore take no longer than one sample period to
finish it's operation. In fact, it has a bit less time than that in order
to guarantee that the samples reach the next stage of the pipeline.

Given the speed of a single thread in a system (for example 600 / 8 = 75
MHz) and the sample rate (say, 44.1 KHz sample rate) we can calculate the
number of issue slots available for DSP inside this function: 75,000,000 /
44,100 = 1,700 issue slots. In general, a load from memory, a store to
memory, and a multiply-accumulate all require an issue slot, so one can see
that these 1,700 issue slots only allow for a limited number of FIR taps
biquads to be used.

Depending on the architecture system, it is likely that there is a lot more
compute available. In a simple system with just I2S USB Audio uses around
60% of the compute of one of the two physical cores; the other core can be
completely empty and be used for DSP. To use the full computational
capability of the XCORE.AI chip we need to use at least five threads which
means that we need to run the DSP in parallel; as we will see this is easy
to achieve. 

We need to look at two aspects:

* Using UserBufferManagement to transport data to the other physical core

* Using a multitude of threads on the other physical core to implement the
  DSP in a parallel manner. 

Transporting data to another core
---------------------------------




Parallelising DSP
-----------------



