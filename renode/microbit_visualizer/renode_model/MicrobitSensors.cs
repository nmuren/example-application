// SPDX-License-Identifier: MIT
// nRF52833 TWIM0 + the BBC micro:bit V2's LSM303AGR sensor pair.
// Register-level model: an unchanged Zephyr board ELF uses its normal nrfx
// EasyDMA I2C controller and ST lis2dh/lis2mdl sensor drivers.
using System;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public class MicrobitSensors : IDoubleWordPeripheral, IKnownSize
    {
        public MicrobitSensors(IMachine machine)
        {
            this.machine = machine;
            IRQ = new GPIO();
            Reset();
        }

        public long Size { get { return 0x1000; } }
        public GPIO IRQ { get; private set; }

        // Monitor: twim0 SetOrientation 90 0 0
        // Body axes: +X right, +Y up, +Z out of front face. Angles in degrees:
        // yaw about +Z, pitch about +Y, roll about +X (right-handed, Z-Y-X).
        // At (0,0,0) acceleration is +1g Z; field is (0.25,0.05,-0.38) G.
        // Rotate world vectors into the board frame with the inverse rotation.
        public void SetOrientation(int yaw, int pitch, int roll)
        {
            lock(sync)
            {
                yawDegrees = yaw % 360;
                pitchDegrees = pitch % 360;
                rollDegrees = roll % 360;
            }
        }

        public int YawDegrees { get { lock(sync) { return yawDegrees; } } }
        public int PitchDegrees { get { lock(sync) { return pitchDegrees; } } }
        public int RollDegrees { get { lock(sync) { return rollDegrees; } } }
        public int Transfers { get { lock(sync) { return transfers; } } }

        public void Reset()
        {
            lock(sync)
            {
                registers.Clear();
                Array.Clear(accelRegisters, 0, accelRegisters.Length);
                Array.Clear(magnRegisters, 0, magnRegisters.Length);
                accelRegisters[0x0F] = 0x33; // LIS2DH / LSM303AGR accelerometer WHO_AM_I
                magnRegisters[0x4F] = 0x40;  // LIS2MDL / LSM303AGR magnetometer WHO_AM_I
                accelPointer = magnPointer = 0;
                accelIncrement = false;
                enabled = false;
                shorts = inten = errorSource = 0;
                txPointer = rxPointer = 0;
                txCount = rxCount = txAmount = rxAmount = 0;
                address = 0;
                yawDegrees = pitchDegrees = rollDegrees = 0;
                transfers = 0;
                IRQ.Set(false);
            }
        }

        public uint ReadDoubleWord(long offset)
        {
            lock(sync)
            {
                switch(offset)
                {
                    case 0x200: return shorts;
                    case 0x300: case 0x304: case 0x308: return inten;
                    case 0x4C4: return errorSource;
                    case 0x500: return enabled ? 6u : 0u;
                    case 0x53C: return rxAmount;
                    case 0x54C: return txAmount;
                    case 0x588: return address;
                    default:
                        uint result;
                        return registers.TryGetValue(offset, out result) ? result : 0;
                }
            }
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            lock(sync)
            {
                if(IsEvent(offset))
                {
                    // Nordic event registers are cleared by writing zero.
                    if(value == 0) registers[offset] = 0;
                    UpdateIRQ();
                    return;
                }
                switch(offset)
                {
                    case 0x000: if(value != 0) Start(false); return; // STARTRX
                    case 0x008: if(value != 0) Start(true); return;  // STARTTX
                    case 0x014: if(value != 0) Event(0x104); return; // STOP
                    case 0x01C: if(value != 0) Event(0x148); return; // SUSPEND
                    case 0x020: return; // RESUME, next START task advances
                    case 0x200: shorts = value; return;
                    case 0x300: inten = value; UpdateIRQ(); return;
                    case 0x304: inten |= value; UpdateIRQ(); return;
                    case 0x308: inten &= ~value; UpdateIRQ(); return;
                    case 0x4C4: errorSource &= ~value; return; // write-one-to-clear
                    case 0x500: enabled = value == 6; registers[offset] = value; return;
                    case 0x534: rxPointer = value; break;
                    case 0x538: rxCount = value & 0xFFFF; break;
                    case 0x544: txPointer = value; break;
                    case 0x548: txCount = value & 0xFFFF; break;
                    case 0x588: address = value & 0x7F; break;
                }
                registers[offset] = value;
            }
        }

        private void Start(bool transmit)
        {
            if(!enabled) return;
            transfers++;
            var count = transmit ? txCount : rxCount;
            var pointer = transmit ? txPointer : rxPointer;
            // The BBC micro:bit V2 has 128 KiB of RAM at 0x20000000.
            // Reject malformed DMA requests rather than crossing RAM or allocating
            // arbitrary buffers from a guest-controlled register value.
            if(count > 4096 || pointer < 0x20000000 ||
               pointer > 0x20020000 || count > 0x20020000u - pointer)
            {
                Fail(1); // OVERRUN
                return;
            }
            if(address != 0x19 && address != 0x1E)
            {
                Fail(2); // ANACK
                return;
            }
            if(transmit)
            {
                Event(0x150); // TXSTARTED
                var data = new byte[(int)count];
                for(var i = 0; i < data.Length; ++i)
                    data[i] = machine.SystemBus.ReadByte((ulong)(pointer + (uint)i));
                WriteSensor(data);
                txAmount = count;
                Event(0x160); // LASTTX
                if((shorts & (1u << 7)) != 0) Start(false); // LASTTX_STARTRX
                else if((shorts & (1u << 8)) != 0) Event(0x148); // SUSPEND
                else if((shorts & (1u << 9)) != 0) Event(0x104); // STOP
            }
            else
            {
                Event(0x14C); // RXSTARTED
                for(var i = 0u; i < count; ++i)
                    machine.SystemBus.WriteByte((ulong)(pointer + i), ReadSensor());
                rxAmount = count;
                Event(0x15C); // LASTRX
                if((shorts & (1u << 12)) != 0) Event(0x104); // STOP
                else if((shorts & (1u << 11)) != 0) Event(0x148); // SUSPEND
            }
        }

        private void Fail(uint code)
        {
            errorSource |= code;
            Event(0x124); // ERROR
            Event(0x104); // STOPPED, as after an actual failed bus transaction
        }

        private void WriteSensor(byte[] data)
        {
            if(data.Length == 0) return;
            var accel = address == 0x19;
            // LIS2DH's bit 7 selects multi-byte register auto-increment on I2C;
            // LIS2MDL uses a plain register pointer and auto-increments normally.
            if(accel)
            {
                accelIncrement = (data[0] & 0x80) != 0;
                accelPointer = data[0] & 0x7F;
            }
            else magnPointer = data[0];
            for(var i = 1; i < data.Length; ++i)
            {
                if(accel)
                {
                    if(accelPointer != 0x0F) accelRegisters[accelPointer] = data[i];
                    if(accelIncrement) accelPointer = (accelPointer + 1) & 0x7F;
                }
                else
                {
                    if(magnPointer != 0x4F) magnRegisters[magnPointer] = data[i];
                    // The LIS2MDL soft-reset bit self-clears. Zephyr sets it at init
                    // then immediately configures BDU/ODR via read-modify-write.
                    if(magnPointer == 0x60) magnRegisters[0x60] &= 0xDF;
                    magnPointer = (magnPointer + 1) & 0xFF;
                }
            }
        }

        private byte ReadSensor()
        {
            if(address == 0x19)
            {
                var reg = accelPointer;
                var result = ReadAccel(reg);
                if(accelIncrement) accelPointer = (accelPointer + 1) & 0x7F;
                return result;
            }
            else
            {
                var reg = magnPointer;
                var result = ReadMagn(reg);
                magnPointer = (magnPointer + 1) & 0xFF;
                return result;
            }
        }

        private byte ReadAccel(int reg)
        {
            if(reg == 0x27) return 0x0F; // X/Y/Z new data ready
            if(reg >= 0x28 && reg <= 0x2D)
            {
                // LIS2DH ±2g full-scale is 16384 raw counts per g in the
                // left-aligned 16-bit register pair; the FS bits widen range.
                var fs = (accelRegisters[0x23] >> 4) & 3;
                var range = new int[] { 2, 4, 8, 16 }[fs];
                var gravity = BodyVector(0, 0, 1);
                var raw = ToInt16(gravity[(reg - 0x28) / 2] * (32768.0 / range));
                return (byte)((reg & 1) == 0 ? raw & 0xFF : (raw >> 8) & 0xFF);
            }
            return accelRegisters[reg];
        }

        private byte ReadMagn(int reg)
        {
            if(reg == 0x67) return 0x08; // XYZ data ready
            if(reg >= 0x68 && reg <= 0x6D)
            {
                // LIS2MDL sensitivity: 1.5 mG/LSB (0.0015 G/LSB).
                var field = BodyVector(0.25, 0.05, -0.38);
                var raw = ToInt16(field[(reg - 0x68) / 2] / 0.0015);
                return (byte)((reg & 1) == 0 ? raw & 0xFF : (raw >> 8) & 0xFF);
            }
            if(reg == 0x6E || reg == 0x6F) return 0; // room temperature baseline
            return magnRegisters[reg];
        }

        private double[] BodyVector(double x, double y, double z)
        {
            var yaw = yawDegrees * Math.PI / 180.0;
            var pitch = pitchDegrees * Math.PI / 180.0;
            var roll = rollDegrees * Math.PI / 180.0;
            var cy = Math.Cos(yaw); var sy = Math.Sin(yaw);
            var cp = Math.Cos(pitch); var sp = Math.Sin(pitch);
            var cr = Math.Cos(roll); var sr = Math.Sin(roll);
            // Inverse of Rz(yaw) * Ry(pitch) * Rx(roll): world -> body.
            var a = cy * x + sy * y;
            var b = -sy * x + cy * y;
            var c = cp * a - sp * z;
            var d = sp * a + cp * z;
            return new double[] { c, cr * b + sr * d, -sr * b + cr * d };
        }

        private static short ToInt16(double value)
        {
            return (short)Math.Max(short.MinValue, Math.Min(short.MaxValue, Math.Round(value)));
        }

        private static bool IsEvent(long offset)
        {
            return offset == 0x104 || offset == 0x124 || offset == 0x148 ||
                   offset == 0x14C || offset == 0x150 || offset == 0x15C || offset == 0x160;
        }

        private static int EventBit(long offset)
        {
            switch(offset)
            {
                case 0x104: return 1; case 0x124: return 9;
                case 0x148: return 18; case 0x14C: return 19;
                case 0x150: return 20; case 0x15C: return 23;
                case 0x160: return 24; default: return -1;
            }
        }

        private void Event(long offset)
        {
            registers[offset] = 1;
            UpdateIRQ();
        }

        private void UpdateIRQ()
        {
            foreach(var offset in new long[] { 0x104, 0x124, 0x148, 0x14C, 0x150, 0x15C, 0x160 })
            {
                uint value;
                if((inten & (1u << EventBit(offset))) != 0 &&
                   registers.TryGetValue(offset, out value) && value != 0)
                {
                    IRQ.Set(true);
                    return;
                }
            }
            IRQ.Set(false);
        }

        private readonly IMachine machine;
        private readonly object sync = new object();
        private readonly Dictionary<long, uint> registers = new Dictionary<long, uint>();
        private readonly byte[] accelRegisters = new byte[256];
        private readonly byte[] magnRegisters = new byte[256];
        private int accelPointer, magnPointer;
        private bool accelIncrement, enabled;
        private uint shorts, inten, errorSource, address;
        private uint txPointer, rxPointer, txCount, rxCount, txAmount, rxAmount;
        private int yawDegrees, pitchDegrees, rollDegrees, transfers;
    }
}
