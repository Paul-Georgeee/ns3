/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Vivek Jain <jain.vivek.anand@gmail.com>
 *          Viyom Mittal <viyommittal@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#ifndef TCPBBR_H
#define TCPBBR_H

#include "tcp-congestion-ops.h"
#include "windowed-filter.h"

#include "ns3/data-rate.h"
#include "ns3/random-variable-stream.h"
#include "ns3/traced-value.h"

class TcpBbrCheckGainValuesTest;

namespace ns3
{

using TcpCongState_t = TcpSocketState::TcpCongState_t;
using TcpRateConnection = TcpRateOps::TcpRateConnection;
using TcpRateSample = TcpRateOps::TcpRateSample;

/**
 * \ingroup congestionOps
 *
 * \brief BBR congestion control algorithm
 *
 * This class implement the BBR (Bottleneck Bandwidth and Round-trip propagation time)
 * congestion control type.
 */
class TcpBbr : public TcpCongestionOps
{
  public:
    /**
     * \brief The number of phases in the BBR ProbeBW gain cycle.
     */
    static const uint8_t GAIN_CYCLE_LENGTH = 8;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \brief Constructor
     */
    TcpBbr();

    /**
     * Copy constructor.
     * \param sock The socket to copy from.
     */
    TcpBbr(const TcpBbr& sock) = default;

    /**
     * \brief BBR has the following 4 modes for deciding how fast to send:
     */
    enum BbrMode_t
    {
        BBR_STARTUP,   /**< Ramp up sending rate rapidly to fill pipe */
        BBR_DRAIN,     /**< Drain any queue created during startup */
        BBR_PROBE_BW,  /**< Discover, share bw: pace around estimated bw */
        BBR_PROBE_RTT, /**< Cut inflight to min to probe min_rtt */
    };

    typedef WindowedFilter<DataRate,
                           MaxFilter<DataRate>,
                           uint32_t,
                           uint32_t>
        MaxBandwidthFilter_t; //!< Definition of max bandwidth filter.

    /**
     * Assign a fixed random variable stream number to the random variables
     * used by this model.
     *
     * \param stream first stream index to use
     */
    void SetStream(uint32_t stream);

    std::string GetName() const override;
    bool HasCongControl() const override;
    void CongControl(Ptr<TcpSocketState> tcb,
                     const TcpRateOps::TcpRateConnection& rc,
                     const TcpRateOps::TcpRateSample& rs) override;
    void CongestionStateSet(Ptr<TcpSocketState> tcb,
                            const TcpSocketState::TcpCongState_t newState) override;
    void CwndEvent(Ptr<TcpSocketState> tcb, const TcpSocketState::TcpCAEvent_t event) override;
    uint32_t GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) override;
    Ptr<TcpCongestionOps> Fork() override;

  protected:

    DataRate Bw() const;

    uint32_t Bdp(Ptr<TcpSocketState> tcb, DataRate bw, double gain) const;

    uint32_t GetQuantizationBudget(Ptr<TcpSocketState> tcb, uint32_t cwnd) const;

    void BbrInit(Ptr<TcpSocketState> tcb);

