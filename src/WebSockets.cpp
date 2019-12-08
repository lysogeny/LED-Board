/**
 * @file WebSockets.cpp
 * @date 20.05.2015
 * @author Markus Sattler
 *
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * This file is part of the WebSockets for Arduino.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "WebSockets.h"

#define LEGACY_SHA1

#ifdef ESP8266
#include <core_esp8266_features.h>
#endif

extern "C" {
#ifdef CORE_HAS_LIBB64
    #include <libb64/cencode.h>
#else
    #include "libb64/cencode_inc.h"
#endif
}

#ifdef ESP8266
#include <Hash.h>
#else


#ifdef LEGACY_SHA1
    #include "sha1/sha1.h"
#else
    extern "C" {
        #include "libsha1/libsha1.h"
    }
#endif

#endif

#define WEBSOCKETS_MAX_HEADER_SIZE  (14)

/**
 *
 * @param client WSclient_t *  ptr to the client struct
 * @param code uint16_t see RFC
 * @param reason
 * @param reasonLen
 */
void WebSockets::clientDisconnect(WSclient_t * client, uint16_t code, char * reason, size_t reasonLen) {
    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][handleWebsocket] clientDisconnect code: ");
	DEBUG_WEBSOCKETS(code);
    DEBUG_WEBSOCKETS("\n");
    if(client->status == WSC_CONNECTED && code) {
        if(reason) {
            sendFrame(client, WSop_close, (uint8_t *) reason, reasonLen);
        } else {
            uint8_t buffer[2];
            buffer[0] = ((code >> 8) & 0xFF);
            buffer[1] = (code & 0xFF);
            sendFrame(client, WSop_close, &buffer[0], 2);
        }
    }
    clientDisconnect(client);
}

/**
 *
 * @param client WSclient_t *   ptr to the client struct
 * @param opcode WSopcode_t
 * @param payload uint8_t *
 * @param length size_t
 * @param mask bool             add dummy mask to the frame (needed for web browser)
 * @param fin bool              can be used to send data in more then one frame (set fin on the last frame)
 * @param headerToPayload bool  set true if the payload has reserved 14 Byte at the beginning to dynamically add the Header (payload neet to be in RAM!)
 */
bool WebSockets::sendFrame(WSclient_t * client, WSopcode_t opcode, uint8_t * payload, size_t length, bool mask, bool fin, bool headerToPayload) {

    if(client->tcp && !client->tcp->connected()) {
        DEBUG_WEBSOCKETS("[WS][");
	    DEBUG_WEBSOCKETS(client->num);
        DEBUG_WEBSOCKETS("][sendFrame] not Connected!?\n");
        return false;
    }

    if(client->status != WSC_CONNECTED) {
        DEBUG_WEBSOCKETS("[WS][");        
	    DEBUG_WEBSOCKETS(client->num);
        DEBUG_WEBSOCKETS("][sendFrame] not in WSC_CONNECTED state!?\n");
        return false;
    }

    DEBUG_WEBSOCKETS("[WS][");    
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][sendFrame] ------- send massage frame -------\n");
    
    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][sendFrame] fin: ");
	DEBUG_WEBSOCKETS(fin);
    DEBUG_WEBSOCKETS(" opCode: ");
	DEBUG_WEBSOCKETS(opcode);
    DEBUG_WEBSOCKETS(" mask: ");
	DEBUG_WEBSOCKETS(mask);
    DEBUG_WEBSOCKETS(" length: ");
	DEBUG_WEBSOCKETS(length);
    DEBUG_WEBSOCKETS(" headerToPayload: ");
	DEBUG_WEBSOCKETS(headerToPayload);
    DEBUG_WEBSOCKETS("\n");

    if(opcode == WSop_text) {
        DEBUG_WEBSOCKETS("[WS][");
        DEBUG_WEBSOCKETS(client->num);
        DEBUG_WEBSOCKETS("][sendFrame] text: ");
        DEBUG_WEBSOCKETS((char*) (payload + (headerToPayload ? 14 : 0)));
        DEBUG_WEBSOCKETS("\n");
    }

    uint8_t maskKey[4] = { 0x00, 0x00, 0x00, 0x00 };
    uint8_t buffer[WEBSOCKETS_MAX_HEADER_SIZE] = { 0 };

    uint8_t headerSize;
    uint8_t * headerPtr;
    uint8_t * payloadPtr = payload;
    bool useInternBuffer = false;
    bool ret = true;

    // calculate header Size
    if(length < 126) {
        headerSize = 2;
    } else if(length < 0xFFFF) {
        headerSize = 4;
    } else {
        headerSize = 10;
    }

    if(mask) {
        headerSize += 4;
    }


