/*
 * Copyright (c) 2019 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * efm32-series1/hw/USART.cpp
 */

#include <hw/USART.h>

#if defined(_SILICON_LABS_32B_SERIES_1) && defined(_EFM32_GIANT_FAMILY)

template<> const GPIOLocations_t _USART<0>::locRx = GPIO_LOC(pE(11), pE(6), pC(10), pE(12), pB(8), pC(1), pG(13));
template<> const GPIOLocations_t _USART<0>::locTx = GPIO_LOC(pE(10), pE(7), pC(11), pE(13), pB(7), pC(0), pG(12));
template<> const GPIOLocations_t _USART<0>::locCs = GPIO_LOC(pE(13), pE(4), pC(8), pC(14), pB(14), pA(13), pG(15));
template<> const GPIOLocations_t _USART<0>::locClk = GPIO_LOC(pE(12), pE(5), pC(9), pC(15), pB(13), pA(12), pG(14));
template<> const GPIOLocations_t _USART<0>::locCts = GPIO_LOC(pE(14), pE(3), pC(7), pC(13), pB(6), pB(11), pH(0));
template<> const GPIOLocations_t _USART<0>::locRts = GPIO_LOC(pE(15), pE(2), pC(6), pC(12), pB(5), pD(6), pH(1));

template<> const GPIOLocations_t _USART<1>::locRx = GPIO_LOC(pC(1), pD(1), pD(6), pF(7), pC(2), pA(0), pA(2));
template<> const GPIOLocations_t _USART<1>::locTx = GPIO_LOC(pC(0), pD(0), pD(7), pF(6), pC(1), pF(2), pA(14));
template<> const GPIOLocations_t _USART<1>::locCs = GPIO_LOC(pB(8), pD(3), pF(1), pC(14), pC(0), pE(4), pB(2));
template<> const GPIOLocations_t _USART<1>::locClk = GPIO_LOC(pB(7), pD(2), pF(0), pC(15), pC(3), pB(11), pE(5));
template<> const GPIOLocations_t _USART<1>::locCts = GPIO_LOC(pB(9), pD(4), pF(3), pC(6), pC(12), pB(13), pH(2));
template<> const GPIOLocations_t _USART<1>::locRts = GPIO_LOC(pB(10), pD(5), pF(4), pC(7), pC(13), pB(14), pH(3));

template<> const GPIOLocations_t _USART<2>::locRx = GPIO_LOC(pC(3), pB(4), pA(8), pA(14), pF(7), pF(1));
template<> const GPIOLocations_t _USART<2>::locTx = GPIO_LOC(pC(2), pB(3), pA(7), pA(13), pF(6), pF(0));
template<> const GPIOLocations_t _USART<2>::locCs = GPIO_LOC(pC(5), pB(6), pA(10), pB(11), pF(9), pF(5));
template<> const GPIOLocations_t _USART<2>::locClk = GPIO_LOC(pC(4), pB(5), pA(9), pA(15), pF(8), pF(2));
template<> const GPIOLocations_t _USART<2>::locCts = GPIO_LOC(pC(1), pB(12), pA(11), pB(10), pC(12), pD(6));
template<> const GPIOLocations_t _USART<2>::locRts = GPIO_LOC(pC(0), pB(15), pA(12), pC(14), pC(13), pD(8));

template<> const GPIOLocations_t _USART<3>::locRx = GPIO_LOC(pA(1), pE(7), pB(7), pG(7), pG(1), pI(13));
template<> const GPIOLocations_t _USART<3>::locTx = GPIO_LOC(pA(0), pE(6), pB(3), pG(6), pG(0), pI(12));
template<> const GPIOLocations_t _USART<3>::locCs = GPIO_LOC(pA(3), pE(4), pC(14), pC(0), pG(3), pI(15));
template<> const GPIOLocations_t _USART<3>::locClk = GPIO_LOC(pA(2), pD(7), pD(4), pG(8), pG(2), pI(14));
template<> const GPIOLocations_t _USART<3>::locCts = GPIO_LOC(pA(4), pE(5), pD(6), pG(10), pG(4), pG(9));
template<> const GPIOLocations_t _USART<3>::locRts = GPIO_LOC(pA(5), pC(1), pA(14), pC(15), pG(5), pG(11));

template<> const GPIOLocations_t _USART<4>::locRx = GPIO_LOC(pB(8), pD(10), pI(1), pI(7), pH(5));
template<> const GPIOLocations_t _USART<4>::locTx = GPIO_LOC(pB(7), pD(9), pI(0), pI(6), pH(4));
template<> const GPIOLocations_t _USART<4>::locCs = GPIO_LOC(pC(5), pD(12), pI(3), pI(9), pH(7));
template<> const GPIOLocations_t _USART<4>::locClk = GPIO_LOC(pC(4), pD(11), pI(2), pI(8), pH(6));
template<> const GPIOLocations_t _USART<4>::locCts = GPIO_LOC(pA(7), pD(13), pI(4), pI(10), pH(8));
template<> const GPIOLocations_t _USART<4>::locRts = GPIO_LOC(pA(8), pD(14), pI(5), pI(11), pH(9));

