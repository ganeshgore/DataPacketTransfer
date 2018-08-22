import csv
import random
from struct import pack

START_SYMBOL = 3
STOP_SYMBOL = 2

with open('PacketSequence.csv', 'r') as f:
    reader = csv.reader(f)
    PacketList = list(reader)


InputPacketsFp = open("InputPackets.txt", "w")
FiFoEscapeSequenceFp = open("FiFoEscapeSequence.txt", "w")
FiFoEscapeSequenceWithZerosFp = open("FiFoEscapeSequenceWithZeros.txt", "w")
OutputPacketsFp = open("OutputPackets.txt", "w")

InputPacketsFpb = open("InputPacketsBin.bin", "wb")
FiFoEscapeSequenceFpb = open("FiFoEscapeSequenceBin.bin", "wb")
FiFoEscapeSequenceWithZerosFpb = open(
    "FiFoEscapeSequenceWithZerosBin.bin", "wb")
OutputPacketsFpb = open("OutputPacketsBin.bin", "wb")

for Index, (In, Len, Type) in enumerate(PacketList):
    print "Processing Packet ", In, Len, Type,
    Len = int(Len)
    In = int(In)
    data = range(Len) if Type == "Sequence" else \
        range(Len, 0, -1) if Type == "RSequence" else \
        random.sample(range(0, 255) * 8, Len) if Type == "Random" else []

    data = [(each % 256) for each in data]
    dataString = " ".join("%02X" % (each) for each in data)

    # Write First and Final File
    InputPacketsFp.write("%04X\n%04X\n%s\n" % (In, Len, dataString))
    InputPacketsFpb.write(
        bytearray([In >> 8, In & 0xFF] + [Len >> 8, Len & 0xFF] + data))
    OutputPacketsFp.write("%04X\n%04X\n%s\n" % (In, Len, dataString))
    OutputPacketsFpb.write(
        bytearray([In >> 8, In & 0xFF] + [Len >> 8, Len & 0xFF] + data))

    # Add first word to stream
    FirstWord = "%08X" % ((In << 16) | (Len))
    for i in reversed(range(4)):
        data.insert(0, int(FirstWord[i * 2:(i * 2) + 2], 16))

    # Escape Stream
    iterData = enumerate(data)
    for Idx, each in iterData:
        if each in [START_SYMBOL, STOP_SYMBOL]:
            data.insert(Idx + 1, each)
            iterData.next()

    # Write Second File
    FiFoEscapeSequenceFp.write(("%02X\n00\n" % START_SYMBOL))
    for Idx, each in enumerate(data):
        FiFoEscapeSequenceFp.write("%02X\n" % each)
    FiFoEscapeSequenceFp.write(("%02X\n00\n" % STOP_SYMBOL))

    # ################### Write First File Addded Zeroes ###########
    AddedZerosNo = (0 if (In < 5) else random.randint(1, 40))

    FiFoEscapeSequenceWithZerosFp.write(("%02X\n00\n" % START_SYMBOL))
    for Idx, each in enumerate(data):
        FiFoEscapeSequenceWithZerosFp.write("%02X\n" % each)
    FiFoEscapeSequenceWithZerosFp.write(("%02X\n00\n" % STOP_SYMBOL))
    FiFoEscapeSequenceWithZerosFp.write("00\n" * AddedZerosNo)

    # Write First File Addded Zeroes  - Binary
    FiFoEscapeSequenceWithZerosFpb.write(bytearray([START_SYMBOL, 0]))
    for Idx, each in enumerate(data):
        FiFoEscapeSequenceWithZerosFpb.write(bytearray([each]))
    FiFoEscapeSequenceWithZerosFpb.write(bytearray([STOP_SYMBOL, 0]))
    FiFoEscapeSequenceWithZerosFpb.write(bytearray([0] * AddedZerosNo))

    print "Escape Length %d" % len(data), FirstWord, "Done"

InputPacketsFp.close()
FiFoEscapeSequenceFp.close()
FiFoEscapeSequenceWithZerosFp.close()
OutputPacketsFp.close()

InputPacketsFpb.close()
FiFoEscapeSequenceFpb.close()
FiFoEscapeSequenceWithZerosFpb.close()
OutputPacketsFpb.close()
