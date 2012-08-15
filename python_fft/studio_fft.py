# dependencies:
# python 2.7
#   scipy / numpy
#   pyaudio
#   pySerial

from __future__ import division
import pyaudio
import numpy
import serial
from numpy import array, zeros, linspace, short, fromstring, hstack, transpose, fft as nfft
from scipy import fft
from datetime import datetime
import operator
import os
from time import clock, sleep
import math
from socket import socket, AF_INET, SOCK_STREAM


COMM_PORT = '/dev/tty.usbmodemfa1431'

class TonalCenter(object):
    # helper class used to calculate tonal center of song based on a series of notes

    def __init__(self):
        # weighting
        ton = 1.000  # tonic, unison, fundamental
        sub = 0.500  # sub dominant
        dom = 0.750  # dominant
        nct = 0.000  # non-chord tone
        ict = 0.100  # (in) chord tone

        self.modes = {}          #  Uni  m2   M2   m3   M3   P4   TT   P5   m6   M6   m7   M7
        self.modes['ionian'    ] = [ton, nct, ict, nct, sub, ict, nct, dom, nct, ict, nct, ict]    # 'major'
        #self.modes['dorian'    ] = [ton, nct, ict, sub, nct, ict, nct, dom, nct, ict, ict, nct]
        #self.modes['phrygian'  ] = [ton, ict, nct, sub, nct, ict, nct, dom, ict, nct, ict, nct]
        #self.modes['lydian'    ] = [ton, nct, ict, nct, sub, nct, ict, dom, nct, ict, nct, ict]
        #self.modes['mixolydian'] = [ton, nct, ict, nct, sub, ict, nct, dom, nct, ict, ict, nct]
        self.modes['aeolian'   ] = [ton, nct, ict, sub, nct, ict, nct, dom, ict, nct, ict, nct]    # 'minor'
        #self.modes['locrian'   ] = [ton, ict, nct, sub, nct, ict, dom, nct, ict, nct, ict, nct]

        self.sharps = ['A ','A#','B ','C ','C#','D ','D#','E ','F ','F#','G ','G#']
        self.flats  = ['A ','Bb','B ','C ','Db','D ','Eb','E ','F ','Gb','G ','Ab']

    def get_tonal_center(self, found_notes):
        # work in progress, this isnt working well at the moment
        score_sheet = {}
        series = found_notes * 2
        for note in range(0, 12):
            TONAL_CENTER = self.sharps[note]
            sub_series = series[note:note+12]
            for TONAL_MODE in self.modes.keys():
                distribution = self.modes[TONAL_MODE]
                score = 0.0
                for i in range(0, 12):
                    # no note found in scale
                    if sub_series[i] == 0:
                        continue
                    score += distribution[i]
                score_sheet['%s %s' % (TONAL_CENTER, TONAL_MODE)] = score

        sorted_scores = sorted(score_sheet.iteritems(), key=operator.itemgetter(1))  # power sort
        return sorted_scores[-1]


