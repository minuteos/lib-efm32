/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/PRS.h
 */

#pragma once

#include <base/base.h>

#include <hw/CMU.h>
#include <hw/GPIO.h>

#ifndef PRS_1MHZ_DMA
#define PRS_1MHZ_DMA	0
#endif

class PRSChannel : public PRS_CH_TypeDef
{
public:
    enum Flags
    {
        SourceNone = 0,

        SourcePRS0 = PRS_CH_CTRL_SOURCESEL_PRSL | PRS_CH_CTRL_SIGSEL_PRSCH0,
        SourcePRS1, SourcePRS2, SourcePRS3, SourcePRS4, SourcePRS5, SourcePRS6, SourcePRS7,

        SourcePRS8 = PRS_CH_CTRL_SOURCESEL_PRSH | PRS_CH_CTRL_SIGSEL_PRSCH8,
        SourcePRS9, SourcePRS10, SourcePRS11,

        SourceADCMP0 = PRS_CH_CTRL_SOURCESEL_ACMP0 | PRS_CH_CTRL_SIGSEL_ACMP0OUT,
        SourceADCMP1 = PRS_CH_CTRL_SOURCESEL_ACMP1 | PRS_CH_CTRL_SIGSEL_ACMP1OUT,

        SourceADC0_Single = PRS_CH_CTRL_SOURCESEL_ADC0 | PRS_CH_CTRL_SIGSEL_ADC0SINGLE,
        SourceADC0_Scan = PRS_CH_CTRL_SOURCESEL_ADC0 | PRS_CH_CTRL_SIGSEL_ADC0SCAN,

        SourceUSART0_IRTx = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0IRTX,
        SourceUSART0_TxComplete = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0TXC,
        SourceUSART0_DataValid = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0RXDATAV,
        SourceUSART0_Rts = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0RTS,
        SourceUSART0_Tx = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0TX,
        SourceUSART0_Cs = PRS_CH_CTRL_SOURCESEL_USART0 | PRS_CH_CTRL_SIGSEL_USART0CS,

        SourceUSART1_TxComplete = PRS_CH_CTRL_SOURCESEL_USART1 | PRS_CH_CTRL_SIGSEL_USART1TXC,
        SourceUSART1_DataValid = PRS_CH_CTRL_SOURCESEL_USART1 | PRS_CH_CTRL_SIGSEL_USART1RXDATAV,
        SourceUSART1_Rts = PRS_CH_CTRL_SOURCESEL_USART1 | PRS_CH_CTRL_SIGSEL_USART1RTS,
        SourceUSART1_Tx = PRS_CH_CTRL_SOURCESEL_USART1 | PRS_CH_CTRL_SIGSEL_USART1TX,
        SourceUSART1_Cs = PRS_CH_CTRL_SOURCESEL_USART1 | PRS_CH_CTRL_SIGSEL_USART1CS,

        SourceUSART2_IRTx = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2IRTX,
        SourceUSART2_TxComplete = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2TXC,
        SourceUSART2_DataValid = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2RXDATAV,
        SourceUSART2_Rts = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2RTS,
        SourceUSART2_Tx = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2TX,
        SourceUSART2_Cs = PRS_CH_CTRL_SOURCESEL_USART2 | PRS_CH_CTRL_SIGSEL_USART2CS,

        SourceTIMER0_Underflow = PRS_CH_CTRL_SOURCESEL_TIMER0 | PRS_CH_CTRL_SIGSEL_TIMER0UF,
        SourceTIMER0_Overflow = PRS_CH_CTRL_SOURCESEL_TIMER0 | PRS_CH_CTRL_SIGSEL_TIMER0OF,
        SourceTIMER0_CC0 = PRS_CH_CTRL_SOURCESEL_TIMER0 | PRS_CH_CTRL_SIGSEL_TIMER0CC0,
        SourceTIMER0_CC1 = PRS_CH_CTRL_SOURCESEL_TIMER0 | PRS_CH_CTRL_SIGSEL_TIMER0CC1,

        SourceTIMER1_Underflow = PRS_CH_CTRL_SOURCESEL_TIMER1 | PRS_CH_CTRL_SIGSEL_TIMER1UF,
        SourceTIMER1_Overflow = PRS_CH_CTRL_SOURCESEL_TIMER1 | PRS_CH_CTRL_SIGSEL_TIMER1OF,
        SourceTIMER1_CC0 = PRS_CH_CTRL_SOURCESEL_TIMER1 | PRS_CH_CTRL_SIGSEL_TIMER1CC0,
        SourceTIMER1_CC1 = PRS_CH_CTRL_SOURCESEL_TIMER1 | PRS_CH_CTRL_SIGSEL_TIMER1CC1,

        SourceRTCC_CV0 = PRS_CH_CTRL_SOURCESEL_RTCC | PRS_CH_CTRL_SIGSEL_RTCCCCV0,
        SourceRTCC_CV1, SourceRTCC_CV2,

