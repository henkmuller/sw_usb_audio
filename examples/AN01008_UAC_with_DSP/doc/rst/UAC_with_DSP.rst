Extending USB Audio with Digital Signal Processing
==================================================

TODO:
  All code fragments need to be written in actual C, coded, and tested.
  These are blind programming so far.
  
In this app note we describe how to extend the XMOS USB Audio stack with
DSP capabilities.

USB Audio is a highly configurable piece of software; in its simplest form
it may just interface a single ADC to USB Audio; but it can deal with a
multitude of I2S, TDM, DSD, S/PDIF, ADAT and other interfaces. Often data is
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

* ``git@github.com:xmos/lib_xua.git`` for the USB Audio library
  design

Introduction to USB Audio
-------------------------

The basic structure or USB Audio is shown below in
:ref:`extending_usb_audio_with_digital_signal_processing_usb_structure`. 

.. _extending_usb_audio_with_digital_signal_processing_usb_structure:

.. figure:: images/USB-structure.*

            Structure of USB Audio

On the left is a USB interface to the host - this is dealt with by the XUD
and XUA libraries. XUD is the low level USB library for XCORE
(https://www.github.com/xmos/lib_xud), XUA is the USB-Audio protocol
implementation on xcore (https://www.github.com/xmos/lib_xua).
On the right is a series of interfaces (ADC, DAC,
S/PDIF, ADAT). USB Audio provides a path from the left to the right (USB
host computer to the interfaces), this is called the output path; and a
path from the right to the left (the interfaces to the USB host computer)
that is called the input path. The terms input-path and output-path are
host-centric names, and we use input and output this was as it is
consistent with the USB standard nomenclature.

The XU316 device has two tiles, and for many designs one of the tiles will
be empty. This is not always the case, as there may be a situation where
the ADC/DAC I/O pins are located on the other tile. This subtlety does not
matter for addition of simple DSP. Also, the physical core used for the USB
stack may be tile 0 or tile 1 depending on the design.

API offered by USB Audio
------------------------

The USB Audio stack provides one function that you need to override in
order to add any DSP capability to your system::

  void UserBufferManagement(
         unsigned output_samples[NUM_USB_CHAN_OUT],
         unsigned  input_samples[NUM_USB_CHAN_IN]
  );

For brevity we use ``NUM_OUTPUTS`` and ``NUM_INPUTS`` throughout this code
to refer to the number of output audio-channels (``NUM_USB_CHAN_OUT``) and
the number of input audio-channels (``NUM_USB_CHAN_IN``).

The ``UserBufferManagement`` function is called at the sample rate of the USB Audio stack (eg, 48
kHz) and between them the two arrays contain a full multi-channel
audio-frame. The first array carries all the data that shall be shipped to
the interfaces, the second array carries all the data from the interfaces
that shall be shipped to the USB host. You can chose to intercept and
overwrite the samples stored in these arrays.

A second function that you can overwrite is::
  
  void UserBufferManagementInit(void);

Which is called once before the first call to ``UserBufferManagement``. The
code in this document does not require this function, but other code may
require it.

Note that the values of the type are *unsigned*; a 32-bit number. The
use of these 32 bits depends on the data-types used for the audio, typical values
are 16-bit PCM (the top 16 bits are a signed PCM value), 24-bit PCM (the
top 24 bits are a signed PCM value), 32-bit PCM (the top 32 bits are a
signed PCM value), or DSD (the 32 bits are PDM values, with the least
significant bit representing the oldest 1-bit value).

Suppose that we are just building a single-channel microphone; in this
case, NUM_OUTPUTS=0 and NUM_INPUTS=1. We can run the input_samples through
a cascaded_biquad in order equalise the microphone signal::

  int32_t filter_coeffs[4*5] = { ... };
  int32_t filter_states[4*4];
  
  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS])
  {
      input_samples[0] = dsp_filters_biquad((int32_t) input_samples[0],
                                            fiter_coeffs,
                                            filter_states,
                                            4,
                                            28);
  }