#ifdef WEBSOCKETS_USE_BIG_MEM
    // only for ESP since AVR has less HEAP
    // try to send data in one TCP package (only if some free Heap is there)
    if(!headerToPayload && ((length > 0) && (length < 1400)) && (ESP.getFreeHeap() > 6000)) {
        DEBUG_WEBSOCKETS("[WS][");
        DEBUG_WEBSOCKETS(client->num);
        DEBUG_WEBSOCKETS("][sendFrame] pack to one TCP package...\n");
        
        uint8_t * dataPtr = (uint8_t *) malloc(length + WEBSOCKETS_MAX_HEADER_SIZE);
        if(dataPtr) {
            memcpy((dataPtr + WEBSOCKETS_MAX_HEADER_SIZE), payload, length);
            headerToPayload = true;
            useInternBuffer = true;
            payloadPtr = dataPtr;
        }
    }
#endif

    // set Header Pointer
    if(headerToPayload) {
        // calculate offset in payload
        headerPtr = (payloadPtr + (WEBSOCKETS_MAX_HEADER_SIZE - headerSize));
    } else {
        headerPtr = &buffer[0];
    }

    // create header

    // byte 0
    *headerPtr = 0x00;
    if(fin) {
        *headerPtr |= bit(7);    ///< set Fin
    }
    *headerPtr |= opcode;        ///< set opcode
    headerPtr++;

    // byte 1
    *headerPtr = 0x00;
    if(mask) {
        *headerPtr |= bit(7);    ///< set mask
    }

    if(length < 126) {
        *headerPtr |= length;                   headerPtr++;
    } else if(length < 0xFFFF) {
        *headerPtr |= 126;                      headerPtr++;
        *headerPtr = ((length >> 8) & 0xFF);    headerPtr++;
        *headerPtr = (length & 0xFF);           headerPtr++;
    } else {
        // Normally we never get here (to less memory)
        *headerPtr |= 127;                      headerPtr++;
        *headerPtr = 0x00;                      headerPtr++;
        *headerPtr = 0x00;                      headerPtr++;
        *headerPtr = 0x00;                      headerPtr++;
        *headerPtr = 0x00;                      headerPtr++;
        *headerPtr = ((length >> 24) & 0xFF);   headerPtr++;
        *headerPtr = ((length >> 16) & 0xFF);   headerPtr++;
        *headerPtr = ((length >> 8) & 0xFF);    headerPtr++;
        *headerPtr = (length & 0xFF);           headerPtr++;
    }

    if(mask) {
        if(useInternBuffer) {
            // if we use a Intern Buffer we can modify the data
            // by this fact its possible the do the masking
            for(uint8_t x = 0; x < sizeof(maskKey); x++) {
                maskKey[x] = random(0xFF);
                *headerPtr = maskKey[x];       headerPtr++;
            }

            uint8_t * dataMaskPtr;

            if(headerToPayload) {
                dataMaskPtr = (payloadPtr + WEBSOCKETS_MAX_HEADER_SIZE);
            } else {
                dataMaskPtr = payloadPtr;
            }

            for(size_t x = 0; x < length; x++) {
                dataMaskPtr[x] = (dataMaskPtr[x] ^ maskKey[x % 4]);
            }

        } else {
            *headerPtr = maskKey[0];          headerPtr++;
            *headerPtr = maskKey[1];          headerPtr++;
            *headerPtr = maskKey[2];          headerPtr++;
            *headerPtr = maskKey[3];          headerPtr++;
        }
    }

#ifndef NODEBUG_WEBSOCKETS
    unsigned long start = micros();
#endif

    if(headerToPayload) {
        // header has be added to payload
        // payload is forced to reserved 14 Byte but we may not need all based on the length and mask settings
        // offset in payload is calculatetd 14 - headerSize
        if (client->tcp->write(&payloadPtr[(WEBSOCKETS_MAX_HEADER_SIZE - headerSize)], (length + headerSize)) != (length + headerSize)) {
            ret = false;
        }
    } else {
        // send header
        if (client->tcp->write(&buffer[0], headerSize) != headerSize) {
            ret = false;
        }

        if(payloadPtr && length > 0) {
            // send payload
            if(client->tcp->write(&payloadPtr[0], length) != length) {
                ret = false;
            }
        }
    }

    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][sendFrame] sending Frame Done (");
	DEBUG_WEBSOCKETS((micros() - start));
    DEBUG_WEBSOCKETS("us).\n");

#ifdef WEBSOCKETS_USE_BIG_MEM
    if(useInternBuffer && payloadPtr) {
        free(payloadPtr);
    }
#endif

    return ret;
}

