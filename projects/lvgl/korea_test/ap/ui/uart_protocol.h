/* uart_protocol.h — Packet framing for Beken7258 ↔ device MCU
 * Frame: [STX=0xAA][LENGTH][CMD][DATA...][CHECKSUM][ETX=0x55]
 * LENGTH  = count of CMD + DATA bytes
 * CHECKSUM = (CMD + sum(DATA)) & 0xFF
 */
#ifndef __UART_PROTOCOL_H__
#define __UART_PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_STX  0xAA
#define UART_ETX  0x55

/* TX command codes */
#define CMD_TX_SETDATA      0x10   /* handshake 1: "SETDATA" + RTC  (16 data) */
#define CMD_TX_CONDATA      0x11   /* handshake 2: "CONDATA" + RTC  (16 data) */
#define CMD_TX_CHANGE       0x30   /* write settings to MCU         (32 data) */
#define CMD_TX_FIRST_START  0x31   /* first run: all params + start time (25 data) */
#define CMD_TX_STATUS       0x33   /* periodic status poll          (23 data) */
#define CMD_TX_HW_TEST      0x50   /* hardware test mode            (16 data) */

/* RX command codes */
#define CMD_RX_SETDATA_ACK  0x20   /* response to 0x10 (24 payload bytes) */
#define CMD_RX_CONDATA_ACK  0x21   /* response to 0x11 (17 payload bytes) */
#define CMD_RX_CHANGE_ACK   0x40   /* response to 0x30 (2  payload bytes: cmd+d1) */
#define CMD_RX_FIRST_ACK    0x41   /* response to 0x31 (3  payload bytes) */
#define CMD_RX_STATUS       0x43   /* periodic status packet       (14 payload) */
#define CMD_RX_HW_TEST_ACK  0x60   /* hardware test response       (16 payload) */

#define UART_MAX_PACKET     64     /* maximum raw packet byte count */
#define UART_MAX_DATA       40     /* maximum payload (cmd+data) bytes */

typedef struct {
    uint8_t  cmd;
    uint8_t  data[UART_MAX_DATA];
    uint8_t  data_len;   /* number of DATA bytes (excluding cmd) */
    bool     valid;
} uart_packet_t;

/* Build a raw TX frame into out[].
 * payload[0] = CMD, payload[1..] = DATA.
 * Returns total frame byte count, or 0 on error. */
int  uart_build_frame(const uint8_t *payload, uint8_t payload_len,
                      uint8_t *out, uint8_t out_size);

/* Parse one raw RX frame (must start with STX).
 * Returns true and fills pkt if checksum and framing are valid. */
bool uart_parse_frame(const uint8_t *buf, uint8_t buf_len, uart_packet_t *pkt);

/* Checksum: sum of all bytes in payload (cmd + data) & 0xFF */
uint8_t uart_checksum(const uint8_t *payload, uint8_t len);

#ifdef __cplusplus
}
#endif
#endif /* __UART_PROTOCOL_H__ */