class StudioInput(object):
    """Class to process audio from the studio mixer console."""

    def __init__(self, rate=44100, samples=2048, use_sharps=True):
        """ Initialize DSP Settings ..."""
        # Audio channels to process (1: mono, 2: stereo)
        self.CHANNELS = 1

        # Sampler rate (44.1kHz is the default, max is 96kHz),
        self.SAMPLING_RATE = rate

        # Number of samples to take at a time.
        self.NUM_SAMPLES = samples

        # length of sampled audio (in seconds)
        self.TIME_UNIT = float(self.NUM_SAMPLES) / float(self.SAMPLING_RATE)

        # a slice of a fourier transform that pertains only to the positive y-axis
        self.POSITIVE_AXIS = slice(None, self.NUM_SAMPLES / 2)

        # an array mapping fft indexes to frequencies based on sampling_rate, num_samples limited to the positive y-axis
        self.FREQUENCY_MAP = nfft.fftfreq(self.NUM_SAMPLES, 1 / float(self.SAMPLING_RATE))[self.POSITIVE_AXIS]

        # Equal temperment reference pitch for A4 (default 440Hz)
        self.REFERENCE_PITCH = 440.0   # A 4

        # cents range: 100c = half step, 50c = quarter step, 25c = eight step, 12.5c = 16th step
        self.CENTS_RANGE = 25

        # indicate note names as sharps or flats
        self.USE_SHARPS = use_sharps

        # setup audio streams
        self.pa = pyaudio.PyAudio()
        self.audio_data = []
        self.fft_data = []
        self.DFT_BINS = []
        self.NOTE_NAMES = []
        self._get_notes()

        # LED stuff
        #self.socket = socket(AF_INET, SOCK_STREAM)
        #self.socket.connect((HOSTNAME, PORT))
        self.ser = serial.Serial(COMM_PORT, 9600)
        self.LAST_SENT_NOTE = None

        self._ctr = 0
        self._mode = 1

        self.audio_stream = self.pa.open(format=pyaudio.paInt16, channels=self.CHANNELS, rate=self.SAMPLING_RATE,
                                         input=True, frames_per_buffer=self.NUM_SAMPLES)

    def update_sample(self):
        """get NUM_SAMPLES samples of audio data expressed as an Int16."""
        self.audio_data = fromstring(self.audio_stream.read(self.NUM_SAMPLES), dtype=short)
        self._fft()

    def _fft(self):
        # unsmoothed
        if self._mode == 0:
            normalized_data = self.audio_data / 32768.0

        # hamming
        if self._mode == 1:
            normalized_data = (self.audio_data * numpy.hamming(self.NUM_SAMPLES)) / 32768.0

        # blackman
        if self._mode == 2:
            normalized_data = (self.audio_data * numpy.blackman(self.NUM_SAMPLES)) / 32768.0

        _fft = abs(fft(normalized_data))
        self.fft_data = _fft[self.POSITIVE_AXIS]

    def _orig_fft(self):
        """Return DFT (cooley turkey) of normalized audio data."""
        ## audio_data will be a NUM_SAMPLES sized array of signed 16bit integers (-32767 -> 32767)

        # normalize audio power by reducing it to a range between:  0 < n < 1
        normalized_data = self.audio_data / 32768.0

        # take the absolute value of the FFT of the normalized audio data
        _fft = abs(fft(normalized_data))

        # remove and discard fft data from negative X axis (we only need positive x-axis data)
        self.fft_data = _fft[self.POSITIVE_AXIS]

    def _get_notes(self):
        """ precalculate note names and dft bin mapping."""
        # Pre-calculated values
        sharps = ['A','A#','B','C','C#','D','D#','E','F','F#','G','G#']
        flats  = ['A','Bb','B','C','Db','D','Eb','E','F','Gb','G','Ab']
        cent_ratio  = math.pow(2, 1/1200)
        dft_bandwidth = self.FREQUENCY_MAP[1]
        notes = {}
        for note_number in range(0, 88):

            # calculate base frequency, and min/max to account for tuning errors
            equal_temperment_frequency = self.REFERENCE_PITCH * math.pow(2, ((note_number - 49) / 12))
            note_freq_max = equal_temperment_frequency * math.pow(cent_ratio, (self.CENTS_RANGE *  1))
            note_freq_min = equal_temperment_frequency * math.pow(cent_ratio, (self.CENTS_RANGE * -1))

            # lock in target DFT frequency
            target_dft_slot = int(equal_temperment_frequency / dft_bandwidth)
            target_lo = dft_bandwidth * target_dft_slot
            target_hi = dft_bandwidth * (target_dft_slot + 1)
            if (equal_temperment_frequency - target_lo) > (target_hi - equal_temperment_frequency):
                # next slot up is closer to target frequency, round up!
                target_dft_slot = target_dft_slot + 1

            # does target_dft fall within cents range?
            dft_frequency = self.FREQUENCY_MAP[target_dft_slot]

            if (dft_frequency < note_freq_min) or (dft_frequency > note_freq_max):
                # NOT a pitch match!
                continue

            # evaluate closeness to dft frequency
            dft_closeness = equal_temperment_frequency / self.FREQUENCY_MAP[target_dft_slot]

            # compare previous match to current match, take best one!
            prev_match = notes.get(target_dft_slot, {})

            # current match is better
            if prev_match.get('dft_closeness', 999) > dft_closeness:

                # determine name of Note (A 0, A 1, A# vs Bb, etc...
                octave = int((note_number + 8) / 12)
                note = (note_number - 1) % 12     # A = 0, A#/Bb = 1, B = 2, ...
                if self.USE_SHARPS:
                    note_name = sharps[note]
                else:
                    note_name = flats[note]

                # record note:
                notes[target_dft_slot] = {
                    'dft_closeness': dft_closeness,
                    'base_freq': equal_temperment_frequency,
                    'note_number': note_number,
                    'note': note,
                    'dft_bin_number': target_dft_slot,
                    'note_number': note_number,
                    'note_name': note_name,
                    'note_octave': octave,
                    'note_fullname': '%2s%s' % (note_name, octave),
                }
        bins = notes.keys()
        bins.sort()

        self.DFT_BINS = bins
        self.NOTE_NAMES = notes

        """
        Notes:

        Soprano:       C4 (40) -> C6 (66)
        Mezzo-soprano: A3 (37) -> A5 (61)
        Contralto:     F3 (33) -> F5 (57)
        Tenor:         C3 (28) -> C5 (52)
        Baritone:      F2 (21) -> F4 (45)
        Bass:          E2 (20) -> E4 (44)
        Any:           E2 (20) -> C6 (66)
        """

    def update_lights(self, note_name):
        # skiping this for now
        msg = None
        if note_name == 'A':   msg='b'
        if note_name == 'A#':  msg='c'
        if note_name == 'Bb':  msg='c'
        if note_name == 'B':   msg='d'
        if note_name == 'C':   msg='e'
        if note_name == 'Db':  msg='f'
        if note_name == 'C#':  msg='f'
        if note_name == 'D':   msg='g'
        if note_name == 'Eb':  msg='h'
        if note_name == 'D#':  msg='h'
        if note_name == 'E':   msg='i'
        if note_name == 'F':   msg='j'
        if note_name == 'Gb':  msg='k'
        if note_name == 'F#':  msg='k'
        if note_name == 'G':   msg='l'
        if note_name == 'Ab':  msg='m'
        if note_name == 'G#':  msg='m'
        if not msg:
            return

        if self.LAST_SENT_NOTE != note_name:
            self.LAST_SENT_NOTE = note_name
            #self.socket.send(msg)
            self.ser.write(msg)
    def clear(self):
        os.system('clear')

    def print_score(self):
        sensitivity = 0.1
        found = {}
        for dft_slot in self.DFT_BINS:
            # filter out notes below C4
            if dft_slot < 33:
                continue
            found[dft_slot] = self.fft_data[dft_slot]

        sorted_power_found = sorted(found.iteritems(), key=operator.itemgetter(1))  # power sort
        sorted_found = sorted(found.iteritems(), key=operator.itemgetter(0))   # number sort
        hi_slot, hi_power = sorted_power_found[-1]



        # filter out noise (avg power < 0.005)
        if self.fft_data[0] < 0.005:
            hi_power = 100


        display_mode = 1  #  0: bars, 1: scroll

