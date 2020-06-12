#include "pcapng.h"

#include <stdio.h>
#include <inttypes.h>

static void packet_callback(pcapng_packet_t * const packet)
{
    fprintf(stdout, "got block dir: %d length %"PRIu32" on interface %s\n", packet->direction, packet->packet_length, packet->if_name);
}

int main(int argc, char * * argv)
{
    if (argc > 1)
    {
        pcapng_file_read(argv[1], packet_callback);
    }

    return 0;
}
