/* Copyright (c) 2015, Celerway, Kristian Evensen <kristrev@celerway.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <netinet/in.h>

#include "metadata_exporter.h"

#define MD_ZMQ_BIND_INTVL   1000
#define MD_ZMQ_DATA_VERSION 3

enum md_zmq_topics {
    MD_ZMQ_TOPIC_SYSEVENT,
    MD_ZMQ_TOPIC_SENSOR,
    MD_ZMQ_TOPIC_MODEM,
    MD_ZMQ_TOPIC_CONNECTIVITY,
    MD_ZMQ_TOPIC_GPS,
    MD_ZMQ_TOPIC_MODEM_STATE,
    MD_ZMQ_TOPIC_MODEM_MODE,
    MD_ZMQ_TOPIC_MODEM_SIGNAL,
    MD_ZMQ_TOPIC_MODEM_LTE_BAND,
    MD_ZMQ_TOPIC_MODEM_ISP_NAME,
    MD_ZMQ_TOPIC_MODEM_UPDATE,
    MD_ZMQ_TOPIC_MODEM_IP_ADDR,
    MD_ZMQ_TOPIC_MODEM_LOC_CHANGE,
    MD_ZMQ_TOPIC_MODEM_NW_MCCMNC_CHANGE,
    MD_ZMQ_TOPIC_RADIO_CELL_LOCATION_GERAN,
    MD_ZMQ_TOPIC_RADIO_GSM_RR_CELL_SEL_RESEL_PARAM,
    MD_ZMQ_TOPIC_RADIO_GRR_CELL_RESEL,
    MD_ZMQ_TOPIC_RADIO_GSM_RR_CIPHER_MODE,
    MD_ZMQ_TOPIC_RADIO_GSM_RR_CHANNEL_CONF,
    MD_ZMQ_TOPIC_RADIO_WCDMA_RRC_STATE,
    MD_ZMQ_TOPIC_RADIO_WCDMA_CELL_ID,
    __MD_ZMQ_TOPICS_MAX
};
#define MD_ZMQ_TOPICS_MAX (__MD_ZMQ_TOPICS_MAX - 1)

enum md_zmq_keys {
    MD_ZMQ_KEY_SEQ,
    MD_ZMQ_KEY_TSTAMP,
    MD_ZMQ_KEY_DATAID,
    MD_ZMQ_KEY_DATAVERSION,
    MD_ZMQ_KEY_CID,
    MD_ZMQ_KEY_DEVICE_MODE,
    MD_ZMQ_KEY_DEVICE_SUBMODE,
    MD_ZMQ_KEY_DEVICE_STATE,
    MD_ZMQ_KEY_ECIO,
    MD_ZMQ_KEY_ENODEB_ID,
    MD_ZMQ_KEY_ICCID,
    MD_ZMQ_KEY_IF_NAME,
    MD_ZMQ_KEY_IMSI,
    MD_ZMQ_KEY_IMSI_MCCMNC,
    MD_ZMQ_KEY_IMEI,
    MD_ZMQ_KEY_IP_ADDR,
    MD_ZMQ_KEY_INTERNAL_IP_ADDR,
    MD_ZMQ_KEY_ISP_NAME,
    MD_ZMQ_KEY_LAC,
    MD_ZMQ_KEY_LTE_RSRP,
    MD_ZMQ_KEY_LTE_FREQ,
    MD_ZMQ_KEY_LTE_RSSI,
    MD_ZMQ_KEY_LTE_RSRQ,
    MD_ZMQ_KEY_LTE_BAND,
    MD_ZMQ_KEY_LTE_PCI,
    MD_ZMQ_KEY_MODE,
    MD_ZMQ_KEY_NW_MCCMNC,
    MD_ZMQ_KEY_RSCP,
    MD_ZMQ_KEY_RSSI,
    MD_ZMQ_KEY_SIGNAL,
    MD_ZMQ_KEY_INTERFACEID,
    MD_ZMQ_KEY_INTERFACENAME,
    MD_ZMQ_KEY_OPERATOR,
    MD_ZMQ_KEY_ALTITUDE,
    MD_ZMQ_KEY_LONGITUDE,
    MD_ZMQ_KEY_LATITUDE,
    MD_ZMQ_KEY_NUMSAT,
    MD_ZMQ_KEY_NMEA,
    MD_ZMQ_KEY_SPEED,

    MD_ZMQ_KEY_RADIO_CELL_ID,
    MD_ZMQ_KEY_RADIO_PLMN,
    MD_ZMQ_KEY_RADIO_LAC,
    MD_ZMQ_KEY_RADIO_ARFCN,
    MD_ZMQ_KEY_RADIO_BSIC,
    MD_ZMQ_KEY_RADIO_TIMING_ADVANCE,
    MD_ZMQ_KEY_RADIO_RX_LEV,
    MD_ZMQ_KEY_RADIO_CELL_GERAN_INFO_NMR,
    
    MD_ZMQ_KEY_RADIO_SERVING_BCCH_ARFCN,
    MD_ZMQ_KEY_RADIO_SERVING_PBCCH_ARFCN,
    MD_ZMQ_KEY_RADIO_SERVING_PRIORITY_CLASS,
    MD_ZMQ_KEY_RADIO_SERVING_RXLEV_AVG,
    MD_ZMQ_KEY_RADIO_SERVING_C1,
    MD_ZMQ_KEY_RADIO_SERVING_C2,
    MD_ZMQ_KEY_RADIO_SERVING_C31,
    MD_ZMQ_KEY_RADIO_SERVING_C32,
    MD_ZMQ_KEY_RADIO_SERVING_FIVE_SECOND_TIMER,
    MD_ZMQ_KEY_RADIO_CELL_RESELECT_STATUS ,
    MD_ZMQ_KEY_RADIO_RECENT_CELL_SELECTION,
    MD_ZMQ_KEY_RADIO_GRR_CELL_NEIGHBORS,
    
    MD_ZMQ_KEY_RADIO_CIPHERING_STATE,
    MD_ZMQ_KEY_RADIO_CIPHERING_ALGORITHM,
    
    MD_ZMQ_KEY_RADIO_CELL_RESELECT_HYSTERESIS,
    MD_ZMQ_KEY_RADIO_MS_TXPWR_MAX_CCH,
    MD_ZMQ_KEY_RADIO_RXLEV_ACCESS_MIN,
    MD_ZMQ_KEY_RADIO_POWER_OFFSET_VALID,
    MD_ZMQ_KEY_RADIO_POWER_OFFSET,
    MD_ZMQ_KEY_RADIO_NECI,
    MD_ZMQ_KEY_RADIO_ACS,
    MD_ZMQ_KEY_RADIO_OPT_RESELECT_PARAM_IND,
    MD_ZMQ_KEY_RADIO_CELL_BAR_QUALIFY,
    MD_ZMQ_KEY_RADIO_CELL_RESELECT_OFFSET,
    MD_ZMQ_KEY_RADIO_TEMPORARY_OFFSET,
    MD_ZMQ_KEY_RADIO_PENALTY_TIME,
    
    MD_ZMQ_KEY_RADIO_NUM_DED_CHANS,
    MD_ZMQ_KEY_RADIO_DTX_INDICATOR,
    MD_ZMQ_KEY_RADIO_POWER_LEVEL,
    MD_ZMQ_KEY_RADIO_STARTING_TIME_VALID,
    MD_ZMQ_KEY_RADIO_STARTING_TIME,
    MD_ZMQ_KEY_RADIO_CIPHER_FLAG,
    MD_ZMQ_KEY_RADIO_CIPHER_ALGORITHM,
    MD_ZMQ_KEY_RADIO_AFTER_CHANNEL_CONFIG,
    MD_ZMQ_KEY_RADIO_BEFORE_CHANNEL_CONFIG,
    MD_ZMQ_KEY_RADIO_CHANNEL_MODE_1,
    MD_ZMQ_KEY_RADIO_CHANNEL_MODE_2,

    MD_ZMQ_KEY_MONROE_IIF_NAME,
    __MD_ZMQ_KEYS_MAX
};
#define MD_ZMQ_KEYS_MAX (__MD_ZMQ_KEYS_MAX - 1)

enum md_project_ids {
    MD_PROJECT_NNE = 0,
    MD_PROJECT_MNR
};

extern const char *monroe_topics[MD_ZMQ_TOPICS_MAX + 1];
extern const char *monroe_keys[MD_ZMQ_KEYS_MAX + 1];
extern const char *nne_topics[MD_ZMQ_TOPICS_MAX + 1];
extern const char *nne_keys[MD_ZMQ_KEYS_MAX + 1];

struct backend_timeout_handle;

struct md_writer_zeromq {
    MD_WRITER;

    void *zmq_context;
    void *zmq_publisher;
    struct backend_timeout_handle *bind_timeout_handle;

    const char **topics;
    const char **keys;
    uint8_t topics_limit;
    uint8_t keys_limit;
    uint8_t metadata_project;
    uint8_t socket_bound;

    //INET6_ADDRSTRLEN is 46 (max length of ipv6 + trailing 0), 5 is port, 6 is
    //protocol (we right now only support TCP)
    //todo: carrying this around is not so elegant
    char zmq_addr[INET6_ADDRSTRLEN + 5 + 6];
};

void md_zeromq_writer_setup(struct md_exporter *mde, struct md_writer_zeromq* mwz);
void md_zeromq_writer_usage();
