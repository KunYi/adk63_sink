/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.
Part of 6.3.0

FILE NAME
    operators.c

DESCRIPTION
    Implementation of Kymera operator helpers.
*/

#include <stdlib.h>
#include <string.h>

#include <vmal.h>
#include <panic.h>
#include <vmtypes.h>
#include <byte_utils.h>
#include <audio_input_common.h>

#include "operators.h"
#include "operators_constants.h"

#define USE_32BIT_PARAMETERS 0x1000
#define NUMBER_OF_PARAMS_PER_DATA_BLOCK 1
#define NUMBER_OF_PARAMS_PER_REQUEST_BLOCK 1
#define NUMBER_OF_PARAMS_MASK 0x0fff

#define MILLISECONDS_PER_SECOND 1000

/****************************************************************************
    Operator messages definitions.
    All fields have to be uint16 due to receiving processor requirements.
 */

typedef struct
{
    uint16 message_id; /* iir_resampler_message_id */
    uint16 conversion_rate;
} resampler_set_conversion_rate_message_t;


typedef struct
{
    uint16 id; /* iir_resampler_message_id */
    uint16 in_rate;
    uint16 out_rate;
} iir_resampler_set_sample_rate_msg_t;

typedef struct
{
    uint16 message_id;
    uint16 notes[MAX_NOTES_NUM];
} tone_set_notes_message_t;

typedef struct
{
    uint16 message_id;
    uint16 buffer_size;
} splitter_set_buffer_size_msg_t;

typedef struct
{
    uint16 message_id;
    uint16 running_streams;
} splitter_set_running_streams_msg_t;

typedef struct
{
    uint16 message_id;
    uint16 data_format;
} splitter_set_data_format_msg_t;

typedef struct
{
    uint16 id;
    uint16 working_mode;
} rtp_set_working_mode_msg_t;

typedef struct
{
    uint16 id;
    uint16 protection_enabled;
} rtp_set_content_protection_msg_t;

typedef struct
{
    uint16 id;
    uint16 codec_type;
} rtp_set_codec_type_msg_t;

typedef struct
{
    uint16 id;
    uint16 aac_op;
} rtp_set_aac_codec_msg_t;

typedef struct
{
    uint16 id;
    uint16 max_packet_length_in_octets;
} rtp_set_max_packet_length_msg_t;

typedef struct
{
    uint16 id;
    uint16 str1_ch;
    uint16 str2_ch;
    uint16 str3_ch;
} mixer_set_stream_chan_msg_t;

typedef struct
{
    uint16 id;
    uint16 gain_stream1;
    uint16 gain_stream2;
    uint16 gain_stream3;
} mixer_set_gain_msg_t;

typedef struct
{
    uint16 id;
    uint16 primary_stream;
} mixer_set_primary_msg_t;

typedef struct
{
    uint16 id;
    uint16 samples_most_significant_octet;
    uint16 samples_least_significant_word;
} mixer_set_number_of_samples_to_ramp_t;

typedef struct
{
    uint16 id;
    uint16 sample_rate; /* actual rate/25 */
} common_set_sample_rate_msg_t;

typedef struct
{
    uint16 id;
    uint16 in_rate;
    uint16 out_rate;
} aec_ref_set_sample_rates_msg_t;

typedef struct
{
    uint16 id;
    uint16 value;
} aec_ref_mute_mic_output_msg_t;

typedef struct
{
    uint16 id;
    uint16 enable;
    uint16 initial_delay;
    uint16 post_gate_drift_control;
} aec_ref_enable_spkr_input_gate_msg_t;

typedef struct
{
    uint16 id;
    uint16 sample_rate;
} spdif_set_output_sample_rate_msg_t;

typedef struct
{
    uint16 id;
    uint16 latency_most_significant_word;
    uint16 latency_least_significant_word;
} time_to_play_latency_msg_t;

typedef struct
{
    uint16 id;
    uint16 minimum_latency_most_significant_word;
    uint16 minimum_latency_least_significant_word;
    uint16 maximum_latency_most_significant_word;
    uint16 maximum_latency_least_significant_word;
} latency_limits_msg_t;

typedef struct
{
    uint16 control_id;
    uint16 msw;
    uint16 lsw;
} control_value_t;

typedef struct
{
    uint16 id;
    uint16 num_blocks;
    control_value_t values[];
} common_control_msg_t;

typedef struct
{
    uint16 id;
    uint16 data_format;
    uint16 sample_rate;
    uint16 number_of_channels;
    uint16 subframe_size;
    uint16 subframe_resolution;
} usb_audio_rx_configure_msg_t;

typedef struct
{
    uint16 id;
    uint16 buffer_size;
} buffer_size_msg_t;

typedef struct
{
    uint16 id;
    uint16 number_of_subbands;
    uint16 number_of_blocks;
    uint16 bitpool_size;
    uint16 sample_rate;
    uint16 channel_mode;
    uint16 allocation_method;
} sbc_encoder_set_encoding_params_msg_t;

typedef struct
{
    uint16 id;
    uint16 mode;
    uint16 frame_size;
    uint16 channels;
} celt_encoder_set_encoding_params_msg_t;

typedef struct
{
    uint16 id;
    uint16 ucid;
} set_ucid_msg_t;

typedef struct
{
    uint16 channel_id;
    uint16 gain;
} channel_gain_value_t;

typedef struct
{
    uint16 id;
    uint16 number_of_channels;
    channel_gain_value_t values[];
} mixer_set_channel_gain_msg_t;

typedef struct
{
    uint16 msw;
    uint16 lsw;
} value_32bit_t;

typedef struct
{
    uint16 id;
    uint16 number_of_params; /* always set to 1 */
    value_32bit_t value;
} standard_param_data_block_t;

typedef struct
{
    uint16 id;
    uint16 number_of_params; /* always set to 1 */
} standard_param_request_block_t;

typedef struct
{
    uint16 id;
    uint16 num_blocks;
    standard_param_data_block_t param_data_block[];
} standard_set_param_msg_t;

typedef struct
{
    uint16 id;
    uint16 num_blocks;
    standard_param_request_block_t param_request_block[];
} standard_get_param_msg_t;


typedef struct
{
    uint16 id;
    uint16 result;
    uint16 num_blocks;
    standard_param_data_block_t param_data_block[];
} standard_get_param_response_msg_t;


typedef struct
{
    uint16 most_significant_word;
    uint16 least_significant_word;
} source_sync_group_msg_t;