        SourceGPIO0 = PRS_CH_CTRL_SOURCESEL_GPIOL | PRS_CH_CTRL_SIGSEL_GPIOPIN0,
        SourceGPIO1, SourceGPIO2, SourceGPIO3, SourceGPIO4, SourceGPIO5, SourceGPIO6, SourceGPIO7,

        SourceGPIO8 = PRS_CH_CTRL_SOURCESEL_GPIOH | PRS_CH_CTRL_SIGSEL_GPIOPIN8,
        SourceGPIO9, SourceGPIO10, SourceGPIO11, SourceGPIO12, SourceGPIO13, SourceGPIO14, SourceGPIO15,

        SourceLETIMER0_CH0 = PRS_CH_CTRL_SOURCESEL_LETIMER0 | PRS_CH_CTRL_SIGSEL_LETIMER0CH0,
        SourceLETIMER0_CH1,

        SourcePCNT0_TrigComp = PRS_CH_CTRL_SOURCESEL_PCNT0 | PRS_CH_CTRL_SIGSEL_PCNT0TCC,
        SourcePCNT0_UnderOverflow = PRS_CH_CTRL_SOURCESEL_PCNT0 | PRS_CH_CTRL_SIGSEL_PCNT0UFOF,
        SourcePCNT0_Direction = PRS_CH_CTRL_SOURCESEL_PCNT0 | PRS_CH_CTRL_SIGSEL_PCNT0DIR,

        SourceCRYOTIMER = PRS_CH_CTRL_SOURCESEL_CRYOTIMER | PRS_CH_CTRL_SIGSEL_CRYOTIMERPERIOD,

        SourceCMU_Clock0 = PRS_CH_CTRL_SOURCESEL_CMU | PRS_CH_CTRL_SIGSEL_CMUCLKOUT0,
        SourceCMU_Clock1,

        EdgeNone = PRS_CH_CTRL_EDSEL_OFF,
        EdgeRising = PRS_CH_CTRL_EDSEL_POSEDGE,
        EdgeFalling = PRS_CH_CTRL_EDSEL_NEGEDGE,
        EdgeBoth = PRS_CH_CTRL_EDSEL_BOTHEDGES,
        EdgeMask = _PRS_CH_CTRL_EDSEL_MASK,

        Stretch = PRS_CH_CTRL_STRETCH,
        Invert = PRS_CH_CTRL_INV,
        OrPrev = PRS_CH_CTRL_ORPREV,
        AndNext = PRS_CH_CTRL_ANDNEXT,

        Async = PRS_CH_CTRL_ASYNC,
    };

    ALWAYS_INLINE unsigned Index() const { return ((unsigned)this & 0x3F) >> 2; }
    ALWAYS_INLINE void Setup(Flags flags) { CTRL = flags; }

    ALWAYS_INLINE void DMARequest(unsigned n) { (&PRS->DMAREQ0)[n] = Index() << _PRS_DMAREQ0_PRSSEL_SHIFT; }
};

class PRSChannelHandle
{
private:
    constexpr PRSChannelHandle(unsigned index) : index(index) {}

    unsigned index;

public:
    constexpr operator unsigned() const { return index; }

    ALWAYS_INLINE unsigned Index() const { return index; }
    ALWAYS_INLINE void Setup(PRSChannel::Flags flags) { PRS->CH[index].CTRL = flags; }

    ALWAYS_INLINE void DMARequest(unsigned n) { (&PRS->DMAREQ0)[n] = Index() << _PRS_DMAREQ0_PRSSEL_SHIFT; }

    friend class _PRS;
};

#undef PRS
#define PRS	CM_PERIPHERAL(_PRS, PRS_BASE)