Indeed, by instead applying this to the ``output_samples`` array we can
equalise a USB speaker, and by applying two independent cascaded biquads to
the two channels we can equalise stereo speakers::

  int32_t filter_coeffs[4*5] = { ... };
  int32_t filter_states[2][4*4];

  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS])
  {
      for(int i = 0; i < 2; i++) {
        output_samples[i] = dsp_filters_biquad((int32_t) output_samples[i],
                                               fiter_coeffs,
                                               filter_states[i],
                                               4,
                                               28);
    }
  }

By combining input_samples and output_samples one can mix data from
interfaces or USB into USB or the interfaces.

The sample rate depends on the environment. The USB application typically
has a list of supported sample rates (this may just be one sample-rate),
and the user can on the host select which sample rate they want to use. For
simplicity, we do not discuss sample-rate changes; we assume that there
is just one sample-rate.

DSP functions available
-----------------------

There are a few repositories with DSP and general maths functions
available, with different trade-offs between speed, accuracy, and
ease-of-use.

* [lib_xcore_math](http://github.com/xmos/lib_xcore_math) is the xcore.ai library
  for high performance maths functions. Many of them are optimised to make
  use of the vector unit and use 40-bit accumulators.

* [lib_dsp](https://github.com/xmos/lib_dsp) for high-resolution maths functions
  that execute on the CPU often using 64-bit accumulators. These functions
  are not as fast as ``lib_xcore_math``

* [lib_audio_effects](https://github.com/xmos/lib_audio_effects) for audio effects
  functions.


Timing requirements
-------------------

The XMOS USB Audio stack is designed to operate on single samples in order
to minimise latency introduced by the audio stacks. The
``UserBufferManagement()`` function is called from the core of the USB
stack; it is called at the native frame rate of the system (for example 44.1
kHz), and it should therefore take no longer than one sample period to
finish it's operation. In fact, it has a bit less time than that in order
to guarantee that the samples reach the next stage of the pipeline.

Given the speed of a single thread in a system (for example 600 / 8 = 75
MHz) and the sample rate (say, 44.1 KHz sample rate) we can calculate the
number of issue slots available between two samples: 75,000,000 / 44,100 =
1,700 issue slots. This includes the time taken by the USB stack to shuffle
data around. Taking that into account there is no more than 1,300
issue-slots available for DSP using this method, which allows for only a limited
number of FIR taps or biquads to be used. The timeline is shown in
:ref:`extending_usb_audio_with_digital_signal_processing_inside_usb`. 

.. _extending_usb_audio_with_digital_signal_processing_inside_usb:

.. figure:: images/timelines-inside-usb.*

            Timeline of executing DSP inside a thread

What is more, with higher sample rates the overhead of the USB stack is the
same, but the time between samples is squeezed, further limiting the number
of cycles available for DSP.

As XCORE is a concurrent multi-threaded multi-core processor, there are
other threads and cores available for DSP. It depends on the precise
configuration of the USB stack (whether you use special interfaces such as
S/PDIF, ADAT, MIDI) but in a simple with just I2S, USB Audio uses around
30% of the compute, with one core being completely empty.

We will first look at how to use a single thread on the other tile for DSP,
then we will look in how to generally parallelise DSP, and then we will
look into using multiple threads for DSP.

Executing the DSP on the other physical core
--------------------------------------------

The XCORE architecture offers a communication fabric to efficiently
transport data between threads and cores. Communication works on
*channels*. A *channel* has two ends, *A* and *B*, and data that is
*output* into *A* has to be *input* on *B*, and data that is output into
*B* has to be input from *A*.

A channel is like a two way communication pipe. It has very little
buffering capacity, so both ends of the channel have to agree to
communicate otherwise one side will wait for the other.

The data types and functions for communicating data provided by
``lib_xcore`` are:

* ``chanend_t c`` a type holding the reference to one end of a *channel*

* ``chan ch`` a type holding a complete channel with both ends

* ``chan_out_word(c, x)`` a function that outputs a word ``x`` over channel-end
  ``c``.

* ``x = chan_in_word(c)`` a function that inputs a word ``x`` over channel-end
  ``c``.

* ``chan_out_buf_word(c, x, n)`` a function that outputs ``n`` words from
  array ``x`` over channel-end
  ``c``.

* ``chan_in_buf_word(c, x, n)`` a function that inputs ``n`` words over channel-end
  ``c`` into array ``x``

We could also use XC instead of C and lib-xcore; the resulting behaviour
is identical.

Typical code to off-load the DSP to the other tile involves a
``UserBufferManagement`` function that outputs and inputs samples to the
DSP task, a ``user_main.h`` function that declares the extra code needed to
create the channels and start the DSP task, and a DSP task that receives
and transmits the data.

The UserBufferManagement code is::

  static chanend_t g_c;
  
  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS]
  ) {
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples, NUM_OUTPUTS);
    chan_in_buf_word( g_c, input_samples,  NUM_INPUTS);
  }

  void UserBufferManagementSetChan(chanend_t c) {
    g_c = c;
  }