    void UpdateModel(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void UpdateBw(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void UpdateAckAggregation(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void UpdateCyclePhase(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    bool IsNextCyclePhase(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    uint32_t BytesInNetAtEarliestDepartTime(Ptr<TcpSocketState> tcb, uint32_t inflightNow);

    uint32_t Inflight(Ptr<TcpSocketState> tcb, DataRate bw, double gain);

    void AdvanceCyclePhase();

    void CheckFullBwReached(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void CheckDrain(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void ResetProbeBwMode();

    void UpdateMinRtt(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    void SaveCwnd(Ptr<const TcpSocketState> tcb);

    void CheckProbeRttDone(Ptr<TcpSocketState> tcb);

    void ResetMode();

    void UpdateGains();

    void SetPacingRate(Ptr<TcpSocketState> tcb, double gain);

    void InitPacingRateFromRtt(Ptr<TcpSocketState> tcb);

    void SetCwnd(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    bool SetCwndToRecoverOrRestore(Ptr<TcpSocketState> tcb, const TcpRateSample& rs, uint32_t& cwnd);

    uint32_t AckAggregationCwnd(Ptr<TcpSocketState> tcb) const;

    void ResetLtBwSamplingInterval(Ptr<TcpSocketState> tcb);
    void ResetLtBwSampling(Ptr<TcpSocketState> tcb);
    void LtBwIntervalDone(Ptr<TcpSocketState> tcb, DataRate bw);
    void LtBwSampling(Ptr<TcpSocketState> tcb, const TcpRateSample& rs);

    
  private:
    bool m_enableAckAggrModel{false};
    bool m_enableLongTermBwMeasure{false};
    bool m_isInitialized{false};
    Ptr<UniformRandomVariable> m_uv{nullptr}; //!< Uniform Random Variable
    
    /* Parameters */
    uint32_t m_bwWinLen{10};
    Time m_minRttWinLen{Seconds(10)};
    Time m_probeRttDuration{MilliSeconds(200)};

    /* Variables */
    TracedValue<BbrMode_t> m_mode{BbrMode_t::BBR_STARTUP}; //!< Current bbr mode in state machine
    DataRate m_fullBw{0};      //!< Value of full bandwidth recorded
    uint32_t m_fullBwCnt{0};   //!< Number of rounds without large bw gains
    TracedValue<uint32_t> m_cycleIdx{0};    //!< Current index in pacing_gain cycle array
    TracedValue<double> m_pacingGain{2.88539};    //!< Current gain for setting pacing rate
    double m_cwndGain{2.88539};      //!< Current gain for setting cwnd

    TracedValue<Time> m_minRtt{Time::Max()};               //!< Min RTT in m_minRttWin window
    Time m_minRttTimestamp{Seconds(0)};       //!< Timestamp of m_minRtt
    Time m_probeRttDoneTimestamp{Seconds(0)}; //!< End time for BBR_PROBE_RTT mode
    MaxBandwidthFilter_t m_bwFilter;          //!< Max recent delivery rate
    uint32_t m_rttCnt{0};                     //!< Count of packet-timed rounds elapsed
    uint64_t m_nextRttDelivered{0};           //!< rc.delivered at end of round 
    Time m_cycleTimestamp{Seconds(0)};        //!< Time of this cycle phase start

    bool m_hasSeenRtt{false};          //!< Have we seen an RTT sample yet?
    bool m_isFullBwReached{false};     //!< Reached full bw in Startup?
    bool m_isRoundStart{false};        //!< start of packet-timed tx->ack round?
    bool m_isIdleRestart{false};       //!< restarting after idle?
    bool m_isProbeRttRoundDone{false}; //!< True when it is time to exit BBR_PROBE_RTT

    bool m_packetConservation{false};  //!< Enable/Disable packet conservation
    TcpCongState_t m_prevCaState{TcpCongState_t::CA_OPEN}; //!< CA state on previous ACK
    uint32_t m_priorCwnd{0};           //!< The last-known good congestion window
    
    /* For tracking ACK aggregation: */
    Time m_ackEpochTimestamp{Seconds(0)}; //!< Start of ACK sampling epoch
    uint32_t m_extraAcked[2] = {0, 0};    //!< Max excess data ACKed in epoch
    uint32_t m_ackEpochAcked{0};          //!< Bytes (S)ACKed in sampling epoch
    uint32_t m_extraAckedWinRtts{0};      //!< Age of m_extraAcked, in round trips
    uint32_t m_extraAckedWinIdx{0};       //!< Current index in m_extraAcked array

    /* Long term BW measurement.
     * 
     * This seems to be a feature only implemented in Linux.
     * Disabled by default (m_enableLongTermBwMeasure is set to false in TcpBbr::GetTypeId).
     */

    bool m_ltIsSampling{false};
    bool m_ltUseBw{false};
    uint32_t m_ltRttCnt{0};
    DataRate m_ltBw{0};
    uint64_t m_ltLastDelivered{0};
    Time m_ltLastTimestamp{0};
    uint64_t m_ltLastLost{0};

  //oBBR
  private:
    bool m_oBBR{false};                               // Use oBBR or not
    double m_u{0.5};                                  // The param u in the original oBBR paper
    Time m_lastLossTime{0};                           // Time of last loss event
    Time m_lossTimeWinSize{MilliSeconds(3000)};       // When current time - m_lastLossTime > m_lossTimeWinSize, we recover the cwnd gain in Probe BW. Not mentioned in the original oBBR paper.

    uint32_t m_upRttCnt{0};                           // Count of abnormal RTT samples
    uint32_t m_downBwCnt{0};                          // Count of abnormal BW samples

    bool m_changeSC{false};                           // A flag used in compute Score in Section 4.2. Refer to the orignal implementation of oBBR in github;
    int32_t m_cc;                                     // Refer to the orignal implementation of oBBR in github;
    Time m_scoreTime{Time::Max()};                    // The time when starting to score.
    DataRate m_recentBw[100];                         // Recent BW samples
    int32_t m_recentBwIndex{0};                       // Index of the recent BW samples array
    int32_t m_kSamples{30};                           // Use the last m_kSamples samples to compute the new bw

    uint64_t m_firstSent{0};                          // Used for computing the score. 
    uint64_t m_firstDelivered{0};                     // Used for computing the score.
    Time m_scoreInterval{MilliSeconds(200)};          // The interval for computing the score.

    uint64_t m_score1, m_score2, m_score3, m_score4;  // Used for compare the score between before changing the bw and after changing the bw.
    DataRate m_backupBw;                              // Bandwidth backup
    Time m_reTimer{Seconds(0)};     

    double m_maxCwndGain{2.0};
                  


    void oBBRUpdate(Ptr<TcpSocketState> tcb, const TcpRateConnection& rc, const TcpRateSample& rs);
    void oBBRCwndAdjust(Ptr<TcpSocketState> tcb, const TcpRateConnection& rc, const TcpRateSample& rs);
};

namespace TracedValueCallback{
  typedef void (*BbrMode) (const TcpBbr::BbrMode_t oldValue,
                             const TcpBbr::BbrMode_t newValue); 

  typedef void (*PacingGain) (const double oldValue, const double newValue);

  typedef void (*CycleIdx) (const uint32_t oldValue, const uint32_t newValue);

  typedef void (*MinRtt) (const Time oldValue, const Time newValue);
}

} // namespace ns3
#endif // TCPBBR_H