class _PRS : public PRS_TypeDef
{
public:
#ifdef _SILICON_LABS_32B_SERIES_1
#ifdef _EFM32_GIANT_FAMILY
    void ConfigureOutput0(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 0,
            GPIO_LOC(pA(0), pF(3), pC(14), pF(2))); }
    void ConfigureOutput1(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 1,
            GPIO_LOC(pA(1), pF(4), pC(15), pE(12))); }
    void ConfigureOutput2(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 2,
            GPIO_LOC(pC(0), pF(5), pE(10), pE(13))); }
    void ConfigureOutput3(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 3,
            GPIO_LOC(pC(1), pE(8), pE(11), pA(0))); }
    void ConfigureOutput4(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 4,
            GPIO_LOC(pC(8), pB(0), pF(1))); }
    void ConfigureOutput5(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 5,
            GPIO_LOC(pC(9), pB(1), pD(6))); }
    void ConfigureOutput6(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 6,
            GPIO_LOC(pA(6), pB(14), pE(6))); }
    void ConfigureOutput7(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 7,
            GPIO_LOC(pB(13), pA(7), pE(7))); }
    void ConfigureOutput8(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 8,
            GPIO_LOC(pA(8), pA(2), pE(9))); }
    void ConfigureOutput9(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 9,
            GPIO_LOC(pA(9), pA(3), pB(10))); }
    void ConfigureOutput10(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 10,
            GPIO_LOC(pA(10), pC(2), pD(4))); }
    void ConfigureOutput11(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 11,
            GPIO_LOC(pA(11), pD(3), pD(5))); }
    void ConfigureOutput12(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 12,
            GPIO_LOC(pA(12), pB(6), pD(8))); }
    void ConfigureOutput13(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 13,
            GPIO_LOC(pA(13), pB(9), pE(14))); }
    void ConfigureOutput14(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 14,
            GPIO_LOC(pA(14), pC(6), pE(15))); }
    void ConfigureOutput15(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 15,
            GPIO_LOC(pA(15), pC(7), pF(0))); }
    void ConfigureOutput16(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 16,
            GPIO_LOC(pA(4), pB(12), pE(4))); }
    void ConfigureOutput17(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 17,
            GPIO_LOC(pA(5), pB(15), pE(5))); }
    void ConfigureOutput18(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 18,
            GPIO_LOC(pB(2), pC(10), pC(4))); }
    void ConfigureOutput19(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 19,
            GPIO_LOC(pB(3), pC(11), pC(5))); }
    void ConfigureOutput20(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 20,
            GPIO_LOC(pB(4), pC(12), pE(2))); }
    void ConfigureOutput21(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 21,
            GPIO_LOC(pB(5), pC(13), pB(11))); }
    void ConfigureOutput22(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 22,
            GPIO_LOC(pB(7), pE(0), pF(6))); }
    void ConfigureOutput23(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 23,
            GPIO_LOC(pB(8), pE(1), pF(7))); }
#else
    void ConfigureOutput0(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 0, 4,
            GPIO_LOC(pF(0), pF(1), pF(2), pF(3), pF(4), pF(5), pF(6), pF(7), pC(6), pC(7), pC(8), pC(9), pC(10), pC(11))); }
    void ConfigureOutput1(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 1, 5,
            GPIO_LOC(pF(1), pF(2), pF(3), pF(4), pF(5), pF(6), pF(7), pF(0))); }
    void ConfigureOutput2(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 2, 6,
            GPIO_LOC(pF(2), pF(3), pF(4), pF(5), pF(6), pF(7), pF(0), pF(1))); }
    void ConfigureOutput3(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 3, 7,
            GPIO_LOC(pF(3), pF(4), pF(5), pF(6), pF(7), pF(0), pF(1), pF(2), pD(9), pD(10), pD(11), pD(12), pD(13), pD(14), pD(15))); }
    void ConfigureOutput4(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 4, 8,
            GPIO_LOC(pD(9), pD(10), pD(11), pD(12), pD(13), pD(14), pD(15))); }
    void ConfigureOutput5(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 5, 9,
            GPIO_LOC(pD(10), pD(11), pD(12), pD(13), pD(14), pD(15), pD(9))); }
    void ConfigureOutput6(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 6, 10,
            GPIO_LOC(pA(0), pA(1), pA(2), pA(3), pA(4), pA(5), pB(11), pB(12), pB(13), pB(14), pB(15), pD(9), pD(10), pD(11), pD(12), pD(13), pD(14), pD(15))); }
    void ConfigureOutput7(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 7, 11,
            GPIO_LOC(pA(1), pA(2), pA(3), pA(4), pA(5), pB(11), pB(12), pB(13), pB(14), pB(15), pA(0))); }
    void ConfigureOutput8(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 8, 12,
            GPIO_LOC(pA(2), pA(3), pA(4), pA(5), pB(11), pB(12), pB(13), pB(14), pB(15), pA(0), pA(1))); }
    void ConfigureOutput9(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 9, 13,
            GPIO_LOC(pA(3), pA(4), pA(5), pB(11), pB(12), pB(13), pB(14), pB(15), pA(0), pA(1), pA(2), pC(6), pC(7), pC(8), pC(9), pC(10), pC(11))); }
    void ConfigureOutput10(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 10, 14,
            GPIO_LOC(pC(6), pC(7), pC(8), pC(9), pC(10), pC(11))); }
    void ConfigureOutput11(GPIOPin pin, GPIOPin::Mode mode = GPIOPin::PushPull) { pin.ConfigureAlternate(mode, ROUTEPEN, 11, 15,
            GPIO_LOC(pC(7), pC(8), pC(9), pC(10), pC(11), pC(6))); }
#endif
#endif

    //! Configures a 1 MHz DMA trigger on the specified channel (useful for things such as DMA-based PWM)
    void Configure1MHzDMA(unsigned channel);

    //! Enables the clock to PRS
    void EnableClock() { CMU->EnablePRS(); }

    //! Gets a reference representing the specified channel
    PRSChannel& Channel(unsigned n) const { return *(PRSChannel*)&CH[n]; }
    //! Gets the channel with the specified flags or allocates a new one
    PRSChannelHandle GetChannel(PRSChannel::Flags flags);
    //! Generates a pulse on the specified channel
    void PulseChannel(unsigned n) { SWPULSE = BIT(n); }

private:
    friend class PRSChannel;
};

DEFINE_FLAG_ENUM(PRSChannel::Flags);