The code to be included in the main program is as follows::

  #define USER_MAIN_DECLARATIONS \
    chan c_data_transport;

  #define USER_MAIN_CORES \
    on tile[USB_TILE]: {                                  \
        UserBufferManagementSetChan(c_data_transport);    \
    }                                                     \
    on tile[!USB_TILE]: {                                 \
        dsp_main(c_data_transport);                       \
    }

And finally the code to perform the DSP is the opposite of the
buffer-management function::

  void dsp_main(chanend_t c_data) {
    int samples_for_usb [NUM_INPUTS + NUM_OUTPUTS];
    int samples_from_usb[NUM_INPUTS + NUM_OUTPUTS];
    while(1) {
      chan_in_buf_word( c_data, &for_usb[0],           NUM_OUTPUTS);
      chan_in_buf_word( c_data, &for_usb[NUM_OUTPUTS], NUM_INPUTS);
      chan_out_buf_word(c_data, &from_usb[0],          NUM_OUTPUTS);
      chan_out_buf_word(c_data, &from_usb[NUM_OUTPUTS],NUM_INPUTS);
      // DSP from from_usb -> for_usb
    }
  }

The execution of two of the tasks (the USB Task calling
``UserBufferManagement``) and the DSP task (``dsp_main``) is shown below
in :ref:`extending_usb_audio_with_digital_signal_processing_single_thread`. 

.. _extending_usb_audio_with_digital_signal_processing_single_thread:

.. figure:: images/timelines-single-thread.*

            Timeline of executing the two concurrent threads

Time progresses from top to bottom, and we show a snapshot of what happens
around the time that Frame numbers 5..7 arrive over I2S. The small dark
blue box is when Frame 5 arrives over I2S whilst a processes Frame 3 is
sent out over I2S. The light blue boxes below are the communication between
the two tasks; ``UserBufferManagement()`` on the left, and the first four
lines of the while-loop in ``dsp_main()`` on the right. After that, the
USB task has a bit of idle time (to cope with higher sample rates and
more channels), and the DSP task starts the DSP. Whilst the DSP is operating
on Frame 5; Frame 6 arrives in the USB task, and the DSP task must finish
before the next communication phase. Please note that the boxes are not
drawn to scale otherwise some of them would be too small to see.

In this example, we assume a 44,100 Hz sample rate. If the DSP thread is
too late, then all the timings will fail; it has to be on time, but it is
allowed to be *just in time*. Note that the DSP processing is synchronous
with the frame transmissions, but the phase is off. Every sample is
processed a bit later than arriving, leading to a whole sample delay

Parallelising DSP
-----------------

Parallelisation involves splitting work into a multitude of *tasks*.
*Tasks* can then be mapped onto threads. The reason to separate these two
words is that a *task* is a software concept: a set of instructions that
does something meaningful, for example a shelf-filter. If we have 10 of
those tasks then we can execute five of them in *Thread 1* and five of them
in *Thread 2* and we have achieved 2x parallelism.

