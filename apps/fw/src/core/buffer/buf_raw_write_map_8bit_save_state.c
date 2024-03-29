/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   Part of 6.3.0 */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint8 *
buf_raw_write_map_8bit_save_state(BUFFER *buf, buf_mapping_state *save_state)
{
    buf_save_state( save_state );

    mmu_read_port_open( buf->handle );
    mmu_write_port_open( buf->handle );

    (void) mmu_read_port_map_8bit(buf->handle, buf->index );
    return mmu_write_port_map_8bit(buf->handle, buf->index );
}
