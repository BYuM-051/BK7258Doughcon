/* uart_protocol.c — UART packet framing (SerialComm.java logic in C) */
#include "uart_protocol.h"
#include <string.h>

uint8_t uart_checksum(const uint8_t *payload, uint8_t len)
{
    uint32_t sum = 0;
    for (uint8_t i = 0; i < len; i++) sum += payload[i];
    return (uint8_t)(sum & 0xFF);
}

int uart_build_frame(const uint8_t *payload, uint8_t payload_len,
                     uint8_t *out, uint8_t out_size)
{
    /* Frame: STX | LENGTH | payload[0..n] | CHECKSUM | ETX */
    uint8_t total = 1u + 1u + payload_len + 1u + 1u;
    if (total > out_size) return 0;

    out[0] = UART_STX;
    out[1] = payload_len;                            /* LENGTH = cmd + data count */
    memcpy(&out[2], payload, payload_len);
    out[2 + payload_len]     = uart_checksum(payload, payload_len);
    out[2 + payload_len + 1] = UART_ETX;
    return total;
}

bool uart_parse_frame(const uint8_t *buf, uint8_t buf_len, uart_packet_t *pkt)
{
    memset(pkt, 0, sizeof(*pkt));

    /* Minimum frame: STX + LEN + CMD + CHKSUM + ETX = 5 bytes */
    if (!buf || buf_len < 5) return false;
    if (buf[0] != UART_STX)  return false;

    uint8_t length   = buf[1];             /* CMD + DATA count */
    uint8_t expected = 1u + 1u + length + 1u + 1u;   /* total frame size */
    if (expected != buf_len) return false;
    
    if (buf[buf_len - 1] != UART_ETX) return false;

    /* payload = CMD + DATA (length bytes starting at buf[2]) */
    const uint8_t *payload = &buf[2];
    uint8_t chksum = uart_checksum(payload, length);
    if (chksum != buf[2 + length]) return false;
  
    pkt->cmd      = payload[0];
    pkt->data_len = (length > 0) ? (uint8_t)(length - 1u) : 0u;
    if (pkt->data_len > UART_MAX_DATA) pkt->data_len = UART_MAX_DATA;
    memcpy(pkt->data, &payload[1], pkt->data_len);
    pkt->valid = true;
    return true;
}