/**
 * handle the WebSocket stream
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSockets::handleWebsocket(WSclient_t * client) {

    uint8_t buffer[8] = { 0 };

    bool fin;
    bool rsv1;
    bool rsv2;
    bool rsv3;
    WSopcode_t opCode;
    bool mask;
    size_t payloadLen;

    uint8_t maskKey[4];

    uint8_t * payload = NULL;

    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][handleWebsocket] ------- read massage frame -------\n");

    if(!readWait(client, buffer, 2)) {
        //timeout
        clientDisconnect(client, 1002);
        return;
    }

    // split first 2 bytes in the data
    fin = ((buffer[0] >> 7) & 0x01);
    rsv1 = ((buffer[0] >> 6) & 0x01);
    rsv2 = ((buffer[0] >> 5) & 0x01);
    rsv3 = ((buffer[0] >> 4) & 0x01);
    opCode = (WSopcode_t) (buffer[0] & 0x0F);

    mask = ((buffer[1] >> 7) & 0x01);
    payloadLen = (WSopcode_t) (buffer[1] & 0x7F);

    if(payloadLen == 126) {
        if(!readWait(client, buffer, 2)) {
            //timeout
            clientDisconnect(client, 1002);
            return;
        }
        payloadLen = buffer[0] << 8 | buffer[1];
    } else if(payloadLen == 127) {
        // read 64bit integer as length
        if(!readWait(client, buffer, 8)) {
            //timeout
            clientDisconnect(client, 1002);
            return;
        }

        if(buffer[0] != 0 || buffer[1] != 0 || buffer[2] != 0 || buffer[3] != 0) {
            // really to big!
            payloadLen = 0xFFFFFFFF;
        } else {
            payloadLen = buffer[4] << 24 | buffer[5] << 16 | buffer[6] << 8 | buffer[7];
        }
    }

    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][handleWebsocket] fin: ");
	DEBUG_WEBSOCKETS(fin);
    DEBUG_WEBSOCKETS(" rsv1: ");
	DEBUG_WEBSOCKETS(rsv1);
    DEBUG_WEBSOCKETS(" rsv2: ");
	DEBUG_WEBSOCKETS(rsv2);
    DEBUG_WEBSOCKETS(" rsv3: ");
	DEBUG_WEBSOCKETS(rsv3);
    DEBUG_WEBSOCKETS(" opCode: ");
	DEBUG_WEBSOCKETS(opCode);
    DEBUG_WEBSOCKETS("\n");
    
    DEBUG_WEBSOCKETS("[WS][");
	DEBUG_WEBSOCKETS(client->num);
    DEBUG_WEBSOCKETS("][handleWebsocket] mask: ");
	DEBUG_WEBSOCKETS(mask);
    DEBUG_WEBSOCKETS(" payloadLen: ");
	DEBUG_WEBSOCKETS(payloadLen);
    DEBUG_WEBSOCKETS("\n");

    if(payloadLen > WEBSOCKETS_MAX_DATA_SIZE) {
        DEBUG_WEBSOCKETS("[WS][");
	    DEBUG_WEBSOCKETS(client->num);
        DEBUG_WEBSOCKETS("][handleWebsocket] payload to big! (");
        DEBUG_WEBSOCKETS(payloadLen);
        DEBUG_WEBSOCKETS(")\n");
        clientDisconnect(client, 1009);
        return;
    }

    if(mask) {
        if(!readWait(client, maskKey, 4)) {
            //timeout
            clientDisconnect(client, 1002);
            return;
        }
    }

    if(payloadLen > 0) {
        // if text data we need one more
        payload = (uint8_t *) malloc(payloadLen + 1);

        if(!payload) {
            DEBUG_WEBSOCKETS("[WS][");
            DEBUG_WEBSOCKETS(client->num);
            DEBUG_WEBSOCKETS("][handleWebsocket] to less memory to handle payload ");
            DEBUG_WEBSOCKETS(payloadLen);
            DEBUG_WEBSOCKETS("!\n");
            clientDisconnect(client, 1011);
            return;
        }

        if(!readWait(client, payload, payloadLen)) {
            DEBUG_WEBSOCKETS("[WS][");
            DEBUG_WEBSOCKETS(client->num);
            DEBUG_WEBSOCKETS("][handleWebsocket] missing data!\n");
            free(payload);
            clientDisconnect(client, 1002);
            return;
        }

        payload[payloadLen] = 0x00;

        if(mask) {
            //decode XOR
            for(size_t i = 0; i < payloadLen; i++) {
                payload[i] = (payload[i] ^ maskKey[i % 4]);
            }
        }
    }

    switch(opCode) {
        case WSop_text:
            DEBUG_WEBSOCKETS("[WS][");
            DEBUG_WEBSOCKETS(client->num);
            DEBUG_WEBSOCKETS("][handleWebsocket] text: ");
            DEBUG_WEBSOCKETS((char*) payload);
            DEBUG_WEBSOCKETS("\n");
            // no break here!
        case WSop_binary:
            messageRecived(client, opCode, payload, payloadLen);
            break;
        case WSop_ping:
            // send pong back
            sendFrame(client, WSop_pong, payload, payloadLen);
            break;
        case WSop_pong:
            DEBUG_WEBSOCKETS("[WS][");
            DEBUG_WEBSOCKETS(client->num);
            DEBUG_WEBSOCKETS("][handleWebsocket] get pong  (");
            DEBUG_WEBSOCKETS((char*) payload);
            DEBUG_WEBSOCKETS(")\n");
            break;
        case WSop_close:
            {
                uint16_t reasonCode = 1000;
                if(payloadLen >= 2) {
                    reasonCode = payload[0] << 8 | payload[1];
                }

                DEBUG_WEBSOCKETS("[WS][");
                DEBUG_WEBSOCKETS(client->num);
                DEBUG_WEBSOCKETS("][handleWebsocket] get ask for close. Code: ");
                DEBUG_WEBSOCKETS(reasonCode);
                if(payloadLen > 2) {
                    DEBUG_WEBSOCKETS(" (");
                    DEBUG_WEBSOCKETS((char*) (payload+2));
                    DEBUG_WEBSOCKETS(")\n");
                } else {
                    DEBUG_WEBSOCKETS("\n");
                }
                clientDisconnect(client, 1000);
            }
            break;
        case WSop_continuation:
            // continuation is not supported
            clientDisconnect(client, 1003);
            break;
        default:
            clientDisconnect(client, 1002);
            break;
    }

    if(payload) {
        free(payload);
    }

}

/**
 * generate the key for Sec-WebSocket-Accept
 * @param clientKey String
 * @return String Accept Key
 */
