/*
 * "[...] Sincerity (comprising truth-to-experience, honesty towards the self,
 * and the capacity for human empathy and compassion) is a quality which
 * resides within the laguage of literature. It isn't a fact or an intention
 * behind the work [...]"
 *
 *             - An introduction to Literary and Cultural Theory, Peter Barry
 *
 *
 *                                                   o8o
 *                                                   `"'
 *     oooo    ooo  .oooo.    .ooooo.   .oooo.o     oooo   .ooooo.
 *      `88.  .8'  `P  )88b  d88' `88b d88(  "8     `888  d88' `88b
 *       `88..8'    .oP"888  888   888 `"Y88b.       888  888   888
 *        `888'    d8(  888  888   888 o.  )88b .o.  888  888   888
 *         .8'     `Y888""8o `Y8bod8P' 8""888P' Y8P o888o `Y8bod8P'
 *     .o..P'
 *     `Y8P'                   Kyunghwan Kwon <kwon@yaos.io>
 *
 *  Welcome aboard!
 */

#include <lib/xmodem.h>
#include <kernel/timer.h>

#define TIMEOUT		1000

enum {
	SOH		= 0x01,
	EOT		= 0x04,
	ACK		= 0x06,
	NAK		= 0x15,
	CAN		= 0x18,
	START		= 0x43,
};

struct xmodem_packet_t {
	uint8_t header;
	uint8_t seq;
	uint8_t seq_inverted;
	uint8_t data[XMODEM_DATA_SIZE];
	uint8_t chksum;
};

unsigned int xmodem_received __attribute__((used));

int xmodem_receive(void *dst, int n, int (*get)(), void (*put)(uint8_t))
{
	struct xmodem_packet_t packet;
	uint8_t *pd, *ps, chksum;
	int coming, i, seq, retry;
	unsigned int tout;

	pd = (uint8_t *)dst;
	ps = (uint8_t *)&packet;
	seq = retry = i = 0;
	chksum = 0;
	tout = 0;

	do {
		if (is_timeout(tout)) {
			set_timeout(&tout, msec_to_ticks(TIMEOUT));

			if (++retry > XMODEM_RETRY_MAX) {
				put(CAN);
				put(CAN);
				break;
			}
			put(NAK);
			i = 0;
		}

		/* NOTE: block read will help throughput rather than byte read */
		if ((coming = get()) == -1)
			continue;

		set_timeout(&tout, msec_to_ticks(TIMEOUT));

		chksum = chksum * !!i + (uint8_t)coming;
		ps[i++] = (uint8_t)coming;
		/* packet synchronization */
		i *= !(packet.header ^ SOH);
		if (i == 3 && ((uint8_t)~(packet.seq) != packet.seq_inverted))
			i = 0;

		if (i >= XMODEM_PACKET_SIZE) {
			i = 0;
			chksum = chksum - packet.header - packet.seq -
				packet.seq_inverted - (uint8_t)coming;

			if (chksum != packet.chksum) {
				if (++retry > XMODEM_RETRY_MAX) {
					put(CAN);
					put(CAN);
					break;
				}
				put(NAK);
				continue;
			}

			/* out of sequence check */
			if (seq == packet.seq) {
				put(ACK);
				continue;
			} else if (((seq + 1) % 256) != packet.seq) {
				put(CAN);
				put(CAN);
				break;
			}

			for (i = XMODEM_DATA_SIZE; n && i; i--, n--)
				*pd++ = packet.data[XMODEM_DATA_SIZE - i];

			seq = packet.seq;
			retry = 0;
			put(ACK);

			xmodem_received++;
		}
	} while (n && packet.header != EOT && packet.header != CAN);

	put(ACK);

	return (int)((unsigned int)pd - (unsigned int)dst);
}