Typically tasks are dependent on each other, and when the design is drawn
out that is reflected by arrows from one task into the other, representing
data being transported from one task to the next. When the tasks are mapped
onto threads these data dependencies have to be adhered to.

DSP lends itself to parallelism as there are typically large clusters of
compute on identified sets of data. Each DSP problem will be parallelised
individually, and in this document we distinguish two models on which the
rest can be built:

* Data parallelism, for example, output-conditioning on stereo speakers. In
  this case, one could put the DSP for the left speaker in task 1, and the
  DSP for the right speaker in task 2.

* Data Pipelining. A series of DSP tasks are executed one after the other
  on an audio stream.

In general this gives rise to two sorts of designs. The first design is one
where each sample is being fed into a task, and the tasks independently of
each other all produce the output samples. The second design is one where
the samples run through a sequence of tasks before finally producing the
output samples. The latter architecture has an inherent higher latency than
the former design and a slightly more complex design. The former is a
very simple design that we shall discuss first.

Data Parallel DSP
-----------------

Data parallelism is a simple extension of the previous example. Instead of
using a single channel we use multiple channels to communicate the data
onto the DSP task. This gives rise to the timeline shown below in
:ref:`extending_usb_audio_with_digital_signal_processing_multi_thread`. 

.. _extending_usb_audio_with_digital_signal_processing_multi_thread:

.. figure:: images/timelines-multi-thread.*

            Timeline of executing the two concurrent threads

Like before, we use channels to communicate between the DSP tasks, what is
new is that we have to create those DSP tasks, and create the channels
between them. The only difference is in the ``dsp_main`` function.

The UserBufferManagement code is::

  static chanend_t g_c, g_c2;
  
  void UserBufferManagement(
         unsigned output_samples[NUM_OUTPUTS],
         unsigned  input_samples[NUM_INPUTS]
  ) {
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples, NUM_OUTPUTS/2);
    chan_in_buf_word( g_c, input_samples,  NUM_INPUTS/2);
    chan_out_buf_word(g_c, output_samples, NUM_OUTPUTS);
    chan_out_buf_word(g_c, input_samples,  NUM_INPUTS);
    chan_in_buf_word( g_c, output_samples+NUM_OUTPUTS/2, NUM_OUTPUTS/2);
    chan_in_buf_word( g_c, input_samples+NUM_INPUTS/2,  NUM_INPUTS/2);
  }

  void UserBufferManagementSetChan(chanend_t c, chanend_t c2) {
    g_c = c;
    g_c2 = c2;
  }

The code to be included in the main program is as follows::

  #define USER_MAIN_DECLARATIONS \
    chan c1, c2;

  #define USER_MAIN_CORES \
    on tile[USB_TILE]: {                                  \
        UserBufferManagementSetChan(c1, c2);              \
    }                                                     \
    on tile[!USB_TILE]: {                                 \
        dsp_main1(c1);                                    \
    }                                                     \
    on tile[!USB_TILE]: {                                 \
        dsp_main2(c2);                                    \
    }

And finally the code to perform the DSP is the opposite of the
buffer-management function::

  void dsp_main1(chanend_t c_data) {
    int samples_for_usb [NUM_INPUTS/2 + NUM_OUTPUTS/2];
    int samples_from_usb[NUM_INPUTS + NUM_OUTPUTS];
    while(1) {
      chan_in_buf_word( c_data, &for_usb[0],           NUM_OUTPUTS);
      chan_in_buf_word( c_data, &for_usb[NUM_OUTPUTS], NUM_INPUTS);
      chan_out_buf_word(c_data, &from_usb[0],          NUM_OUTPUTS/2);
      chan_out_buf_word(c_data, &from_usb[NUM_OUTPUTS/2],NUM_INPUTS/2);
      // DSP from from_usb -> for_usb
    }
  }

``dsp_main2`` is identical, and the code may be shared provided they have
separate state to operate on/

This method expands to five threads, after which the XCORE.AI pipeline is
fully used. More threads can be used, but no performance will be gained.

Data Pipelining DSP
-------------------