String WebSockets::acceptKey(String clientKey) {
    uint8_t sha1HashBin[20] = { 0 };
#ifdef ESP8266
    sha1(clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", &sha1HashBin[0]);
#else
    clientKey += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    #ifdef LEGACY_SHA1
      // Miha Nahtigal
      char cbuff[clientKey.length()+1];
      clientKey.toCharArray(cbuff, clientKey.length()+1);//Converts String into character array 
      Sha1.init();
      Sha1.print(cbuff);
      strcpy(sha1HashBin, Sha1.result());
      Serial.println(cbuff);
      //old:uint8_t *sha1HashBin = Sha1.result();     
    #else
      SHA1_CTX ctx;
      SHA1Init(&ctx);           
      SHA1Update(&ctx, (const unsigned char*)clientKey.c_str(), clientKey.length());
      SHA1Final(&sha1HashBin[0], &ctx); 
    #endif
#endif

    String key = base64_encode(sha1HashBin, 20); 
    key.trim();

    return key;
}

/**
 * base64_encode
 * @param data uint8_t *
 * @param length size_t
 * @return base64 encoded String
 */
String WebSockets::base64_encode(uint8_t * data, size_t length) {
    size_t size = ((length*1.6f)+1);
    char * buffer = (char *) malloc(size);
    if(buffer) {
        base64_encodestate _state;
        base64_init_encodestate(&_state);
        int len = base64_encode_block((const char *) &data[0], length, &buffer[0], &_state);
        len = base64_encode_blockend((buffer + len), &_state);

        String base64 = String(buffer);
        free(buffer);
        return base64;
    }
    return String("-FAIL-");
}

/**
 * read x byte from tcp or get timeout
 * @param client WSclient_t *
 * @param out  uint8_t * data buffer
 * @param n size_t byte count
 * @return true if ok
 */
bool WebSockets::readWait(WSclient_t * client, uint8_t *out, size_t n) {
    unsigned long t = millis();
    size_t len;

    while(n > 0) {
        if(!client->tcp) {
            DEBUG_WEBSOCKETS("[readWait] tcp is null!\n");
            return false;
        }

        if(!client->tcp->connected()) {
            DEBUG_WEBSOCKETS("[readWait] not connected!\n");
            return false;
        }

        if((millis() - t) > WEBSOCKETS_TCP_TIMEOUT) {
            DEBUG_WEBSOCKETS("[readWait] receive TIMEOUT!\n");
            return false;
        }

        if(!client->tcp->available()) {
#ifdef ESP8266
            delay(0);
#endif
            continue;
        }

        len = client->tcp->read((uint8_t*) out, n);
        if(len) {
            t = millis();
            out += len;
            n -= len;
            //DEBUG_WEBSOCKETS("Receive %d left %d!\n", len, n);
        } else {
            //DEBUG_WEBSOCKETS("Receive %d left %d!\n", len, n);
        }
        //DEBUG_WEBSOCKETS("Receive ");
        //DEBUG_WEBSOCKETS(len);
        //DEBUG_WEBSOCKETS("left ");
        //DEBUG_WEBSOCKETS(n);      
        //DEBUG_WEBSOCKETS("!\n");                  
#ifdef ESP8266
        delay(0);
#endif
    }
    return true;
}