typedef struct
{
    uint16 id;
    uint16 number_of_groups;
    source_sync_group_msg_t groups[];
} source_sync_set_group_msg_t;

#define size_of_source_sync_group_message(number_of_groups) \
    (sizeof(source_sync_set_group_msg_t) + \
    (number_of_groups * sizeof(source_sync_group_msg_t)))

typedef struct
{
    uint16      output_terminal;
    uint16      input_terminal;
    uint16      sample_rate;
    uint16      gain;
    uint16      transition_samples;
} source_sync_route_msg_t;

typedef struct
{
    uint16 id;
    uint16 number_of_routes;
    source_sync_route_msg_t routes[];
} source_sync_set_routes_msg_t;

typedef struct
{
    uint16 lsw;
    uint16 msw;
} framework_kick_period_param_t;

#define size_of_source_sync_route_message(number_of_routes) \
    (sizeof(source_sync_set_routes_msg_t) + \
    (number_of_routes * sizeof(source_sync_route_msg_t)))

typedef struct
{
    uint16 msg_id;
    uint16 number_of_files;
    uint16 file_id;
}sva_data_msg_t;

typedef struct
{
    value_32bit_t ssrc_id;
    value_32bit_t ttp_in_us;
} ssrc_to_ttp_mapping_t;

typedef struct
{
    uint16 msg_id;
    uint16 number_of_ttp_entries;
    ssrc_to_ttp_mapping_t ttp_entry[];
} rtp_set_aptx_ad_ttp_t;

typedef struct
{
    uint16 msg_id;
    uint16 set_value;
}spc_msg_t;

/****************************************************************************
DESCRIPTION
    Convert music_processing_mode_t to a value understood by the dsp
 */
static uint16 getMusicProcessingMode(music_processing_mode_t mode_id)
{
    switch (mode_id)
    {
    case music_processing_mode_standby:
        return MUSIC_PROCESSING_MODE_STANDBY;
    case music_processing_mode_full_processing:
        return MUSIC_PROCESSING_MODE_FULL_PROCESSING;
    case music_processing_mode_passthrough:
        return MUSIC_PROCESSING_MODE_PASSTHROUGH;
    default:
        return 0;
    }
}

/****************************************************************************
DESCRIPTION
    Convert standard scaled dB gain to format accepted by mixer and volume capabilities.
 */
static uint16 scaledDbToDspGain(int scaled_db)
{
    return (uint16)(0x10000 + scaled_db);
}

/****************************************************************************
DESCRIPTION
    Create a priority key for use when creating operators.
 */
static void populateOperatorCreatePriorityKey(vmal_operator_keys_t * priority_key,
                                              operator_priority_t priority)
{
    memset(priority_key, 0, sizeof(*priority_key));
    priority_key->key = OPERATOR_CREATE_KEY_SET_PRIORITY;
    priority_key->value = priority;
}

/****************************************************************************
DESCRIPTION
    Create a processor ID key to define the processor on which the operator
    will be created.
 */
static void populateOperatorCreateProcessorIdKey(vmal_operator_keys_t * processor_id_key,
                                                 operator_processor_id_t processor_id)
{
    memset(processor_id_key, 0, sizeof(*processor_id_key));
    processor_id_key->key = OPERATOR_CREATE_KEY_SET_PROCESSOR_ID;
    processor_id_key->value = processor_id;
}

/****************************************************************************
DESCRIPTION
    Fill in a set group message
 */
static void operatorsSetGroupMessage(uint32 group_config, source_sync_group_msg_t* group)
{
    group->most_significant_word  = (uint16)(0xffff & (group_config >> 16));
    group->least_significant_word = (uint16)(0xffff & (group_config));
}

/****************************************************************************
DESCRIPTION
    Fill in a set source group message
 */
static void operatorsSetSourceGroupMessage(uint16 number_of_groups, const source_sync_source_group_t* groups, source_sync_set_group_msg_t* message)
{
    int i;

    message->number_of_groups = number_of_groups;

    for(i = 0; i < number_of_groups; i++)
    {
        const source_sync_source_group_t* group = &groups[i];
        uint32 group_config = group->channel_mask;

        if(group->meta_data_required)
            group_config |= SOURCE_SYNC_GROUP_META_DATA_FLAG;

        if(group->ttp_required)
            group_config |= SOURCE_SYNC_GROUP_TTP_FLAG;

        operatorsSetGroupMessage(group_config, &message->groups[i]);
    }
}

/****************************************************************************
DESCRIPTION
    Fill in a set sink group message
 */
static void operatorsSetSinkGroupMessage(uint16 number_of_groups, const source_sync_sink_group_t* groups, source_sync_set_group_msg_t* message)
{
    int i;

    message->number_of_groups = number_of_groups;

    for(i = 0; i < number_of_groups; i++)
    {
        const source_sync_sink_group_t* group = &groups[i];

        uint32 group_config = group->channel_mask;

        if(group->meta_data_required)
            group_config |= SOURCE_SYNC_GROUP_META_DATA_FLAG;

        if(group->rate_match)
            group_config |= SOURCE_SYNC_GROUP_RATE_MATCH_FLAG;

        operatorsSetGroupMessage(group_config, &message->groups[i]);
    }
}

/****************************************************************************
DESCRIPTION
    Function to set a single parameter
 */
static void operatorsSetSingleParameter(Operator op, const standard_param_t* parameter)
{
    set_params_data_t* set_params_data = OperatorsCreateSetParamsData(1);

    set_params_data->number_of_params = 1;
    set_params_data->standard_params[0].id = parameter->id;
    set_params_data->standard_params[0].value = parameter->value;

    OperatorsStandardSetParameters(op, set_params_data);
    free(set_params_data);
}

/****************************************************************************
DESCRIPTION
    Apply a configuration item
 */
static void operatorsApplySetupItem(Operator op, const operator_setup_item_t* item)
{
    switch(item->key)
    {
        case operators_setup_buffer_size:
            OperatorsStandardSetBufferSize(op, item->value.buffer_size);
            break;

        case operators_setup_parameter:
            operatorsSetSingleParameter(op, &item->value.parameter);
            break;

        case operators_setup_buffer_latency:
            /* Handled in OperatorsStandardSetBufferSizeFromSampleRate */
            break;

        case operators_setup_usb_config:
            OperatorsUsbAudioSetConfig(op, item->value.usb_config);
            break;

        case operators_setup_sample_rate:
            OperatorsStandardSetSampleRate(op, item->value.sample_rate);
            break;

        case operators_setup_switched_passthrough_set_format:
            OperatorsSetSwitchedPassthruEncoding(op, item->value.spc_format);
            break;

        default:
            Panic();
            break;
    }
}

