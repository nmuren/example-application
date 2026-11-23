// SPDX-License-Identifier: MIT
// Minimal nRF52833 PWM0 bus model and BBC micro:bit V2 matrix observer.
// This sees the *physical* Nordic Zephyr display driver's PWM sequence writes;
// the firmware does not know about a new fake register or UART protocol.
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class MicrobitPWM : IDoubleWordPeripheral, IKnownSize
    {
        public MicrobitPWM(IMachine machine)
        {
            this.machine = machine;
            udp = new UdpClient();
            Reset();
        }

        public long Size { get { return 0x1000; } }
        public int OutputPort { get; set; } = 60033;
        public uint LastPixels { get; private set; }
        public int LastBrightness { get; private set; }
        public int CompletedFrames { get; private set; }
        public int SequenceStarts { get; private set; }
        public uint SeenPixels { get { return seenPixels; } }
        public uint RowsSeen { get; private set; }
        public uint LastRowOutputs { get; private set; }

        public void Reset()
        {
            registers.Clear();
            for(var i = 0; i < channelPins.Length; ++i) channelPins[i] = 0xFFFFFFFF;
            enabled = false;
            sequencePointer = 0;
            sequenceCount = 0;
            framePixels = 0;
            seenPixels = 0;
            frameBrightness = 0;
            LastPixels = 0;
            LastBrightness = 255;
            CompletedFrames = 0;
            SequenceStarts = 0;
            RowsSeen = 0;
            LastRowOutputs = 0;
            emitted = false;
            stableCycles = 0;
            candidatePixels = 0;
            candidateBrightness = 0;
        }

        public uint ReadDoubleWord(long offset)
        {
            uint value;
            return registers.TryGetValue(offset, out value) ? value : 0;
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            registers[offset] = value;
            switch(offset)
            {
                case 0x500: enabled = value != 0; break; // ENABLE
                case 0x520: sequencePointer = value; break; // SEQ[0].PTR
                case 0x524: sequenceCount = value; break;   // SEQ[0].CNT
                case 0x560: case 0x564: case 0x568: case 0x56C:
                    channelPins[(offset - 0x560) / 4] = value; // PSEL.OUT[n]
                    break;
                case 0x008: // TASKS_SEQSTART[0]
                    if(value != 0 && enabled) { SequenceStarts++; CaptureSequence(); }
                    break;
            }
        }

        private void CaptureSequence()
        {
            if(sequencePointer == 0 || sequenceCount == 0) return;
            var rowOutputs = machine.SystemBus.ReadDoubleWord(0x50000504);
            LastRowOutputs = rowOutputs;
            var row = -1;
            for(var i = 0; i < rowPins.Length; ++i)
            {
                if((rowOutputs & (1u << rowPins[i])) != 0)
                {
                    row = i;
                    break;
                }
            }
            if(row < 0) return;
            RowsSeen |= 1u << row;

            for(var channel = 0; channel < channelPins.Length && channel < sequenceCount; ++channel)
            {
                var pin = channelPins[channel];
                if((pin & 0x80000000) != 0) continue; // disconnected
                var column = Array.IndexOf(columnPins, (int)(pin & 0x3F));
                if(column < 0) continue;
                var index = row * 5 + column;
                var mask = 1u << index;
                var sample = machine.SystemBus.ReadWord(sequencePointer + (uint)(channel * 2));
                var pulse = sample & 0x7FFF;
                uint counterTop;
                var quantum = registers.TryGetValue(0x508, out counterTop)
                    ? Math.Max(1, (int)(counterTop & 0x7FFF) / 255) : 62;
                if(pulse > 0 && (sample & 0x8000) == 0)
                {
                    framePixels |= mask;
                    frameBrightness = Math.Max(frameBrightness,
                        Math.Min(255, (pulse + quantum / 2) / quantum));
                }
                else framePixels &= ~mask;
                seenPixels |= mask;
            }
            if(seenPixels != AllPixels) return;
            seenPixels = 0;
            if(frameBrightness > 0) LastBrightness = frameBrightness;
            frameBrightness = 0;
            LastPixels = framePixels;
            CompletedFrames++;
            // A Zephyr display_write can modify the framebuffer between two
            // scan slots. Wait for two identical complete scans, so the UI
            // never shows a half-updated heart/blank pattern.
            if(LastPixels == candidatePixels && LastBrightness == candidateBrightness) stableCycles++;
            else { candidatePixels = LastPixels; candidateBrightness = LastBrightness; stableCycles = 1; }
            if(stableCycles < 2) return;
            if(emitted && LastPixels == lastEmittedPixels && LastBrightness == lastEmittedBrightness) return;
            emitted = true;
            lastEmittedPixels = LastPixels;
            lastEmittedBrightness = LastBrightness;
            var line = string.Format("@MB2:{0:x8}:{1}:0\n", LastPixels, LastBrightness);
            var data = Encoding.ASCII.GetBytes(line);
            try { udp.Send(data, data.Length, new IPEndPoint(IPAddress.Loopback, OutputPort)); }
            catch(SocketException) { /* UI receiver is optional. */ }
        }

        private readonly IMachine machine;
        private readonly UdpClient udp;
        private readonly Dictionary<long, uint> registers = new Dictionary<long, uint>();
        private readonly uint[] channelPins = new uint[4];
        private readonly int[] rowPins = { 21, 22, 15, 24, 19 };
        private readonly int[] columnPins = { 28, 11, 31, 37, 30 }; // P1.5 = 32+5
        private const uint AllPixels = (1u << 25) - 1;
        private bool enabled;
        private uint sequencePointer;
        private uint sequenceCount;
        private uint framePixels;
        private uint seenPixels;
        private int frameBrightness;
        private bool emitted;
        private int stableCycles;
        private uint candidatePixels;
        private int candidateBrightness;
        private uint lastEmittedPixels;
        private int lastEmittedBrightness;
    }
}

namespace Antmicro.Renode.Peripherals.Timers
{
    // Model the Nordic TIMER4 COMPARE0_STOP + COMPARE0_CLEAR shortcuts needed
    // by Zephyr's matrix scan; without them only the first row is scanned.
    public class MicrobitTimer4 : NRF52840_Timer
    {
        public MicrobitTimer4(IMachine machine, int numberOfEvents) : base(machine, numberOfEvents)
        {
            EventTriggered += OnEventTriggered;
        }

        public override void Reset()
        {
            base.Reset();
            shortcuts = 0;
        }

        public override void WriteDoubleWord(long offset, uint value)
        {
            if(offset == 0x200) { shortcuts = value; return; }
            base.WriteDoubleWord(offset, value);
        }

        public override uint ReadDoubleWord(long offset)
        {
            return offset == 0x200 ? shortcuts : base.ReadDoubleWord(offset);
        }

        private void OnEventTriggered(uint offset)
        {
            if(offset != 0x140) return; // COMPARE[0]
            if((shortcuts & 0x100u) != 0) base.WriteDoubleWord(0x004, 1); // STOP
            if((shortcuts & 0x001u) != 0) base.WriteDoubleWord(0x00C, 1); // CLEAR
        }

        private uint shortcuts;
    }
}