template<> const GPIOLocations_t _USART<5>::locRx = GPIO_LOC(pE(9), pA(7), pB(1), pH(11));
template<> const GPIOLocations_t _USART<5>::locTx = GPIO_LOC(pE(8), pA(6), pF(15), pH(10));
template<> const GPIOLocations_t _USART<5>::locCs = GPIO_LOC(pB(13), pD(14), pF(12), pH(13));
template<> const GPIOLocations_t _USART<5>::locClk = GPIO_LOC(pB(11), pD(13), pF(13), pH(12));
template<> const GPIOLocations_t _USART<5>::locCts = GPIO_LOC(pB(14), pD(15), pF(11), pH(14));
template<> const GPIOLocations_t _USART<5>::locRts = GPIO_LOC(pB(12), pB(15), pF(10), pH(15));

#define ALL_LOCS(type) const GPIOLocations_t USART::locs ## type[] = { _USART<0>::loc ## type, _USART<1>::loc ## type, _USART<2>::loc ## type, _USART<3>::loc ## type, _USART<4>::loc ## type, _USART<5>::loc ## type }

ALL_LOCS(Rx);
ALL_LOCS(Tx);
ALL_LOCS(Cs);
ALL_LOCS(Clk);
ALL_LOCS(Cts);
ALL_LOCS(Rts);

#endif

const uint8_t USART::SyncTransferDescriptor::s_zero = 0;
uint8_t USART::SyncTransferDescriptor::s_discard;

bool USART::BindCs(unsigned loc)
{
    if (ROUTEPEN & USART_ROUTEPEN_CSPEN)
    {
        return false;
    }

    MODMASK(ROUTELOC0, _USART_ROUTELOC0_CSLOC_MASK, loc << _USART_ROUTELOC0_CSLOC_SHIFT);
    ROUTEPEN |= USART_ROUTEPEN_CSPEN;
    return true;
}

async(USART::BindCs, unsigned loc, mono_t timeout)
async_def()
{
    if (!await_mask_ticks(ROUTEPEN, USART_ROUTEPEN_CSPEN, 0, timeout))
    {
        async_return(false);
    }

    MODMASK(ROUTELOC0, _USART_ROUTELOC0_CSLOC_MASK, loc << _USART_ROUTELOC0_CSLOC_SHIFT);
    ROUTEPEN |= USART_ROUTEPEN_CSPEN;
    async_return(true);
}
async_end

LDMAChannelHandle USART::BeginSyncBidirectionalTransfer(Buffer buffer)
{
    // we can use a temporary variable, becuase the descriptors will be
    // copied over to the LDMA primary descriptor before BeginSyncTransfer returns
    SyncTransferDescriptor trx;
    trx.Bidirectional(buffer);
    return BeginSyncTransfer(&trx, 1);
}

res_pair_t USART::BeginSyncTransferImpl(SyncTransferDescriptor* descriptors, size_t count)
{
    ASSERT(RxEmpty());
    ASSERT(count > 0);

    size_t i;
    for (i = 0; i < count - 1; i++)
    {
        descriptors[i].tx.Destination(&TXDATA);
        descriptors[i].tx.Link(&descriptors[i + 1].tx);
        descriptors[i].rx.Source(&RXDATA);
        descriptors[i].rx.Link(&descriptors[i + 1].rx);
    }

    descriptors[i].tx.Destination(&TXDATA);
    descriptors[i].tx.Link(LDMALink::None);
    descriptors[i].tx.CTRL |= LDMA_CH_CTRL_DONEIFSEN;
    descriptors[i].rx.Source(&RXDATA);
    descriptors[i].rx.Link(LDMALink::None);
    descriptors[i].rx.CTRL |= LDMA_CH_CTRL_DONEIFSEN;

    auto dmaRx = LDMA->GetUSARTChannel(Index(), LDMAChannel::USARTSignal::RxDataValid);
    auto dmaTx = LDMA->GetUSARTChannel(Index(), LDMAChannel::USARTSignal::TxFree);

    dmaRx.LinkLoad(descriptors[0].rx);
    dmaTx.LinkLoad(descriptors[0].tx);

    return RES_PAIR(dmaRx, dmaTx);
}

async(USART::SyncBidirectionalTransfer, Buffer buffer)
async_def(uint32_t dmaWaitMask)
{
    SyncTransferDescriptor trx;
    trx.Bidirectional(buffer);
    auto pair = BeginSyncTransferImpl(&trx, 1);
    f.dmaWaitMask = BIT(RES_PAIR_FIRST(pair)) | BIT(RES_PAIR_SECOND(pair));
    await(LDMA->WaitForDoneMask, f.dmaWaitMask);
}
async_end

async(USART::SyncTransfer, SyncTransferDescriptor* descriptors, size_t count)
async_def(uint32_t dmaWaitMask)
{
    auto pair = BeginSyncTransferImpl(descriptors, count);
    f.dmaWaitMask = BIT(RES_PAIR_FIRST(pair)) | BIT(RES_PAIR_SECOND(pair));
    await(LDMA->WaitForDoneMask, f.dmaWaitMask);
}
async_end

async(USART::SyncTransferSingle, uint32_t data)
async_def()
{
    ASSERT(RxEmpty());

    Transmit(data);
    await_mask(STATUS, USART_STATUS_RXDATAV, USART_STATUS_RXDATAV);
    uint32_t res = Receive();

    ASSERT(RxEmpty());

    async_return(res);
}
async_end