We can make an arbitrary pipeline of DSP processes by creating an extra
thread that acts as the source of the data and as the sync of the data.
This thread's purpose is to perform just those tasks. The reason that this
task is special is that it loops the data path around, because what came
out of the pipe has to go back into the USB Audio stack at a determined
point in time. The pipeline that we're building is shown in
:ref:`extending_usb_audio_with_digital_signal_processing_pipeline_figure`. 

.. _extending_usb_audio_with_digital_signal_processing_pipeline_figure:

.. figure:: images/example-pipeline.*

            Example pipeline
   
The pipeline that we are building requires a bit of plumbing to make it all
work but the code is reasonably straightforward otherwise.

DSP task 1B is implemented by ``dsp_thread1b`` and picks up data from
the distributor, and outputs data to dsp tasks 1A and 1B::

  void dsp_thread0(chanend_t c_fromusb,
                    chanend_t c_to1a, chanend_t c_to1b) {     
    int from_usb[NUM_OUTPUTS];
    int for_1[NUM_OUTPUTS];
    while(1) {
      // Pick up my chunk of data to work on
      chan_in_buf_word(c_fromusb, &from_usb[0], NUM_OUTPUTS);

      ..... Perform DSP on from_usb into for_usb .....
      
      // And deliver my answer back
      chan_out_buf_word(c_to1a, &for_1[0], NUM_OUTPUTS);
      chan_out_buf_word(c_to1b, &for_1[0], NUM_OUTPUTS);
    }
  }

DSP task 1A is implemented by ``dsp_thread1a`` and picks up data from
the DSP task 0, and outputs data to dsp task 2::

  void dsp_thread1a(chanend_t c_from0,
                    chanend_t c_to2) {     
    int from_0[NUM_OUTPUTS];
    int for_2[NUM_OUTPUTS];
    while(1) {
      // Pick up my chunk of data to work on
      chan_in_buf_word(c_from0, &from_0[0], NUM_OUTPUTS);

      ..... Perform DSP on from_usb into for_usb .....
      
      // And deliver my answer back
      chan_out_buf_word(c_to2, &for_2[0], NUM_OUTPUTS);
    }
  }

DSP task 1B is implemented by ``dsp_thread1b`` and picks up data from
the DSP task 0, and outputs data to dsp task 2::

  void dsp_thread1b(chanend_t c_from0,
                    chanend_t c_to2) {     
    int from_0[NUM_OUTPUTS];
    int for_2[NUM_OUTPUTS];
    while(1) {
      // Pick up my chunk of data to work on
      chan_in_buf_word(c_from0, &from_0[0], NUM_OUTPUTS);

      ..... Perform DSP on from_usb into for_usb .....
      
      // And deliver my answer back
      chan_out_buf_word(c_to2, &for_2[0], NUM_OUTPUTS);
    }
  }

Similarly, DSP task 2 is implemented by dsp_thread2 and picks up data from
the DSP tasks 1A and 1B, and outputs data t the distribution task. The
weird part of the code is that we need to stick some data into the output
channel end prior to starting the loop - otherwise the data_distribution
task would hang::

  void dsp_thread2(chanend_t c_from1a, chanend_t c_from1b,
                   chanend_t c_todist) {     
    int from_1a[NUM_OUTPUTS];
    int from_1b[NUM_OUTPUTS];
    int for_usb[NUM_OUTPUTS];
    chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS); // Sample -2
    chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS); // Sample -1
    while(1) {
      // Pick up my chunk of data to work on
      chan_in_buf_word(c_from1a, &from_1a[0], NUM_OUTPUTS);
      chan_in_buf_word(c_from1b, &from_1b[0], NUM_OUTPUTS);

      ..... Perform DSP on from_usb into for_usb .....
      
      // And deliver my answer back
      chan_out_buf_word(c_todist, &for_usb[0], NUM_OUTPUTS);
    }
  }

