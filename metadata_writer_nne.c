/* Copyright (c) 2015, Celerway, Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>
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
#include <sys/time.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>

#include "metadata_writer_nne.h"
#include "backend_event_loop.h"
#include "metadata_exporter_log.h"

static struct nne_value md_iface_parse_mode(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_submode(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_rssi(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_rscp(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_ecio(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_rsrp(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_rsrq(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_lac(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_cid(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_oper(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_ipaddr(struct nne_modem *modem, struct md_iface_event *mie);
static struct nne_value md_iface_parse_dev_state(struct nne_modem *modem, struct md_iface_event *mie);

static struct nne_metadata_descr NNE_METADATA_DESCR[] = {
    { NNE_IDX_MODE,      "mode",         0, NNE_TYPE_UINT8,  IFACE_EVENT_MODE_CHANGE, md_iface_parse_mode },
    { NNE_IDX_SUBMODE,   "submode",      0, NNE_TYPE_UINT8,  IFACE_EVENT_MODE_CHANGE, md_iface_parse_submode },
    { NNE_IDX_RSSI,      "rssi",         1, NNE_TYPE_INT8,   IFACE_EVENT_SIGNAL_CHANGE, md_iface_parse_rssi },
    { NNE_IDX_RSCP,      "rscp",         1, NNE_TYPE_INT16,  IFACE_EVENT_SIGNAL_CHANGE, md_iface_parse_rscp },
    { NNE_IDX_ECIO,      "ecio",         1, NNE_TYPE_INT8,   IFACE_EVENT_SIGNAL_CHANGE, md_iface_parse_ecio },
    { NNE_IDX_RSRP,      "rsrp",         1, NNE_TYPE_INT16,  IFACE_EVENT_SIGNAL_CHANGE, md_iface_parse_rsrp },
    { NNE_IDX_RSRQ,      "rsrq",         1, NNE_TYPE_INT8,   IFACE_EVENT_SIGNAL_CHANGE, md_iface_parse_rsrq },
    { NNE_IDX_LAC,       "lac",          0, NNE_TYPE_STRING, IFACE_EVENT_LOC_CHANGE, md_iface_parse_lac },
    { NNE_IDX_CID,       "cid",          0, NNE_TYPE_STRING, IFACE_EVENT_LOC_CHANGE, md_iface_parse_cid },
    { NNE_IDX_OPER,      "oper",         0, NNE_TYPE_UINT32, IFACE_EVENT_NW_MCCMNC_CHANGE, md_iface_parse_oper },
    { NNE_IDX_IPADDR,    "ipaddr",       0, NNE_TYPE_STRING, IFACE_EVENT_IP_ADDR_CHANGE, md_iface_parse_ipaddr },
    { NNE_IDX_DEV_STATE, "device_state", 0, NNE_TYPE_UINT8,  IFACE_EVENT_DEV_STATE, md_iface_parse_dev_state }
};

#define NNE_METADATA_DESCR_LEN (sizeof(NNE_METADATA_DESCR) / sizeof(struct nne_metadata_descr))


static struct nne_value md_iface_parse_mode(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_UINT8;
    switch (mie->device_mode) {
        case 0: // UNKNOWN
        case 1: // DISCONNECTED
            value.type = NNE_TYPE_NULL;
            break;
        case 2:
            value.u.v_int8 = NNE_MODE_NOSERVICE;
            break;
        case 3:
            value.u.v_int8 = NNE_MODE_GSM;
            break;
        case 4:
            value.u.v_int8 = NNE_MODE_WCDMA;
            break;
        case 5:
            value.u.v_int8 = NNE_MODE_LTE;
            break;
        default:
            value.type = NNE_TYPE_NULL;
    }
    return value;
}

static struct nne_value md_iface_parse_submode(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_NULL;
    if (modem->metadata[NNE_IDX_MODE].value.u.v_uint8 == NNE_MODE_WCDMA) {
        value.type = NNE_TYPE_UINT8;
        switch(mie->device_submode) {
            case 0: // UNKNOWN
                value.u.v_int8 = NNE_SUBMODE_UNKWONW;
                break;
            // case 1: // MODE_UMTS:
            case 2: // MODE_WCDMA:
                value.u.v_int8 = NNE_SUBMODE_WCDMA;
                break;
            //case 3: // MODE_EVDO,
            case 4: // MODE_HSPA
                value.u.v_int8 = NNE_SUBMODE_HSPA;
                break;
            case 5: // MODE_HSPA_PLUS
                value.u.v_int8 = NNE_SUBMODE_HSPA_PLUS;
                break;
            //case 6: // MODE_DC_HSPA
            case 7: // MODE_DC_HSPA_PLUS
                value.u.v_int8 = NNE_SUBMODE_DC_HSPA_PLUS;
                break;
            case 8: // MODE_HSDPA
                value.u.v_int8 = NNE_SUBMODE_HSDPA;
                break;
            case 9: // MODE_HSUPA
                value.u.v_int8 = NNE_SUBMODE_HSUPA;
                break;
            case 10: // MODE_HSDPA_HSUPA
                value.u.v_int8 = NNE_SUBMODE_HSPA;
                break;
            //case 11: // MODE_HSDPA_PLUS
            case 12: // MODE_HSDPA_PLUS_HSUPA
                value.u.v_int8 = NNE_SUBMODE_HSPA_PLUS;
                break;
            //case 13: // MODE_DC_HSDPA_PLUS
            case 14: // MODE_DC_HSDPA_PLUS_HSUPA
                value.u.v_int8 = NNE_SUBMODE_DC_HSPA_PLUS;
                break;
            default:
                value.type = NNE_TYPE_NULL;
        }
    }
    return value;
}

static struct nne_value md_iface_parse_rssi(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_INT8;
    if (modem->metadata[NNE_IDX_MODE].value.u.v_uint8 == NNE_MODE_LTE)
        value.u.v_int8 = mie->lte_rssi;
    else
        value.u.v_int8 = mie->rssi;
    return value;
}

static struct nne_value md_iface_parse_rscp(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    uint8_t mode = modem->metadata[NNE_IDX_MODE].value.u.v_uint8;
    if (mode == NNE_MODE_GSM || mode == NNE_MODE_WCDMA) {
        value.type = NNE_TYPE_INT16;
        value.u.v_int16 = mie->rscp;
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_ecio(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    uint8_t mode = modem->metadata[NNE_IDX_MODE].value.u.v_uint8;
    if (mode == NNE_MODE_GSM || mode == NNE_MODE_WCDMA) {
        value.type = NNE_TYPE_INT8;
        value.u.v_int8 = mie->ecio;
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_rsrp(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    uint8_t mode = modem->metadata[NNE_IDX_MODE].value.u.v_uint8;
    if (mode == NNE_MODE_LTE) {
        value.type = NNE_TYPE_INT16;
        value.u.v_int16 = mie->lte_rsrp;
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_rsrq(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    uint8_t mode = modem->metadata[NNE_IDX_MODE].value.u.v_uint8;
    if (mode == NNE_MODE_LTE) {
        value.type = NNE_TYPE_INT8;
        value.u.v_int16 = mie->lte_rsrq;
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_lac(struct nne_modem *modem, struct md_iface_event *mie)
{
    int len = 16;
    size_t retval;
    struct nne_value value;
    value.type = NNE_TYPE_STRING;
    value.u.v_str = malloc(len);
    if (value.u.v_str != NULL) {
        retval = snprintf(value.u.v_str, len, "%X", mie->lac);
        if (retval >= len) {
            value.type = NNE_TYPE_NULL;
            free(value.u.v_str);
            value.u.v_str = NULL;
        }
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_cid(struct nne_modem *modem, struct md_iface_event *mie)
{
    int len = 16;
    size_t retval;
    struct nne_value value;
    value.type = NNE_TYPE_STRING;
    value.u.v_str = malloc(len);
    if (value.u.v_str != NULL) {
        retval = snprintf(value.u.v_str, len, "%X", mie->cid);
        if (retval >= len) {
            value.type = NNE_TYPE_NULL;
            free(value.u.v_str);
            value.u.v_str = NULL;
        }
    }
    else
        value.type = NNE_TYPE_NULL;
    return value;
}

static struct nne_value md_iface_parse_oper(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_UINT32;
    value.u.v_uint32 = mie->nw_mccmnc;
    return value;
}

static struct nne_value md_iface_parse_ipaddr(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_STRING;
    if (mie->ip_addr != NULL)
        value.u.v_str = strdup("UP");
    else
        value.u.v_str = strdup("DOWN");
    return value;
}

static struct nne_value md_iface_parse_dev_state(struct nne_modem *modem, struct md_iface_event *mie)
{
    struct nne_value value;
    value.type = NNE_TYPE_UINT8;
    value.u.v_int8 = mie->device_state;
    return value;
}

static uint8_t md_nne_handle_gps_event(struct md_writer_nne *mwn,
                                       struct md_gps_event *mge)
{
    char name_buf[256];
    char tm_buf[128];
    char xml_buf[128];
    struct tm *gtm;
    size_t retval;

    if (mge->minmea_id == MINMEA_SENTENCE_RMC)
        return RETVAL_IGNORE;

    if (mwn->gps_file == NULL) {
        if ((mwn->gps_file = fopen(mwn->gps_fname, "a")) == NULL) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to open file %s\n",
                    mwn->gps_fname);
            return RETVAL_FAILURE;
        }
    }

    if ((gtm = gmtime((time_t *)&mge->tstamp_tv.tv_sec)) == NULL) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Invalid GPS timestamp\n");
        return RETVAL_FAILURE;
    }

    if (!strftime(tm_buf, sizeof(tm_buf), "%Y-%m-%d %H:%M:%S", gtm)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format timestamp\n");
        return RETVAL_FAILURE;
    }

    retval = snprintf(xml_buf, sizeof(xml_buf),
            "<d e=\"0\"><lat>%f</lat><lon>%f</lon><speed>%f</speed></d>",
             mge->latitude, mge->longitude, mge->speed);
    if (retval >= sizeof(xml_buf)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format XML data\n");
        return RETVAL_FAILURE;
    }

    mwn->gps_sequence += 1;

    if (fprintf(mwn->gps_file, "%s.%06ld\t%i\t%i\t%s\n", tm_buf, mge->tstamp_tv.tv_usec,
            mwn->gps_instance_id, mwn->gps_sequence, xml_buf) < 0) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to write to export file%s\n",
                name_buf);
        return RETVAL_FAILURE;
    }

    return RETVAL_SUCCESS;
}

static struct nne_modem *md_nne_get_modem(struct nne_modem_list *modem_list, uint32_t network_id)
{
    struct nne_modem *e = NULL;
    for (e = modem_list->lh_first; e != NULL; e = e->entries.le_next)
        if (e->network_id == network_id)
            return e;

    return NULL;
}

static struct nne_value nne_value_init_str(char* str)
{
    struct nne_value value;
    value.type = NNE_TYPE_STRING;
    value.u.v_str = str;
    return value;
}

static struct nne_value nne_value_init(enum nne_type type, void *ptr, int offset)
{
    struct nne_value value;
    value.type = type;
    switch(type)
    {
        case NNE_TYPE_NULL:
            break;
        case NNE_TYPE_INT8:
            value.u.v_int8 = *(int8_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_UINT8:
            value.u.v_uint8 = *(uint8_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_INT16:
            value.u.v_int16 = *(int16_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_UINT16:
            value.u.v_uint16 = *(uint16_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_INT32:
            value.u.v_int32 = *(int32_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_UINT32:
            value.u.v_uint32 = *(uint32_t*)((char*)ptr + offset);
            break;
        case NNE_TYPE_STRING:
            value.u.v_str = (char*)ptr + offset;
            break;
    }

    return value;
}

static int nne_value_compare(struct nne_value v1, struct nne_value v2)
{
    if (v1.type != v2.type)
        return -2;

    switch(v1.type)
    {
        case NNE_TYPE_NULL:
            return 0; // NULL equals NULL
        case NNE_TYPE_INT8:
            return (v1.u.v_int8 == v2.u.v_int8) ? 0 : ((v1.u.v_int8 < v2.u.v_int8) ? -1 : 1);
        case NNE_TYPE_UINT8:
            return (v1.u.v_uint8 == v2.u.v_uint8) ? 0 : ((v1.u.v_uint8 < v2.u.v_uint8) ? -1 : 1);
        case NNE_TYPE_INT16:
            return (v1.u.v_int16 == v2.u.v_int16) ? 0 : ((v1.u.v_int16 < v2.u.v_int16) ? -1 : 1);
        case NNE_TYPE_UINT16:
            return (v1.u.v_uint16 == v2.u.v_uint16) ? 0 : ((v1.u.v_uint16 < v2.u.v_uint16) ? -1 : 1);
        case NNE_TYPE_INT32:
            return (v1.u.v_int32 == v2.u.v_int32) ? 0 : ((v1.u.v_int32 < v2.u.v_int32) ? -1 : 1);
        case NNE_TYPE_UINT32:
            return (v1.u.v_uint32 == v2.u.v_uint32) ? 0 : ((v1.u.v_uint32 < v2.u.v_uint32) ? -1 : 1);
        case NNE_TYPE_STRING:
            return strcmp(v1.u.v_str, v2.u.v_str);
    }

    return -2;
}

static int md_nne_add_json_key_value_int(struct json_object *obj,
        const char *key, int64_t value)
{
    struct json_object *value_obj = NULL;

    value_obj = json_object_new_int64(value);
    if (!value_obj)
        return RETVAL_FAILURE;

    json_object_object_add(obj, key, value_obj);
    return RETVAL_SUCCESS;
}

static int md_nne_add_json_key_value_string(struct json_object *obj,
        const char *key, const char *value)
{
    struct json_object *value_obj = NULL;

    value_obj = json_object_new_string(value);
    if (!value_obj)
        return RETVAL_FAILURE;

    json_object_object_add(obj, key, value_obj);
    return RETVAL_SUCCESS;
}

static int md_nne_add_json_key_value_null(struct json_object *obj,
        const char *key)
{
    json_object_object_add(obj, key, NULL);
    return RETVAL_SUCCESS;
}

static int md_nne_add_json_key_value(struct json_object *obj,
        const char* key, struct nne_value value)
{
    switch(value.type)
    {
        case NNE_TYPE_NULL:
            return md_nne_add_json_key_value_null(obj, key);
        case NNE_TYPE_INT8:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_int8);
        case NNE_TYPE_UINT8:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_uint8);
        case NNE_TYPE_INT16:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_int16);
        case NNE_TYPE_UINT16:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_uint16);
        case NNE_TYPE_INT32:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_int32);
        case NNE_TYPE_UINT32:
            return md_nne_add_json_key_value_int(obj, key, value.u.v_uint32);
        case NNE_TYPE_STRING:
            return md_nne_add_json_key_value_string(obj, key, value.u.v_str);
    }
    return RETVAL_FAILURE;
}

static void md_nne_send_message(struct md_writer_nne *mwn,
        struct nne_message* msg)
{
    static const char *NNE_MESSAGE_TYPE_STR[] = {
        "event",
        "bins-1min"
    };

    static const char *NNE_MESSAGE_SOURCE_STR[] = {
        "report",
        "query"
    };

    struct json_object* obj = json_object_new_object();

    md_nne_add_json_key_value_string(obj, "type", NNE_MESSAGE_TYPE_STR[msg->type]);
    md_nne_add_json_key_value_int(obj, "ts", msg->tstamp);
    md_nne_add_json_key_value_string(obj, "node", msg->node);
    md_nne_add_json_key_value_int(obj, "network_id", msg->network_id);

    md_nne_add_json_key_value_string(obj, "key", msg->key);
    md_nne_add_json_key_value(obj, "value", msg->value);

    if (msg->extra == NULL)
        md_nne_add_json_key_value_null(obj, "extra");
    else
        md_nne_add_json_key_value_string(obj, "extra", msg->extra);

    md_nne_add_json_key_value_string(obj, "source", NNE_MESSAGE_SOURCE_STR[msg->source]);

    if (msg->type == NNE_MESSAGE_TYPE_BINS1MIN)
        md_nne_add_json_key_value_int(obj, "delta", msg->delta);

    META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: network_id %u: %s\n", msg->network_id,
                      json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN));

    if (mwn->metadata_cache == NULL)
        mwn->metadata_cache = json_object_new_array();

    json_object_array_add(mwn->metadata_cache, obj);
}

static void md_nne_process_iface_event(struct md_writer_nne *mwn,
                                       struct nne_metadata_descr* descr,
                                       struct nne_modem* modem,
                                       struct md_iface_event *mie,
                                       enum nne_message_source source)
{
    int i;
    struct nne_value value =  descr->parse_cb(modem, mie);

    if (nne_value_compare(modem->metadata[descr->idx].value, value) != 0) {

        if (modem->metadata[descr->idx].value.type == NNE_TYPE_STRING)
            free(modem->metadata[descr->idx].value.u.v_str);
        modem->metadata[descr->idx].value = value;

        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: network_id %u: %d,%s changed %d\n",
                          modem->network_id, descr->idx, descr->key,
                          modem->metadata[descr->idx].value.u.v_uint8);

        // Generate json event
        struct nne_message msg;
        msg.type = NNE_MESSAGE_TYPE_EVENT;
        msg.tstamp = mie->tstamp;
        msg.node = mwn->node_id;
        msg.network_id = modem->network_id;
        msg.key = descr->key;
        msg.value = value;
        msg.extra = NULL;
        msg.source = source;
        msg.delta = 0;

        md_nne_send_message(mwn, &msg);

        // if mode has changed nullify mode dependend metadata
        if (descr->idx == NNE_IDX_MODE) {
            for(i = 0; i <= NNE_IDX_MAX; i++) {
                if (descr->mode_dependent) {
                    if (modem->metadata[i].value.type == NNE_TYPE_STRING) {
                        free(modem->metadata[i].value.u.v_str);
                    }
                    modem->metadata[i].value.type = NNE_TYPE_NULL;

                    META_PRINT_SYSLOG(mwn->parent, LOG_ERR,
                            "NNE writer: network_id %u: %d,%s nullified\n",
                            modem->network_id, descr->idx, descr->key);
                }
            }
        }
    }
    else {
        if (value.type == NNE_TYPE_STRING) {
            free(value.u.v_str);
        }
    }

    modem->metadata[descr->idx].tstamp = mie->tstamp;
}

static void md_nne_generate_bins1min(struct md_writer_nne *mwn,
                                        uint64_t tstamp)
{
    struct nne_modem *modem = NULL;
    struct nne_metadata_descr *descr;
    struct nne_message msg;
    int i;

    LIST_FOREACH(modem, &(mwn->modem_list), entries)
    {
        // On first event after new minute has begun
        // generate bins1min entries for all metadata values
        if ((tstamp / 60) <= (modem->tstamp / 60))
            continue;

        for (i = 0; i < NNE_METADATA_DESCR_LEN; i++)
        {
            descr = &(NNE_METADATA_DESCR[i]);

            msg.type = NNE_MESSAGE_TYPE_BINS1MIN;
            msg.tstamp = (tstamp / 60) * 60;
            msg.node = mwn->node_id;
            msg.network_id = modem->network_id;
            msg.key = descr->key;
            msg.value = modem->metadata[descr->idx].value;
            msg.extra = NULL;
            msg.source = NNE_MESSAGE_SOURCE_QUERY;
            msg.delta = msg.tstamp - modem->metadata[descr->idx].tstamp;

            md_nne_send_message(mwn, &msg);
        }
    }
}

static void md_nne_check_removed_modems(struct md_writer_nne *mwn,
                                        uint64_t tstamp)
{
    struct nne_modem *modem = NULL, *rm_modem = NULL;
    struct nne_message msg;

    modem = mwn->modem_list.lh_first;
    while(modem != NULL)
    {
        // check if modem has been removed
        rm_modem = NULL;
        if (modem->tstamp < tstamp - 40)
            rm_modem = modem;

        modem = LIST_NEXT(modem, entries);

        // if modem has been removed generate
        // usbmodem DOWN event and remove the entry
        if (rm_modem != NULL)
        {
            msg.type = NNE_MESSAGE_TYPE_EVENT;
            msg.tstamp = tstamp;
            msg.node = mwn->node_id;
            msg.network_id = rm_modem->network_id;
            msg.key = "usbmodem";
            msg.value = nne_value_init_str("DOWN");
            msg.extra = NULL;
            msg.source = NNE_MESSAGE_SOURCE_REPORT;
            msg.delta = 0;
            md_nne_send_message(mwn, &msg);

            LIST_REMOVE(rm_modem, entries);

            for(int i = 0; i <= NNE_IDX_MAX; i++)
                if (rm_modem->metadata[i].value.type == NNE_TYPE_STRING)
                    free(rm_modem->metadata[i].value.u.v_str);

            free(rm_modem);

            META_PRINT_SYSLOG(mwn->parent, LOG_ERR,
                    "NNE writer: network_id %u: Removed modem\n", rm_modem->network_id);
        }
    }
}

static uint32_t md_find_network_id(struct md_writer_nne *mwn, const char *imsi)
{
    struct nne_network *network;
    size_t len;;

    if (imsi == NULL)
        return 0;
    len = strlen(imsi);
    LIST_FOREACH(network, &(mwn->network_list), entries)
    {
        if (!strncmp(imsi,network->imsi, len))
            return network->network_id;
    }

    return 0;
}

static void md_nne_handle_iface_event(struct md_writer_nne *mwn,
                                      struct md_iface_event *mie)
{

    const char* const iface_event_type[] = {
        "",
        "IFACE_EVENT_DEV_STATE",
        "IFACE_EVENT_MODE_CHANGE",
        "IFACE_EVENT_SIGNAL_CHANGE",
        "IFACE_EVENT_LTE_BAND_CHANGE",
        "IFACE_EVENT_ISP_NAME_CHANGE",
        "IFACE_EVENT_UPDATE",
        "IFACE_EVENT_IP_ADDR_CHANGE",
        "IFACE_EVENT_LOC_CHANGE",
        "IFACE_EVENT_NW_MCCMNC_CHANGE",
    };

    META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: %s: "
                      "ip_addr=%s, "
                      "ifname=%s, "
                      "imsi=%s, "
                      "imsi_mccmnc=%d, "
                      "nw_mccmnc=%d, "
                      "cid=%d, "
                      "rscp=%d, "
                      "lte_rsrp=%d, "
                      "lac=%d, "
                      "rssi=%d, "
                      "ecio=%d, "
                      "lte_rssi=%d, "
                      "lte_rsrq=%d, "
                      "device_mode=%d, "
                      "device_submode=%d\n",
                      iface_event_type[mie->event_param],
                      mie->ip_addr,
                      mie->ifname,
                      mie->imsi,
                      mie->imsi_mccmnc,
                      mie->nw_mccmnc,
                      mie->cid,
                      mie->rscp,
                      mie->lte_rsrp,
                      mie->lac,
                      mie->rssi,
                      mie->ecio,
                      mie->lte_rssi,
                      mie->lte_rsrq,
                      mie->device_mode,
                      mie->device_submode);


    struct nne_modem *modem = NULL;
    uint32_t network_id;
    int i;


    // Get network_id and check if it is supported
    network_id = md_find_network_id(mwn, mie->imsi);
    META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: "
            "network: imsi %s network_id %u\n",
            mie->imsi, network_id);
    if (network_id == 0) {
        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: "
                "unsupported subcriber: imsi: %s\n", mie->imsi);
        return;
    }

    modem = md_nne_get_modem(&(mwn->modem_list), network_id);
    if (modem == NULL)
    {
        modem = malloc(sizeof(struct nne_modem));
        LIST_INSERT_HEAD(&(mwn->modem_list), modem, entries);
        modem->network_id = network_id;
        modem->tstamp = mie->tstamp;

        for(i = 0; i <= NNE_IDX_MAX; i++)
        {
            modem->metadata[i].tstamp = mie->tstamp;
            modem->metadata[i].value.type = NNE_TYPE_NULL;
        }

        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: network_id %u: Added new  modem\n", network_id);

        // generate usbmodem UP event
        struct nne_message msg;
        msg.type = NNE_MESSAGE_TYPE_EVENT;
        msg.tstamp = mie->tstamp;
        msg.node = mwn->node_id;
        msg.network_id = network_id;
        msg.key = "usbmodem";
        msg.value = nne_value_init_str("UP");
        msg.extra = NULL;
        msg.source = NNE_MESSAGE_SOURCE_REPORT;
        msg.delta = 0;

        md_nne_send_message(mwn, &msg);

        // Initiate cache with current values
        for (i = 0; i < NNE_METADATA_DESCR_LEN; i++) {
            md_nne_process_iface_event(mwn, &(NNE_METADATA_DESCR[i]), modem, mie, NNE_MESSAGE_SOURCE_QUERY);
        }
    }

    md_nne_generate_bins1min(mwn, mie->tstamp);

    modem->tstamp = mie->tstamp;

    // Process metadata; only related to this iface event
    for (i = 0; i < NNE_METADATA_DESCR_LEN; i++) {
        if (mie->event_param == NNE_METADATA_DESCR[i].event) {
            md_nne_process_iface_event(mwn, &(NNE_METADATA_DESCR[i]), modem, mie, NNE_MESSAGE_SOURCE_REPORT);
        }
    }

    md_nne_check_removed_modems(mwn, mie->tstamp);
}

static int md_nne_format_fname_tm(char *s, int max, const char *format)
{
    struct timeval tv;
    struct tm *gtm;
    char tm_buf[128];
    size_t retval;

    if (gettimeofday(&tv, NULL)) {
        return RETVAL_FAILURE;
    }

    if ((gtm = gmtime((time_t *)&tv.tv_sec)) == NULL) {
        return RETVAL_FAILURE;
    }

    if (!strftime(tm_buf, sizeof(tm_buf), "%Y%m%dT%H%M%S", gtm)) {
        return RETVAL_FAILURE;
    }

    retval = snprintf(s, max, format, tm_buf);
    if (retval >= max) {
        return RETVAL_FAILURE;
    }

    return RETVAL_SUCCESS;
}

static void md_nne_handle_gps_timeout(struct md_writer_nne *mwn)
{
    char fname_tm[128];

    if (mwn->gps_file != NULL) {
        fclose(mwn->gps_file);
        mwn->gps_file = NULL;

        if (md_nne_format_fname_tm(fname_tm, sizeof(fname_tm), mwn->gps_fname_tm)) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
            return;
        }
        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: fname_tm: %s\n", fname_tm);

        if (rename(mwn->gps_fname, fname_tm)) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to rename file %s to %s\n",
                              mwn->gps_fname, fname_tm);
            return;
        }

        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: Rotated file %s to %s\n",
                          mwn->gps_fname, fname_tm);
    }
}

static void md_nne_handle_metadata_timeout(struct md_writer_nne *mwn)
{
    FILE *file;
    const char* str;
    char fname_tm[128];
    struct nne_message msg;
    struct timeval tv;

    if (gettimeofday(&tv, NULL))
    {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: gettimeofday failed\n");
    }

    // Generate collector UP bins1min entry
    // every minute (at first invocation of this
    // timeout handler after minute changes)
    if ((tv.tv_sec / 60) > (mwn->timeout_tstamp / 60))
    {
        msg.type = NNE_MESSAGE_TYPE_BINS1MIN;
        msg.tstamp = (tv.tv_sec / 60) * 60;
        msg.node = mwn->node_id;
        msg.network_id = 0;
        msg.key = "collector";
        msg.value = nne_value_init_str("UP");
        msg.extra = NULL;
        msg.source = NNE_MESSAGE_SOURCE_QUERY;
        msg.delta = 0;
        md_nne_send_message(mwn, &msg);
    }

    mwn->timeout_tstamp = tv.tv_sec;

    md_nne_check_removed_modems(mwn, tv.tv_sec);

    if (mwn->metadata_cache != NULL)
    {
        if ((file = fopen(mwn->metadata_fname, "w")) == NULL) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to open file %s\n",
                    mwn->metadata_fname);
            return;
        }

        str = json_object_to_json_string_ext(mwn->metadata_cache, JSON_C_TO_STRING_PLAIN);

        if (fprintf(file, "%s", str) < 0) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to write to file %s\n",
                    mwn->metadata_fname);
            return;
        }

        fclose(file);

        json_object_put(mwn->metadata_cache);
        mwn->metadata_cache = NULL;

        if (md_nne_format_fname_tm(fname_tm, sizeof(fname_tm), mwn->metadata_fname_tm)) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
            return;
        }

        if (rename(mwn->metadata_fname, fname_tm)) {
            META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to rename file %s to %s\n",
                mwn->metadata_fname, fname_tm);
            return;
        }

        META_PRINT_SYSLOG(mwn->parent, LOG_INFO, "NNE writer: Metadata exported to file %s\n",
                fname_tm);
    }
}

static void md_nne_handle_timeout(void *ptr)
{
    struct md_writer_nne *mwn = ptr;
    md_nne_handle_gps_timeout(mwn);
    md_nne_handle_metadata_timeout(mwn);
}

static int32_t md_nne_init(void *ptr, json_object* config)
{
    struct md_writer_nne *mwn = ptr;
    struct json_object *subconfig, *netconfig;
    struct nne_network *network;
    size_t retval;

    strcpy(mwn->directory, NNE_DEFAULT_DIRECTORY);
    mwn->interval = NNE_DEFAULT_INTERVAL_MS;
    strcpy(mwn->node_id, "");
    strcpy(mwn->gps_prefix, NNE_DEFAULT_GPS_PREFIX);
    mwn->gps_instance_id = 0;
    strcpy(mwn->gps_extension, NNE_DEFAULT_GPS_EXTENSION);
    strcpy(mwn->metadata_prefix, NNE_DEFAULT_METADATA_PREFIX);
    strcpy(mwn->metadata_extension, NNE_DEFAULT_METADATA_EXTENSION);

    mwn->gps_file = NULL;
    mwn->gps_sequence = 0;

    mwn->metadata_cache = NULL;
    LIST_INIT(&(mwn->modem_list));
    mwn->timeout_tstamp = 0;

    if (json_object_object_get_ex(config, "nne", &subconfig)) {
        json_object_object_foreach(subconfig, key, val) {
            if (!strcmp(key, "node_id")) {
                const char* node_id = json_object_get_string(val);
                if (strlen(node_id) > sizeof(mwn->node_id) - 1) {
                    META_PRINT_SYSLOG(mwn->parent, LOG_ERR,
                            "NNE writer: node_id too long (>%ld)\n",
                            sizeof(mwn->node_id) - 1);
                    return RETVAL_FAILURE;
                } else
                    strcpy(mwn->node_id, node_id);
            }
            else if (!strcmp(key, "interval"))
                mwn->interval = ((uint32_t) json_object_get_int(val)) * 1000;
            else if (!strcmp(key, "gps_instance"))
                mwn->gps_instance_id = ((uint32_t) json_object_get_int(val)) * 1000;
            else if (!strcmp(key, "gps_prefix")) {
                const char* prefix = json_object_get_string(val);
                if (strlen(prefix) > sizeof(mwn->gps_prefix) - 1) {
                    META_PRINT_SYSLOG(mwn->parent, LOG_ERR,
                            "NNE writer: gps prefix too long (>%ld)\n",
                            sizeof(mwn->gps_prefix) - 1);
                    return RETVAL_FAILURE;
                } else
                    strcpy(mwn->gps_prefix, prefix);
            }
            else if (!strcmp(key, "networks")) {
                json_object_object_get_ex(subconfig, "networks", &netconfig);
                json_object_object_foreach(netconfig, key, val) {
                    network = malloc(sizeof(struct nne_network));
                    LIST_INSERT_HEAD(&(mwn->network_list), network, entries);
                    strncpy(network->imsi, key, sizeof(network->imsi));
                    network->network_id = (uint32_t) json_object_get_int(val);
                }
            }
        }
    }

    if (strlen(mwn->node_id) == 0) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Missing mandatory parameter node_id\n");
        return RETVAL_FAILURE;
    }

    retval = snprintf(mwn->gps_fname, sizeof(mwn->gps_fname), "%s%s%d%s",
                      mwn->directory, mwn->gps_prefix,
                      mwn->gps_instance_id, mwn->gps_extension);
    if (retval >= sizeof(mwn->gps_fname)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
        return RETVAL_FAILURE;
    }

    retval = snprintf(mwn->gps_fname_tm, sizeof(mwn->gps_fname_tm), "%s%s%d%s.%%s",
                      mwn->directory, mwn->gps_prefix,
                      mwn->gps_instance_id, mwn->gps_extension);
    if (retval >= sizeof(mwn->gps_fname_tm)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
        return RETVAL_FAILURE;
    }

    retval = snprintf(mwn->metadata_fname, sizeof(mwn->metadata_fname), "%s.%s%stmp%s",
                      mwn->directory, mwn->node_id,
                      mwn->metadata_prefix, mwn->metadata_extension);
    if (retval >= sizeof(mwn->metadata_fname)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
        return RETVAL_FAILURE;
    }

    retval = snprintf(mwn->metadata_fname_tm, sizeof(mwn->metadata_fname_tm), "%s%s%s%%s%s",
                      mwn->directory, mwn->node_id,
                      mwn->metadata_prefix, mwn->metadata_extension);
    if (retval >= sizeof(mwn->metadata_fname_tm)) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to format file name\n");
        return RETVAL_FAILURE;
    }

    if(!(mwn->timeout_handle = backend_event_loop_create_timeout(0,
             md_nne_handle_timeout, mwn, mwn->interval))) {
        META_PRINT_SYSLOG(mwn->parent, LOG_ERR, "NNE writer: Failed to create file rotation/export timeout\n");
        return RETVAL_FAILURE;
    }

    mde_start_timer(mwn->parent->event_loop, mwn->timeout_handle,
                    mwn->interval);

    return RETVAL_SUCCESS;
}

static void md_nne_handle(struct md_writer *writer, struct md_event *event)
{
    struct md_writer_nne *mwn = (struct md_writer_nne*) writer;

    switch (event->md_type) {
        case META_TYPE_POS:
            md_nne_handle_gps_event(mwn, (struct md_gps_event*) event);
            break;
        case META_TYPE_INTERFACE:
            md_nne_handle_iface_event(mwn, (struct md_iface_event*) event);
            break;
        default:
            return;
        }
}

void md_nne_usage()
{
    fprintf(stderr, "\"nne\": {\t\tNornet Edge writer.\n");
    fprintf(stderr, "  \"node_id\":\t\tNornet node id (e.g. nne601)\n");
    fprintf(stderr, "  \"interval\":\t\tFile rotation/export interval (in seconds)\n");
    fprintf(stderr, "  \"gsp_instance\":\tNNE measurement instance id for gps\n");
    fprintf(stderr, "  \"gps_prefix\":\t\tFile prefix for gps /nne/data/<PREFIX><INSTANCE>.sdat\n");
    fprintf(stderr, "  \"networks\": {\t\tMapping of imsi (or only mccmnc) to network_id\n");
    fprintf(stderr, "    \"<IMSI/MCCMNC>\": <NETWORK-ID>\n");
    fprintf(stderr, "    \"<IMSI/MCCMNC>\": <NETWORK-ID>\n");
    fprintf(stderr, "    ...\n");
    fprintf(stderr, "  },\n");
    fprintf(stderr, "},\n");
}

void md_nne_setup(struct md_exporter *mde, struct md_writer_nne* mwn)
{
    mwn->parent = mde;
    mwn->init = md_nne_init;
    mwn->handle = md_nne_handle;
}