/****************************************************************************
DESCRIPTION
    Apply full configuration
 */
static void operatorsApplySetup(Operator op, const operator_setup_t* setup)
{
    unsigned i;

    if(setup)
    {
        for(i = 0; i < setup->num_items; i++)
        {
            operatorsApplySetupItem(op, &setup->items[i]);
        }
    }
}

/****************************************************************************
DESCRIPTION
    Extract operators_setup_buffer_latency from operator_setup_t if it exists
 */
static unsigned operatorsGetBufferLatencyFromSetup(const operator_setup_t* setup)
{
    unsigned i;

    if(setup)
    {
        for(i = 0; i < setup->num_items; i++)
        {
            const operator_setup_item_t* item = &setup->items[i];
            if(item->key == operators_setup_buffer_latency)
            {
                return item->value.buffer_latency;
            }
        }
    }

    return 0;
}

/****************************************************************************
    Low level API
*/

void OperatorsFrameworkEnable(void)
{
    PanicFalse(VmalOperatorFrameworkEnableMainProcessor(TRUE));
}

void OperatorsFrameworkDisable(void)
{
    PanicFalse(VmalOperatorFrameworkEnableMainProcessor(FALSE));
}

bool OperatorsFrameworkSetKickPeriod(unsigned kick_period)
{
    framework_kick_period_param_t param;
    param.lsw = LOWORD(kick_period);
    param.msw = HIWORD(kick_period);

    return OperatorFrameworkConfigurationSet(FRAMEWORK_KICK_PERIOD_PARAM, (uint16 *)&param, SIZEOF_OPERATOR_MESSAGE(param));
}

Operator OperatorsCreate(capability_id_t id, operator_processor_id_t processor_id, operator_priority_t priority)
{
    return OperatorsCreateWithSetup(id, processor_id, priority, NULL);
}

Operator OperatorsCreateWithSetup(capability_id_t id, operator_processor_id_t processor_id, operator_priority_t priority, const operator_setup_t* setup)
{
    Operator op;
    uint16 num_keys = 0;
    vmal_operator_keys_t keys[2];

    if(processor_id != OPERATOR_PROCESSOR_ID_0)
    {
        populateOperatorCreateProcessorIdKey(&keys[num_keys++], processor_id);
    }

    populateOperatorCreatePriorityKey(&keys[num_keys++], priority);

    op = VmalOperatorCreateWithKeys(id, num_keys ? keys : NULL, num_keys);
    PanicZero(op);

    operatorsApplySetup(op, setup);

    return op;
}

void OperatorsDestroy(Operator *operators, unsigned number_of_operators)
{
    PanicFalse(OperatorDestroyMultiple((uint16) number_of_operators, operators, NULL));
}

/****************************************************************************
    Messages sending low level API.
    Messages content is sent as array of 16 bit words and size is a number of 16 bit words.
*/

static uint16 getLegacyResamplerRateId(unsigned rate)
{
    switch (rate)
    {
        case 8000:
            return 0;
        case 11025:
            return 1;
        case 12000:
            return 2;
        case 16000:
            return 3;
        case 22050:
            return 4;
        case 24000:
            return 5;
        case 32000:
            return 6;
        case 44100:
            return 7;
        case 48000:
            return 8;
        default:
            /* Unsupported rate for legacy resampler */
            Panic();
    }
    /* Return can't be reached due to the Panic, but compiler misses that */
    return (uint16)-1;
}