The distributor picks up data from the USB stack, posts it to DSP task 0,
and picks up an answer from DSP task 2::

  void dsp_data_distributor(chanend_t c_usb, chanend_t c_to0, chanend_t c_from2) {     
    int for_usb [NUM_OUTPUTS];
    int from_usb[NUM_OUTPUTS];
    while(1) {
      // First deal with the USB side
      chan_in_buf_word( c_data, &from_usb[0], NUM_OUTPUTS);
      chan_out_buf_word(c_data, &for_usb[0],  NUM_OUTPUTS);
      // Now supply all data to both DSP tasks
      chan_out_buf_word(c_dsp0, &from_usb[0], NUM_OUTPUTS);
      // Now pick up data from DSP task 2
      chan_in_buf_word( c_from2, &for_usb[0],             NUM_OUTPUTS);
    }
  }

Finally, we need the code to start all the parallel threads. This code
starts five tasks, and connects them up using six channels::
  
  DECLARE_JOB(dsp_data_distributor, (chanend_t, chanend_t, chanend_t));
  DECLARE_JOB(dsp_thread0,  (chanend_t, chanend_t, chanend_t));
  DECLARE_JOB(dsp_thread1a, (chanend_t, chanend_t));
  DECLARE_JOB(dsp_thread1b, (chanend_t, chanend_t));
  DECLARE_JOB(dsp_thread2,  (chanend_t, chanend_t, chanend_t));

  void dsp_main(chanend_t c_data) {
    channel_t c_dist_to_0 = chan_alloc();
    channel_t c_0_to_1a   = chan_alloc();
    channel_t c_0_to_1b   = chan_alloc();
    channel_t c_1a_to_2   = chan_alloc();
    channel_t c_1b_to_2   = chan_alloc();
    channel_t c_2_to_dist = chan_alloc();
    PAR_JOBS(
        PJOB(dsp_data_distributor, (c_data, c_dist_to_0.end_a, c_2_to_dist.end_b)),
        PJOB(dsp_thread0,  (c_dist_to_0.end_b, c_0_to_1a.end_a, c_0_to_1b.end_a)),
        PJOB(dsp_thread1a, (c_0_to_1a.end_b, c_1a_to_2.end_a)),
        PJOB(dsp_thread1b, (c_0_to_1b.end_b, c_1b_to_2.end_a)),
        PJOB(dsp_thread2,  (c_1a_to_2.end_b, c_1a_to_2.end_b, c_2_to_dist.end_a)),
  }

In order to show how this code works, we show a diagram in
:ref:`extending_usb_audio_with_digital_signal_processing_pipeline_timing`. 
Note that the distribution task is mostly idle; it ony consumes very little
processing in the beginning and the end of the sample-cycle. This means
that five other threads can be used to soak up the available DSP.

.. _extending_usb_audio_with_digital_signal_processing_pipeline_timing:

.. figure:: images/timelines-complex-thread.*

            Timeline of the pipelined example
            
Controlling
-----------

In order to control the DSP that you have inserted into the code (eg,
volume control, equaliser settings), the easiest method is to store the
settings in memory, and run an
asynchronous thread that has access to those variables. This asynchronous
thread could be controlled from an A/P (over, say, I2C or SPI), or it can
interface directly with, for example, rotary encoders, push buttons,
sliders, or a touch screen.

This method of updating needs to be done with some care as the memory is
updated asynchronously to the DSP. Updating a volume
control is completely safe; it either takes effect this sample or the next.
Updating the taps of an FIR filter is also safe, the worst that can happen
is that for a sample it will use some of the old and some of the new
parameters. When updating an IIR filter (a biquad), it can be damaging to
update the biquad coefficients in the middle of the execution. In
particular, it may make the filter unstable. This can be avoided by
updating the biquad in small steps (which is generally a good idea because
the internal state needs to settle too), or one can use a synchronous
thread instead.

If using a synchronised thread, the idea is that the thread does not just
update the variables, but it requests the variables to be updated by the
DSP thread itself; at a time that is safe for the DSP. This will require a
channel between the two threads and a protocol that causes the
control-thread to request an update, and an answer from the DSP task when
it is ready, whereupon the control-task posts the new filter coefficients
that can be used by the DSP-thread.