#        if display_mode == 0:
#            self.clear()
#
#            print '%-4s : %s    %-2s <-> %-2s' % (0, self.fft_data[0], self._ctr, self._mode)
#            for dft_slot, dft_power in sorted_found:
#                slots = int(dft_power / (hi_power / 65))
#                if slots < int(65 * 0.33):
#                    slots = 0
#
#                note  = self.NOTE_NAMES[dft_slot]['note_fullname']
#                s = ''.join(['*' for i in range(0, slots)])
#                print '%-4s : %-4s : %s' % (dft_slot, note, s)

        if display_mode == 1:
            sensitivity = 1.0
            score = []
            for note in range(0, 88):
                score.append('-')

            dft_slot, dft_power = sorted_power_found[-1]
            if dft_power < sensitivity:
                note_name       = '--'
            else:
                note            = self.NOTE_NAMES[dft_slot]
                note_num        = note['note_number']
                note_name       = note['note_name']
                score[note_num] = 'X'

            print ' %-9s  %-2s  %s' % (str(datetime.utcnow())[14:23], note_name,
                                       ''.join(score)[33:])

            # update led lights
            #dft_slot, dft_power = sorted_power_found[0]
            if note_name == '--':
                return
            note            = self.NOTE_NAMES[dft_slot]
            note_num        = note['note_number']
            note_name       = note['note_name']
            #print note['note_fullname']
            self.update_lights(note_name)

audio = StudioInput(44100, 4000)
print '--------------------------------------------------------------'
print 'Sample Rate: %s' % audio.SAMPLING_RATE
print '    Samples: %s' % audio.NUM_SAMPLES
print ' Delay Loop: %2.5f sec/iteration' % audio.TIME_UNIT
print 'Active Note: %s' % len(audio.DFT_BINS)
print ''
print ''
print ''

while True:
    audio.update_sample()
    audio.print_score()

