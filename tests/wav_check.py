"""wav_check.py file.wav start_s end_s -> prints the loudness and pitch of that stretch."""
import struct, sys, wave
w = wave.open(sys.argv[1])
rate = w.getframerate()
data = struct.unpack("<%dh" % w.getnframes(), w.readframes(w.getnframes()))
a, b = int(float(sys.argv[2]) * rate), int(float(sys.argv[3]) * rate)
part = data[a:b]
peak = max(abs(s) for s in part) if part else 0
ups = sum(1 for i in range(1, len(part)) if part[i - 1] < 0 <= part[i])
print("peak %d freq %.0f" % (peak, ups * rate / max(1, len(part))))