void OperatorsLegacyResamplerSetConversionRate(Operator opid, unsigned input_sample_rate, unsigned output_sample_rate)
{
    resampler_set_conversion_rate_message_t msg;
    msg.message_id = RESAMPLER_SET_CONVERSION_RATE;
    msg.conversion_rate = (uint16)(  getLegacyResamplerRateId(input_sample_rate) * 16
                                   + getLegacyResamplerRateId(output_sample_rate));

    PanicFalse(VmalOperatorMessage(opid, &msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsResamplerSetConversionRate(Operator opid, unsigned input_sample_rate, unsigned output_sample_rate)
{
    iir_resampler_set_sample_rate_msg_t msg;
    msg.id = IIR_RESAMPLER_SET_SAMPLE_RATES;
    msg.in_rate = (uint16)(input_sample_rate/25);
    msg.out_rate = (uint16)(output_sample_rate/25);

    PanicFalse(VmalOperatorMessage(opid, &msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsToneSetNotes(Operator opid, const ringtone_note * tone)
{
    uint16 note_index;
    tone_set_notes_message_t msg;
    const unsigned *tone_internals;

    tone_internals = (const unsigned *)tone;
    PanicFalse(tone_internals != 0);

    msg.message_id = 1;

    for(note_index = 0; note_index < MAX_NOTES_NUM; ++note_index)
    {
        msg.notes[note_index] = (uint16)tone_internals[note_index];

        if(tone_internals[note_index] == (unsigned)RINGTONE_END)
        {
            break;
        }
    }

    if(note_index > 0)
    {
        /* 1 is added to note_index to accommodate for zero indexing
           and another 1 is for message id */
        uint16 message_len = (uint16)(note_index + 2);

        PanicFalse(VmalOperatorMessage(opid, &msg, message_len, NULL, 0));
    }
}

void OperatorsSplitterEnableSecondOutput(Operator opid, bool is_second_output_active)
{
    splitter_set_running_streams_msg_t msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(msg);

    msg.message_id = SPLITTER_SET_RUNNING_STREAMS;
    msg.running_streams = (uint16)(is_second_output_active ?
            SPLITTER_BOTH_OUTPUTS_ACTIVE : SPLITTER_FIRST_OUTPUT_ACTIVE);

    PanicFalse(VmalOperatorMessage(opid, &msg, msg_size, NULL, 0));
}

void OperatorsSplitterSetDataFormat(Operator opid, operator_data_format_t data_format)
{
    splitter_set_data_format_msg_t msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(msg);

    msg.message_id = SPLITTER_SET_DATA_FORMAT;
    msg.data_format = (uint16)(data_format == operator_data_format_pcm ?
            SET_DATA_FORMAT_PCM : SET_DATA_FORMAT_ENCODED);

    PanicFalse(VmalOperatorMessage(opid, &msg, msg_size, NULL, 0));
}

void OperatorsRtpSetWorkingMode(Operator op, rtp_working_mode_t mode)
{
    rtp_set_working_mode_msg_t rtp_msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(rtp_msg);

    rtp_msg.id = RTP_SET_MODE;
    rtp_msg.working_mode = mode;

    PanicFalse(VmalOperatorMessage(op, &rtp_msg, msg_size, NULL, 0));
}

void OperatorsRtpSetContentProtection(Operator op, bool protection_enabled)
{
    rtp_set_content_protection_msg_t rtp_msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(rtp_msg);

    rtp_msg.id = RTP_SET_CONTENT_PROTECTION;
    rtp_msg.protection_enabled = (protection_enabled ? 1 : 0);

    PanicFalse(VmalOperatorMessage(op, &rtp_msg, msg_size, NULL, 0));
}

void OperatorsRtpSetCodecType(Operator op, rtp_codec_type_t codec_type)
{
    rtp_set_codec_type_msg_t rtp_msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(rtp_msg);

    rtp_msg.id = RTP_SET_CODEC_TYPE;
    rtp_msg.codec_type = codec_type;

    PanicFalse(VmalOperatorMessage(op, &rtp_msg, msg_size, NULL, 0));
}

static value_32bit_t split32BitWordTo16Bits(uint32 input)
{
    value_32bit_t output;

    output.msw = (uint16) ((input >> 16) & 0xFFFF);
    output.lsw = input & 0xFFFF;

    return output;
}

static ssrc_to_ttp_mapping_t getSsrcToTtpMappingEntry(aptx_adaptive_ssrc_ids_t ssrc_id, unsigned ttp_in_ms)
{
    ssrc_to_ttp_mapping_t entry;

    entry.ssrc_id = split32BitWordTo16Bits(ssrc_id);
    entry.ttp_in_us = split32BitWordTo16Bits(TTP_LATENCY_IN_US(ttp_in_ms));

    return entry;
}

void OperatorsRtpSetAptxAdaptiveTTPLatency(Operator op, aptx_adaptive_ttp_in_ms_t aptx_ad_ttp)
{
    unsigned number_of_ttp_entries = 4;
    unsigned size_of_msg = (unsigned)sizeof(rtp_set_aptx_ad_ttp_t) + number_of_ttp_entries * (unsigned)sizeof(ssrc_to_ttp_mapping_t);

    rtp_set_aptx_ad_ttp_t *msg = PanicUnlessMalloc(size_of_msg);

    msg->msg_id = RTP_SET_SSRC_LATENCY_MAPPING;
    msg->number_of_ttp_entries = (uint16) number_of_ttp_entries;

    msg->ttp_entry[0] = getSsrcToTtpMappingEntry(aptx_ad_ll_0_ssrc_id, aptx_ad_ttp.low_latency_0);
    msg->ttp_entry[1] = getSsrcToTtpMappingEntry(aptx_ad_ll_1_ssrc_id, aptx_ad_ttp.low_latency_1);
    msg->ttp_entry[2] = getSsrcToTtpMappingEntry(aptx_ad_hq_ssrc_id, aptx_ad_ttp.high_quality);
    msg->ttp_entry[3] = getSsrcToTtpMappingEntry(aptx_ad_tws_ssrc_id, aptx_ad_ttp.tws_legacy);

    PanicFalse(VmalOperatorMessage(op, msg, (uint16) (size_of_msg / sizeof(uint16)), NULL, 0));

    free(msg);
}

void OperatorsRtpSetAacCodec(Operator op, Operator aac_op)
{
    rtp_set_aac_codec_msg_t rtp_msg;
    uint16 size_msg = SIZEOF_OPERATOR_MESSAGE(rtp_msg);

    rtp_msg.id = RTP_SET_AAC_CODEC;
    rtp_msg.aac_op = aac_op;

    PanicFalse(VmalOperatorMessage(op, &rtp_msg, size_msg, NULL, 0));
}

void OperatorsRtpSetMaximumPacketLength(Operator op, uint16 packet_length_in_octets)
{
    rtp_set_max_packet_length_msg_t rtp_msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(rtp_msg);

    rtp_msg.id = RTP_SET_MAX_PACKET_LENGTH;
    rtp_msg.max_packet_length_in_octets = packet_length_in_octets;

    PanicFalse(VmalOperatorMessage(op, &rtp_msg, msg_size, NULL, 0));
}

void OperatorsMixerSetChannelsPerStream(Operator op, unsigned str1_ch, unsigned str2_ch, unsigned str3_ch)
{
    mixer_set_stream_chan_msg_t set_chan_msg;

    set_chan_msg.id = MIXER_SET_STREAM_CHANNELS;
    set_chan_msg.str1_ch = (uint16)str1_ch;
    set_chan_msg.str2_ch = (uint16)str2_ch;
    set_chan_msg.str3_ch = (uint16)str3_ch;

    PanicFalse(VmalOperatorMessage(op, &set_chan_msg, SIZEOF_OPERATOR_MESSAGE(set_chan_msg), NULL, 0));
}

void OperatorsMixerSetGains(Operator op, int str1_gain, int str2_gain, int str3_gain)
{
    mixer_set_gain_msg_t set_gain_msg;

    set_gain_msg.id = MIXER_SET_GAINS;
    set_gain_msg.gain_stream1 = scaledDbToDspGain(str1_gain);
    set_gain_msg.gain_stream2 = scaledDbToDspGain(str2_gain);
    set_gain_msg.gain_stream3 = scaledDbToDspGain(str3_gain);

    PanicFalse(VmalOperatorMessage(op, &set_gain_msg, SIZEOF_OPERATOR_MESSAGE(set_gain_msg), NULL, 0));
}

void OperatorsMixerSetPrimaryStream(Operator op, unsigned primary_stream)
{
    mixer_set_primary_msg_t set_prim_chan_msg;

    set_prim_chan_msg.id = MIXER_SET_PRIMARY_STREAM;
    set_prim_chan_msg.primary_stream = (uint16)primary_stream;

    PanicFalse(VmalOperatorMessage(op, &set_prim_chan_msg, SIZEOF_OPERATOR_MESSAGE(set_prim_chan_msg), NULL, 0));
}

void OperatorsMixerSetNumberOfSamplesToRamp(Operator op, unsigned number_of_samples)
{
    mixer_set_number_of_samples_to_ramp_t set_ramp_msg;

    set_ramp_msg.id = MIXER_SET_RAMP_NUM_SAMPLES;
    set_ramp_msg.samples_most_significant_octet = (number_of_samples >> 16) & 0xff;
    set_ramp_msg.samples_least_significant_word = (number_of_samples) & 0xffff;

    PanicFalse(VmalOperatorMessage(op, &set_ramp_msg, SIZEOF_OPERATOR_MESSAGE(set_ramp_msg), NULL, 0));
}

void OperatorsVolumeSetMainGain(Operator op, int gain)
{
    OperatorsStandardSetControl(op, MAIN_GAIN, (unsigned)gain);
}

void OperatorsVolumeSetPostGain(Operator op, int gain)
{
    OperatorsStandardSetControl(op, POST_GAIN, (unsigned)gain);
}

void OperatorsVolumeMute(Operator op, bool enable)
{
    OperatorsStandardSetControl(op, OPMSG_CONTROL_MUTE_ID, (unsigned)enable);
}

void OperatorsVolumeSetAuxGain(Operator op, int gain)
{
    operator_set_control_data_t control[2];
    control[0].control_id = AUX_GAIN1;
    control[0].value = (unsigned)gain;
    control[1].control_id = AUX_GAIN2;
    control[1].value = (unsigned)gain;
    OperatorsStandardSetControlMultiple(op, 2, control);
}

void OperatorsVolumeSetTrims(Operator op, const volume_trims_t* trims)
{
    operator_set_control_data_t control[4];

    control[0].control_id = TRIM_GAIN1;
    control[0].value = (unsigned)trims->primary_left;
    control[1].control_id = TRIM_GAIN2;
    control[1].value = (unsigned)trims->primary_right;
    control[2].control_id = TRIM_GAIN3;
    control[2].value = (unsigned)trims->secondary_left;
    control[3].control_id = TRIM_GAIN4;
    control[3].value = (unsigned)trims->secondary_right;

    OperatorsStandardSetControlMultiple(op, 4, control);
}

void OperatorsVolumeSetMainAndAuxGain(Operator op, int gain)
{
    operator_set_control_data_t control[3];
    control[0].control_id = MAIN_GAIN;
    control[0].value = (unsigned)gain;
    control[1].control_id = AUX_GAIN1;
    control[1].value = (unsigned)gain;
    control[2].control_id = AUX_GAIN2;
    control[2].value = (unsigned)gain;
    OperatorsStandardSetControlMultiple(op, 3, control);
}

void OperatorsAecSetSampleRate(Operator op, unsigned in_rate, unsigned out_rate)
{
    aec_ref_set_sample_rates_msg_t aec_set_sample_rate_msg;

    /* configure the operator for the sample rate */
    /* rates sent in directly (capability does not comply to rate/25 standard !!! */
    aec_set_sample_rate_msg.id = AEC_REF_SET_SAMPLE_RATES;
    aec_set_sample_rate_msg.in_rate = (uint16)in_rate;
    aec_set_sample_rate_msg.out_rate = (uint16)out_rate;

    PanicFalse(VmalOperatorMessage(op, &aec_set_sample_rate_msg, SIZEOF_OPERATOR_MESSAGE(aec_set_sample_rate_msg), NULL, 0));
}

void OperatorsAecEnableTtpGate(Operator op, bool enable, uint16 initial_delay_ms, bool control_drift)
{
    aec_ref_enable_spkr_input_gate_msg_t aec_enable_gate_msg;

    aec_enable_gate_msg.id = AEC_REF_ENABLE_SPKR_GATE;
    aec_enable_gate_msg.enable = (uint16)enable;
    aec_enable_gate_msg.initial_delay = initial_delay_ms;
    aec_enable_gate_msg.post_gate_drift_control = (uint16)control_drift;

    PanicFalse(VmalOperatorMessage(op, &aec_enable_gate_msg, SIZEOF_OPERATOR_MESSAGE(aec_enable_gate_msg), NULL, 0));
}

void OperatorsAecMuteMicOutput(Operator op, bool enable)
{
    aec_ref_mute_mic_output_msg_t aec_ref_mute_mic_output_msg;

    aec_ref_mute_mic_output_msg.id = AEC_REF_MUTE_MIC_OUTPUT;
    aec_ref_mute_mic_output_msg.value = (uint16)enable;

    PanicFalse(VmalOperatorMessage(op, &aec_ref_mute_mic_output_msg, SIZEOF_OPERATOR_MESSAGE(aec_ref_mute_mic_output_msg), NULL, 0));
}

void OperatorsSpdifSetOutputSampleRate(Operator op, unsigned sample_rate)
{
    spdif_set_output_sample_rate_msg_t output_sample_rate_msg;

    output_sample_rate_msg.id = SPDIF_SET_OUTPUT_SAMPLE_RATE;
    output_sample_rate_msg.sample_rate =(uint16)(sample_rate / 25);

    PanicFalse(VmalOperatorMessage(op, &output_sample_rate_msg, SIZEOF_OPERATOR_MESSAGE(spdif_set_output_sample_rate_msg_t), NULL, 0));
}

void OperatorsUsbAudioSetConfig(Operator op, usb_config_t config)
{
    usb_audio_rx_configure_msg_t msg;

    msg.id = USB_AUDIO_SET_CONNECTION_CONFIG;
    msg.data_format = USB_AUDIO_DATA_FORMAT_PCM;
    msg.sample_rate = (uint16)(config.sample_rate / 25);
    msg.number_of_channels = (uint16)(config.number_of_channels);
    /* subframe size in bits */
    msg.subframe_size = (uint16)(config.sample_size * 8);
    /* subframe resolution in bits */
    msg.subframe_resolution = (uint16)(config.sample_size * 8);

    PanicFalse(VmalOperatorMessage(op, (void*)&msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsSbcEncoderSetEncodingParams(Operator op, sbc_encoder_params_t *params)
{
    sbc_encoder_set_encoding_params_msg_t msg;

    PanicNull(params);

    msg.id = SBC_ENCODER_SET_ENCODING_PARAMS;
    msg.number_of_subbands = (uint16)params->number_of_subbands;
    msg.number_of_blocks = (uint16)params->number_of_blocks;
    msg.bitpool_size = (uint16)params->bitpool_size;
    msg.sample_rate = (uint16)(params->sample_rate / 25);
    msg.channel_mode = (uint16)params->channel_mode;
    msg.allocation_method = (uint16)params->allocation_method;

    PanicFalse(VmalOperatorMessage(op, (void*)&msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsCeltEncoderSetEncoderParams(Operator op, celt_encoder_params_t *params)
{
    celt_encoder_set_encoding_params_msg_t msg;

    PanicNull(params);

    msg.id = CELT_ENCODER_SET_ENCODING_PARAMS;
    msg.mode = (params->sample_rate == 44100 ? CELT_ENC_MODE_512_44100 : CELT_ENC_MODE_512_48000);
    msg.frame_size = (params->frame_size == 0 ? CELT_CODEC_FRAME_SIZE_DEFAULT : params->frame_size);
    msg.channels = CELT_ENC_MODE_STEREO;

    PanicFalse(VmalOperatorMessage(op, (void*)&msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsSetMusicProcessingMode(Operator op, music_processing_mode_t mode)
{
    OperatorsStandardSetControl(op, OPMSG_CONTROL_MODE_ID, getMusicProcessingMode(mode));
}

void OperatorsStandardSetBufferSize(Operator op, unsigned buffer_size)
{
    buffer_size_msg_t msg;

    msg.id = SET_BUFFER_SIZE;
    msg.buffer_size = (uint16)buffer_size;

    PanicFalse(VmalOperatorMessage(op, (void*)&msg, SIZEOF_OPERATOR_MESSAGE(msg), NULL, 0));
}

void OperatorsSetPassthroughDataFormat(Operator op, operator_data_format_t data_format)
{
    splitter_set_data_format_msg_t msg;
    uint16 msg_size = SIZEOF_OPERATOR_MESSAGE(msg);

    msg.data_format = (uint16)(data_format == operator_data_format_pcm ? 1 : 0);

    msg.message_id = PASSTHRUOGH_SET_INPUT_DATA_FORMAT;
    PanicFalse(VmalOperatorMessage(op, &msg, msg_size, NULL, 0));

    msg.message_id = PASSTHRUOGH_SET_OUTPUT_DATA_FORMAT;
    PanicFalse(VmalOperatorMessage(op, &msg, msg_size, NULL, 0));
}

/* Scaling factor to convert a scaled dB gain to a scaled dB gain in log2 Q6N format.
 * The scaling factor is 2^(32-6) / 20log2 / DB_DSP_SCALING_FACTOR
 */
#define GAIN_SCALED_DB_TO_LOG2_Q6N_SF (185776)

void OperatorsSetPassthroughGain(Operator op, int32 gain_db_scaled)
{
    int32 value = (gain_db_scaled * GAIN_SCALED_DB_TO_LOG2_Q6N_SF);
    standard_param_t parameter = {1, (standard_param_value_t)value};
    operatorsSetSingleParameter(op, &parameter);
}

/****************************************************************************
    Messages sending low level API and supported by multiple operator types.
    Messages content is sent as array of 16 bit words and size is a number of 16 bit words.
*/

void OperatorsStandardSetSampleRate(Operator op, unsigned sample_rate)
{
    common_set_sample_rate_msg_t set_sample_rate_msg;

    set_sample_rate_msg.id = SET_SAMPLE_RATE;
    set_sample_rate_msg.sample_rate = (uint16)(sample_rate / 25);

    PanicFalse(VmalOperatorMessage(op, &set_sample_rate_msg, SIZEOF_OPERATOR_MESSAGE(set_sample_rate_msg), NULL, 0));
}

void OperatorsStandardSetBufferSizeFromSampleRate(Operator op, uint32 sample_rate, const operator_setup_t* setup)
{
    unsigned buffer_latency = operatorsGetBufferLatencyFromSetup(setup);
    if(buffer_latency)
    {
        unsigned buffer_size = (buffer_latency * sample_rate / MILLISECONDS_PER_SECOND);
        OperatorsStandardSetBufferSize(op, buffer_size);
    }
}

void OperatorsStandardFadeOut(Operator op, bool enable)
{
    uint16 generic_fade_msg;
    generic_fade_msg = enable ? ENABLE_FADE_OUT : DISABLE_FADE_OUT;

    PanicFalse(VmalOperatorMessage(op, &generic_fade_msg, SIZEOF_OPERATOR_MESSAGE(generic_fade_msg), NULL, 0));
}

void OperatorsStandardSetControl(Operator op, unsigned control_id, unsigned value)
{
    operator_set_control_data_t control;
    control.control_id = control_id;
    control.value = value;
    OperatorsStandardSetControlMultiple(op, 1, &control);
}

void OperatorsStandardSetControlMultiple(Operator op, unsigned count, const operator_set_control_data_t *controls)
{
    unsigned i;
    size_t data_len = sizeof(common_control_msg_t) + count * sizeof(control_value_t);

    common_control_msg_t *ctrl = PanicUnlessMalloc(data_len);

    ctrl->id = SET_CONTROL;

    ctrl->num_blocks = (uint16)(count);

    for (i = 0; i < count; i++)
    {
        ctrl->values[i].control_id = (uint16)(controls[i].control_id);
        ctrl->values[i].msw = (uint16)(controls[i].value >> 16);
        ctrl->values[i].lsw = (uint16)(controls[i].value);
    }

    PanicFalse(VmalOperatorMessage(op, ctrl, SIZEOF_OPERATOR_MESSAGE_ARRAY(data_len), NULL, 0));

    free(ctrl);
}

void OperatorsStandardSetTimeToPlayLatency(Operator op, uint32 time_to_play)
{
    time_to_play_latency_msg_t latency_msg;
    latency_msg.id = SET_TIME_TO_PLAY_LATENCY;
    latency_msg.latency_most_significant_word = (uint16)(time_to_play >> 16);
    latency_msg.latency_least_significant_word = (uint16)(time_to_play & 0xffff);

    PanicFalse(VmalOperatorMessage(op, &latency_msg, SIZEOF_OPERATOR_MESSAGE(latency_msg), NULL, 0));
}

void OperatorsStandardSetLatencyLimits(Operator op, uint32 minimum_latency, uint32 maximum_latency)
{
    latency_limits_msg_t latency_limits_msg;

    latency_limits_msg.id = SET_LATENCY_LIMITS;

    latency_limits_msg.minimum_latency_most_significant_word = (uint16)(minimum_latency >> 16);
    latency_limits_msg.minimum_latency_least_significant_word = (uint16)(minimum_latency & 0xffff);

    latency_limits_msg.maximum_latency_most_significant_word = (uint16)(maximum_latency >> 16);
    latency_limits_msg.maximum_latency_least_significant_word = (uint16)(maximum_latency & 0xffff);

    PanicFalse(VmalOperatorMessage(op, &latency_limits_msg, SIZEOF_OPERATOR_MESSAGE(latency_limits_msg), NULL, 0));
}

void OperatorsStandardSetUCID(Operator op, unsigned ucid)
{
    set_ucid_msg_t ucid_msg;
    ucid_msg.id = SET_UCID;
    ucid_msg.ucid = (uint16)ucid;

    PanicFalse(VmalOperatorMessage(op, &ucid_msg, SIZEOF_OPERATOR_MESSAGE(ucid_msg), NULL, 0));
}

void OperatorsMixerSetChannelsGains(Operator op,uint16 number_of_channels,const mixer_channel_gain_t *channels_gains)
{
    size_t msg_len = sizeof(mixer_set_channel_gain_msg_t) + number_of_channels * sizeof(channel_gain_value_t);
    uint16 i;

    mixer_set_channel_gain_msg_t *msg = PanicUnlessMalloc(msg_len);

    msg->id = MIXER_SET_CHANNEL_GAINS;
    msg->number_of_channels = number_of_channels;

    for(i = 0; i < number_of_channels; i++)
    {
        msg->values[i].channel_id = (uint16)channels_gains[i].channel_id;
        msg->values[i].gain       = (uint16)scaledDbToDspGain(channels_gains[i].gain);
    }

    PanicFalse(VmalOperatorMessage(op, msg, SIZEOF_OPERATOR_MESSAGE_ARRAY(msg_len), NULL, 0));

    free(msg);
}

void OperatorsSourceSyncSetSinkGroups(Operator op, uint16 number_of_groups, const source_sync_sink_group_t* groups)
{
    size_t size_message = size_of_source_sync_group_message(number_of_groups);
    source_sync_set_group_msg_t* message = PanicUnlessMalloc(size_message);

    message->id = SOURCE_SYNC_SET_SINK_GROUP;
    operatorsSetSinkGroupMessage(number_of_groups, groups, message);

    PanicFalse(VmalOperatorMessage(op, message, SIZEOF_OPERATOR_MESSAGE_ARRAY(size_message), NULL, 0));

    free(message);
}

void OperatorsSourceSyncSetSourceGroups(Operator op, uint16 number_of_groups, const source_sync_source_group_t* groups)
{
    size_t size_message = size_of_source_sync_group_message(number_of_groups);
    source_sync_set_group_msg_t* message = PanicUnlessMalloc(size_message);

    message->id = SOURCE_SYNC_SET_SOURCE_GROUP;
    operatorsSetSourceGroupMessage(number_of_groups, groups, message);

    PanicFalse(VmalOperatorMessage(op, message, SIZEOF_OPERATOR_MESSAGE_ARRAY(size_message), NULL, 0));

    free(message);
}

void OperatorsSourceSyncSetRoutes(Operator op, uint16 number_of_routes, const source_sync_route_t* routes)
{
    int i;
    size_t size_message = size_of_source_sync_route_message(number_of_routes);
    source_sync_set_routes_msg_t* message = PanicUnlessMalloc(size_message);

    message->id = SOURCE_SYNC_SET_ROUTES;
    message->number_of_routes = number_of_routes;

    for(i = 0; i < number_of_routes; i++)
    {
        message->routes[i].input_terminal       = routes[i].input_terminal;
        message->routes[i].output_terminal      = routes[i].output_terminal;
        message->routes[i].transition_samples   = (uint16)routes[i].transition_samples;
        message->routes[i].sample_rate          = (uint16)(routes[i].sample_rate / 25);
        message->routes[i].gain                 = (uint16)routes[i].gain;
    }

    PanicFalse(VmalOperatorMessage(op, message, SIZEOF_OPERATOR_MESSAGE_ARRAY(size_message), NULL, 0));

    free(message);
}

/****************************************************************************
DESCRIPTION
    Calculate size of set params message given number of params.
 */
static unsigned getSetParamMessageSize(unsigned number_of_params)
{
    return (unsigned) sizeof(standard_set_param_msg_t) + (unsigned) sizeof(standard_param_data_block_t) * number_of_params;
}

void OperatorsStandardSetParameters(Operator op, const set_params_data_t* set_params_data)
{
    /* Data is always stored with 1 parameter value per Parameter Data Block */
    const unsigned number_of_params = set_params_data->number_of_params;
    unsigned message_size = getSetParamMessageSize(number_of_params);

    standard_set_param_msg_t* set_param_msg = PanicUnlessMalloc(message_size);

    set_param_msg->id = SET_PARAMS;
    set_param_msg->num_blocks = (uint16)((number_of_params & NUMBER_OF_PARAMS_MASK) | USE_32BIT_PARAMETERS);
    unsigned i;
    for (i = 0; i < number_of_params; i++)
    {
        set_param_msg->param_data_block[i].id = set_params_data->standard_params[i].id;
        set_param_msg->param_data_block[i].number_of_params = NUMBER_OF_PARAMS_PER_DATA_BLOCK;
        set_param_msg->param_data_block[i].value.msw = (uint16)(set_params_data->standard_params[i].value >> 16);
        set_param_msg->param_data_block[i].value.lsw = (uint16)(set_params_data->standard_params[i].value & 0x0000ffff);
    }

    PanicFalse(VmalOperatorMessage(op, set_param_msg, SIZEOF_OPERATOR_MESSAGE_ARRAY(message_size), NULL, 0));
    free(set_param_msg);
}

/****************************************************************************
DESCRIPTION
    Decode get params response message into a simple array of parameters.
*/
static void operatorsDecodeGetParamsResponse(unsigned number_of_params,
                                             standard_get_param_response_msg_t* get_param_response_msg,
                                             get_params_data_t* get_params_data)
{
    unsigned i;

    if ((get_param_response_msg->num_blocks & NUMBER_OF_PARAMS_MASK) != number_of_params)
        Panic();

    if (get_param_response_msg->result != obpm_ok)
        number_of_params = 0;

    get_params_data->result = get_param_response_msg->result;
    get_params_data->number_of_params = get_param_response_msg->num_blocks;

    for (i = 0; i < number_of_params; i++)
    {
        get_params_data->standard_params[i].id = get_param_response_msg->param_data_block[i].id;
        get_params_data->standard_params[i].value = (unsigned)((get_param_response_msg->param_data_block[i].value.msw << 16) | (get_param_response_msg->param_data_block[i].value.lsw));
    }
}

/****************************************************************************
DESCRIPTION
    Calculate size of get params message given number of params.
 */
static unsigned getGetParamMessageSize(unsigned number_of_params)
{
    return (unsigned) sizeof(standard_get_param_msg_t) + (unsigned) sizeof(standard_param_request_block_t) * number_of_params;
}

/****************************************************************************
DESCRIPTION
    Calculate size of get params response message given number of params.
 */
static unsigned getGetParamResponseMessageSize(unsigned number_of_params)
{
    return (unsigned) sizeof(standard_get_param_response_msg_t) + (unsigned) sizeof(standard_param_data_block_t) * number_of_params;
}

void OperatorsStandardGetParameters(Operator op, get_params_data_t* get_params_data)
{
    const unsigned number_of_params = get_params_data->number_of_params;

    /* Data is always stored with 1 parameter value per Parameter Request Block */
    unsigned message_size = getGetParamMessageSize(number_of_params);
    standard_get_param_msg_t* get_param_msg = PanicUnlessMalloc(message_size);

    unsigned response_message_size = getGetParamResponseMessageSize(number_of_params);
    standard_get_param_response_msg_t* get_param_resp_msg = PanicUnlessMalloc(response_message_size);

    get_param_msg->id = GET_PARAMS;
    get_param_msg->num_blocks = (uint16)((number_of_params & NUMBER_OF_PARAMS_MASK) | USE_32BIT_PARAMETERS);
    unsigned i;
    for (i = 0; i < number_of_params; i++)
    {
        get_param_msg->param_request_block[i].id = get_params_data->standard_params[i].id;
        get_param_msg->param_request_block[i].number_of_params = NUMBER_OF_PARAMS_PER_REQUEST_BLOCK;
    }

    PanicFalse(VmalOperatorMessage(op, get_param_msg, SIZEOF_OPERATOR_MESSAGE_ARRAY(message_size),
                                   get_param_resp_msg, SIZEOF_OPERATOR_MESSAGE_ARRAY(response_message_size)));
    free(get_param_msg);

    operatorsDecodeGetParamsResponse(number_of_params, get_param_resp_msg, get_params_data);
    free(get_param_resp_msg);
}

/****************************************************************************
    Higher level API
*/

void OperatorsConfigureResampler(Operator resampler_op, unsigned input_sample_rate, unsigned output_sample_rate)
{
    OperatorsResamplerSetConversionRate(resampler_op, input_sample_rate, output_sample_rate);
}

void OperatorsConfigureToneGenerator(Operator tone_op, const ringtone_note *tone, Task listener)
{
    OperatorsToneSetNotes(tone_op, tone);
    MessageOperatorTask(tone_op, listener);
}

void OperatorsConfigureSplitter(Operator splitter_op, unsigned buffer_size, bool is_second_output_active,
        operator_data_format_t data_format)
{
    OperatorsStandardSetBufferSize(splitter_op, buffer_size);
    OperatorsSplitterEnableSecondOutput(splitter_op, is_second_output_active);
    OperatorsSplitterSetDataFormat(splitter_op, data_format);
}

void OperatorsConfigureSbcEncoder(Operator sbc_encoder_op, sbc_encoder_params_t *params)
{
    OperatorsSbcEncoderSetEncodingParams(sbc_encoder_op, params);
}

void OperatorsConfigureSpdifDecoder(Operator spdif_op, Task message_handler)
{
    MessageOperatorTask(spdif_op, message_handler);
}

void OperatorsConfigureMixer(Operator mixer_op, unsigned sample_rate, unsigned primary_stream,
                             int str1_gain, int str2_gain, int str3_gain,
                             unsigned str1_ch, unsigned str2_ch, unsigned str3_ch)
{
    OperatorsMixerSetChannelsPerStream(mixer_op, str1_ch, str2_ch, str3_ch);
    OperatorsMixerSetGains(mixer_op, str1_gain, str2_gain, str3_gain);
    OperatorsMixerSetPrimaryStream(mixer_op, primary_stream);
    OperatorsStandardSetSampleRate(mixer_op, sample_rate);
}

void OperatorsConfigureRtp(Operator rtp_op, rtp_codec_type_t codec_type, bool protection_enabled, rtp_working_mode_t mode)
{
    OperatorsRtpSetCodecType(rtp_op, codec_type);
    OperatorsRtpSetContentProtection(rtp_op, protection_enabled);
    OperatorsRtpSetWorkingMode(rtp_op, mode);
}

void OperatorsConfigureTtpPassthrough(Operator pass_op, unsigned ttp_latency, unsigned sample_rate,
                                      operator_data_format_t data_format)
{
    OperatorsStandardSetTimeToPlayLatency(pass_op, ttp_latency);
    OperatorsStandardSetSampleRate(pass_op, sample_rate);
    OperatorsSetPassthroughDataFormat(pass_op, data_format);
}

void OperatorsConfigureAEC(Operator aec_op, unsigned in_rate, unsigned out_rate)
{
    OperatorsAecSetSampleRate(aec_op, in_rate, out_rate);
}

void OperatorsConfigureUsbAudio(Operator usb_audio_op, usb_config_t config)
{
    OperatorsUsbAudioSetConfig(usb_audio_op, config);
}

void OperatorsConfigureSvaTriggerPhrase(Operator sva_op, DataFileID sva_model)
{
    sva_data_msg_t sva_message;

    sva_message.msg_id = SVA_TRIGGER_PHRASE_LOAD;
    sva_message.number_of_files = 0x01;
    sva_message.file_id = sva_model;

    PanicFalse(VmalOperatorMessage(sva_op,
                                   (void*)&sva_message,
                                   sizeof(sva_message)/sizeof(uint16),
                                   NULL,
                                   0));

}

void OperatorsSetSwitchedPassthruMode(Operator  spc_op, spc_mode_t mode)
{
    spc_msg_t switched_passthru_ctrl;

    switched_passthru_ctrl.msg_id = (uint16)SPC_SET_TRANSITION;
    switched_passthru_ctrl.set_value = (uint16)mode;

    PanicFalse(VmalOperatorMessage(spc_op,
                                   (void*)&switched_passthru_ctrl,
                                   sizeof(switched_passthru_ctrl)/sizeof(uint16),
                                   NULL,
                                   0));
}

void OperatorsSetSwitchedPassthruEncoding(Operator  spc_op, spc_format_t format)
{
    spc_msg_t switched_passthru_ctrl;

    switched_passthru_ctrl.msg_id = (uint16)SPC_SET_FORMAT;
    switched_passthru_ctrl.set_value = (uint16)format;

    PanicFalse(VmalOperatorMessage(spc_op,
                                   (void*)&switched_passthru_ctrl,
                                   sizeof(switched_passthru_ctrl)/sizeof(uint16),
                                   NULL,
                                   0));
}

